// lab2.c
// Liam Duncan
// 10.15.20

//  HARDWARE SETUP:
//  PORTA is connected to the segments of the LED display. and to the pushbuttons.
//  PORTA.0 corresponds to segment a, PORTA.1 corresponds to segement b, etc.
//  PORTB bits 4-6 go to a,b,c inputs of the 74HC138.
//  PORTB bit 7 goes to the PWM transistor base.

#define TRUE 1
#define FALSE 0
#include <avr/io.h>
#include <util/delay.h>

//holds data to be sent to the segments. logic zero turns segment on
uint8_t segment_data[5]; 



// Define all PORTA connections 
#define SEG_A 0x01
#define SEG_B 0x02
#define SEG_C 0x04
#define SEG_D 0x08
#define SEG_E 0x10
#define SEG_F 0x20
#define SEG_G 0x40 
#define DP 0x80 


// Define the PORTB Connections that we will be using
#define SEL_0 0x00
#define SEL_1 0x10
#define SEL_2 0x20
#define SEL_3 0x40
#define PWM 0x80 

//decimal to 7-segment LED display encodings, logic "0" turns on segment
uint8_t dec_to_7seg[12] =  {
       0b11000000, // 0
       0b11111001, // 1
       0b10100100, // 2
       0b10110000, // 3
       0b10011001, // 4
       0b10010010, // 5
       0b10000010, // 6 
       0b11111000, // 7
       0b10000000, // 8
       0b10010000, // 9
       0b01111111, // . 
       0b11111111, // Clear 
};

//******************************************************************************
//                            chk_buttons                                      
//Checks the state of the button number passed to it. It shifts in ones till   
//the button is pushed. Function returns a 1 only once per debounced button    
//push so a debounce and toggle function can be implemented at the same time.  
//Adapted to check all buttons from Ganssel's "Guide to Debouncing"            
//Expects active low pushbuttons on PINA port.  Debounce time is determined by 
//external loop delay times 12. 
//
uint8_t chk_buttons(uint8_t button) {
	static uint16_t state[8] = {0}; // Create an array full of 0s to hold the state of the buttons 

	state[button] = (state[button] << 1 | (!bit_is_clear(PINA, button)) | 0xE000); // debounce the button 

	if(state[button] == 0xF000) // check to see if the button has actually been pressed 
		return 1;  // return a 1 if the button has been pressed 

	return 0;  // return a 0 if the button was not pressed  
} 
//******************************************************************************

//***********************************************************************************
//                                   segment_sum                                    
//takes a 16-bit binary input value and places the appropriate equivalent 4 digit 
//BCD segment code in the array segment_data for display.                       
//array is loaded at exit as:  |digit3|digit2|colon|digit1|digit0|
void segsum(uint16_t sum) {

  //determine how many digits there are 
  uint8_t digits = 0; 

  if((sum < 1)) digits = 0; // sum is equal to zero or greater than 1023  
  if((sum < 10)) digits = 1;  // sum is less than 10
  if((sum < 100) ) digits = 2;  // sum has 2 digits 
  if((sum < 1000)) digits = 3; // sum has 3 digits 

  //break up decimal sum into 4 digit-segments
  segment_data[0] = dec_to_7seg[sum % 10];  // Get the digits for the 1s place 
  segment_data[1] = dec_to_7seg[(sum/10) % 10]; // Get the digits for the 10s place 
  segment_data[3] = dec_to_7seg[(sum/100) % 10];  // get the digits for the 100s place 
  segment_data[4] = dec_to_7seg[(sum/1000) % 10];   // get the digits for the 1000s place 
  segment_data[2] = 0xFF; // This is the colon bit so keep the lights off 
  
  //blank out leading zero digits 
  if(sum < 1000) segment_data[4] = 0xFF; 
  if(sum < 100) segment_data[3] = 0xFF; 
  if(sum < 10) segment_data[1] = 0xFF; 
  if(sum > 999) segment_data[4] = dec_to_7seg[1]; 
 // if(digits == 0) segment_data[0] = dec_to_7seg[0];  
 
  return;  
  //now move data to right place for misplaced colon position
}//segment_sum
//***********************************************************************************


//***********************************************************************************
uint8_t main()
{
uint16_t sum = 0x0000; 

//set port bits 4-7 B as outputs
DDRB = 0xF0;  

while(1){
  //insert loop delay for debounce
	  int i = 0; 
	  _delay_ms(1); 

  //make PORTA an input port with pullups 
	  DDRA = 0x00; // make port A an input so that it can read buttons 
	  PORTA = 0xFF; 

 //enable tristate buffer for pushbutton switches
	  PORTB = 0b11110000; 

  //now check each button and increment the count as needed


  	  for(i=0; i<8; i++){
	  
		if(chk_buttons(i))
	      		sum += (1<<i);
	  }		

  //disable tristate buffer for pushbutton switches
           PORTB = 0b00000000; 

  //bound the count to 0 - 1023
  	   if(sum > 1023) 
		   sum  =  0;

  //break up the disp_value to 4, BCD digits in the array: call (segsum)
  	   segsum(sum); 

  //bound a counter (0-4) to keep track of digit to display 
  	   int cnt = 0; 

  //make PORTA an output
           DDRA = 0xFF; 
 
  //send 7 segment code to LED segments
  	  PORTA = segment_data[0];
	  PORTB = SEL_0; 
	  _delay_ms(1); 
	  PORTA = segment_data[1]; 
	  PORTB = SEL_1; 
	  _delay_ms(1); 
	  PORTA = segment_data[2];
	  PORTB = SEL_2; 
	  _delay_ms(1); 
	  PORTA = segment_data[3]; 
	  PORTB = 0x30;
	  _delay_ms(1); 
	  PORTA = segment_data[4]; 
	  PORTB = 0x40; 
         
  //send PORTB the digit to display
  //update digit to display
  }//while
}//main
