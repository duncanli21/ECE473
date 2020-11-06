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


uint8_t main()
{
	DDRB |= 0xF0; // Set bits 4-7 on port B as outputs 
	DDRA = 0xFF; // Set port A as all outputs 
	DDRD = 0xFF; // Set Port D as all outputs 
	DDRE |= 0xFF; // set all bits as an output on port E 

	PORTE |= 


}

