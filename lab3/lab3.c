// lab3.c
// Liam Duncan 
// 11.5.2020
//
// HARDWARE SETUP: 
// PORTA is connected to the segments of the LED display and to the pushbuttons 
// PORTA.0 corresponds to segment a, PORTA.1 corresponds to segment b, etc...
//
// PORTB bits 4-6 go to a,b,c,inputs of the 74HC138 
// PORTB bit 7 goes to the PWM transistor base 
// PORTB bit 1 goes to SCK and SRCLK 
// PORTB bit 3 goes to SER_OUT 
// PORTB bit 2 goes to SDIN
//
// PORTE bit 6 goes to shift/load_n
// PORTE bit 7 goes to CLK_INH
//
// PORTD bit 2 goes to regclk


#include <avr/io.h> 
#include <avr/interrupt.h>
#include <util/delay.h> 
#include <stddef.h> 

// Define the PORTB Connections that we will be using
#define SEL_0 0x00
#define SEL_1 0x10
#define SEL_2 0x20
#define SEL_3 0x40
#define PWM 0x80 



//holds data to be sent to the segments. logic zero turns segment on
uint8_t segment_data[5]; 


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


uint8_t output = 0; 
uint8_t countBy2 = 0; 
uint8_t countBy4 = 0;
uint16_t counter = 0; 

uint8_t prevENC = 0x00;

void add(){
	if(countBy2 && !countBy4)
		counter += 2; 
	else if(countBy4 && !countBy2)
		counter += 4;  
	else if(!countBy2 && !countBy4)
		counter += 1; 

} 


void subtract(){ 
        if(countBy2 && !countBy4)
                counter -= 2;
        else if(countBy4 && !countBy2)
                counter -= 4;
        else if(!countBy2 && !countBy4)
                counter -= 1;

}


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



ISR(TIMER0_OVF_vect)
{
	  int i = 0;

	  uint8_t static sum = 0; 

  //make PORTA an input port with pullups 
          DDRA = 0x00; // make port A an input so that it can read buttons 
          PORTA = 0xFF;

 //enable tristate buffer for pushbutton switches
          PORTB = 0b11110000;

          _delay_us(5); 

  //now check each button and increment the count as needed
        for(i=0; i<2; i++){

                if(chk_buttons(i)){
			if(i == 0) 
				countBy2 ^= 1;
			else if(i == 1)
				countBy4 ^= 1; 

		} 
          }
	
  	if((countBy2 == 1) && (countBy4 != 1))
	       output = 0b00000001; 

	if((countBy4 == 1) && (countBy2 != 1))
		output = 0x02;

	if((countBy2 == 1) && (countBy4 == 1))
		output = 0x03; 

	if((countBy2 == 0) && (countBy4 == 0))
		output = 0x00; 
	
	

	PORTB = 0xFF; // disable the tri state buffer 

	DDRA = 0xFF; // make porta an output again 


	// push the button state to the bargraph 
	SPDR = output; 
	
	while(bit_is_clear(SPSR, SPIF)){};
	prevENC = SPDR; 


        PORTE &= ~(0x40); // Enable CLK 
        PORTE |= (0x80); // disable LD/SH


                // latch clk to push bar graph 
	PORTD |= 0x04; 
	PORTD &= ~0x04; 

        PORTE &= ~(0x80); // Enable CLK
        PORTE |= (0x40); // disable LD/SH



	uint8_t static LastENCL = 0; 
	uint8_t static LastENCR = 0; 
	uint8_t static ENCL = 0; 
	uint8_t static ENCR = 0; 
	
	LastENCL = ENCL; 
	LastENCR = ENCR; 

	ENCL = (prevENC & 0b00000011); 
	ENCR = ((prevENC & 0b00001100) >> 2);	
		
	if(ENCL != LastENCL){
		if((LastENCL == 0x01) && (ENCL == 0x03))
			add(); 
		if((LastENCL == 0x02) && (ENCL == 0x03))
			subtract(); 
	} 

	if(ENCR != LastENCR){
                if((LastENCR == 0x01) && (ENCR == 0x03))
                        add();
                if((LastENCR == 0x02) && (ENCR == 0x03))
                        subtract();
        }

	

}
// Function to configure timer counters 
void initRegisters()
{
     //timer counter 0 setup, running off i/o clock
     TIMSK |= (1<<TOIE0);             //enable interrupts
     TCCR0 |= (1<<CS02) | (1<<CS00);  //normal mode, prescale by 128

     // Configure SPI 
     // clk low on idle, leading edge sample, Master mode
     SPCR = (1<<SPE) | (1<<MSTR) | (0<<CPHA) | (0<<CPOL);

}



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


uint8_t main()
{
	DDRB |= 0xF0; // Set bits 4-7 on port B as outputs 
        DDRB |= 0x07; // Set SPI Pins as outputs 
	DDRA = 0xFF; // Set port A as all outputs 
	DDRD = 0xFF; // Set Port D as all outputs 
	DDRE |= 0xFF; // set all bits as an output on port E 

	PORTE |= 0x80; // disable CLK_INH 
        PORTE &= ~(0x40); // enable SH/LD_n 
	
	initRegisters(); 	

        sei();   // enable interrupts 
        
        PORTB = 0xF0; 


	while(1){

	_delay_ms(1);  

	if(counter > 1023) 
		counter =0; 

	if (counter > 10000)
		counter = 1023;

        

	segsum(counter); 


 	   //make PORTA an output
           DDRA = 0xFF;
	   PORTB = 0x00; 

           _delay_us(100); 

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

          _delay_us(100);


	


	}

}// main 
