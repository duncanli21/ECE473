// lab1_code.c 
// R. Traylor
// 7.13.20

//This program increments a binary display of the number of button pushes on switch 
//S0 on the mega128 board.

#include <avr/io.h>
#include <util/delay.h>

//*******************************************************************************
//                            debounce_switch                                  
// Adapted from Ganssel's "Guide to Debouncing"            
// Checks the state of pushbutton S0 It shifts in ones till the button is pushed. 
// Function returns a 1 only once per debounced button push so a debounce and toggle 
// function can be implemented at the same time. Expects active low pushbutton on 
// Port D bit zero. Debounce time is determined by external loop delay times 12. 
//*******************************************************************************
int8_t debounce_switch0() {
  static uint16_t state0 = 0; //holds present state
  
  state0 = (state0 << 1) | (! bit_is_clear(PIND, 0)) | 0xE000;
  if (state0 == 0xF000) return 1;
  return 0;
}


//*******************************************************************************
//                            debounce_switch                                  
// Adapted from Ganssel's "Guide to Debouncing"            
// Checks the state of pushbutton S0 It shifts in ones till the button is pushed. 
// Function returns a 1 only once per debounced button push so a debounce and toggle 
// function can be implemented at the same time. Expects active low pushbutton on 
// Port D bit zero. Debounce time is determined by external loop delay times 12. 
//*******************************************************************************
int8_t debounce_switch1() {
  static uint16_t state1 = 0; //holds present state
  
  state1 = (state1 << 1) | (! bit_is_clear(PIND, 1)) | 0xE000;
  if (state1 == 0xF000) return 1;
  return 0;
}

//*******************************************************************************
// Check switch S0.  When found low for 12 passes of "debounce_switch(), increment
// PORTB. This will make an incrementing count on the port B LEDS. 
//*******************************************************************************
int main()
{
DDRB = 0xFF;  //set port B to all outputs
DDRD = 0x00;  // set port D to all inputs
while(1){     //do forever
 if(debounce_switch0() == 1) {PORTB++;}  //if switch true for 12 passes, increment port B
 if(debounce_switch1() == 1) {PORTB--;}  //if switch true for 12 passes, decrement port B

  _delay_ms(2);                    //keep in loop to debounce 24ms
  } //while 
} //main

