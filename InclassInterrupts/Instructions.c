//Inclass Exercise (interrupts)
//R. Traylor 9.1.2020

//This exercise will give you some experience in using interrupts 
//including how they can operate differently than you expect. Sometimes
//in embedded programming, you can have code that is logically
//correct but temporally broken.

//First, download the necessary files into a working area using the commands below:
//  wget http://www.ece.orst.edu/~traylor/ece473/inclass_exercises/interrupts/tcnt0_whats_happening.pdf
//  wget http://www.ece.orst.edu/~traylor/ece473/inclass_exercises/interrupts/Makefile

//Look at the downloaded .pdf file. It helps you visualize what's going on in the code.
//View it with: evince tcnt0_whats_happening.pdf & 
//The "&" runs evince in the background and keeps the shell for further use.

//Secondly, fill in the required code as indicated by the comments, replacing the blank lines. 

//Lastly, There is a line of missing code just after the statement "PORTB++;" in main(). 
//What is needed there? Anything? Explain why or why not.

//Submit only this completed file (renamed to Instructions.c) to Canvas by the end of day (5pm).

//-----------------------------------------------------------------------------------

// Inclass exercise code follows

// source is from tcnt0_volatile_skel.c
// TCNT0 is setup in output compare mode 
// create incrementing count pattern on port B
// count is updated is every (32668)*(128)*(256) = 0.999975168 sec

#include <avr/io.h>
//include header files for interrupts (see avrlibc)

#include <avr/interrupt.h> 

//declare a 8-bit variable called "ext_count" that is outside scope of main

volatile uint8_t ext_count; 

//***********************************************************************
//                     ISR for timer counter zero
//***********************************************************************
//Write the ISR for timer counter zero here

ISR(TIMER0_COMP_vect)
{
	ext_count++; 
}

//***********************************************************************
//                           init_tcnt0
//***********************************************************************
//initalize timer/counter zero to CTC mode

void init_tcnt0(){
  ASSR  |=  (1<<AS0);                //run off external 32khz osc (TOSC)
//Enable the interrupt for output compare match 0 (see avrlibc)
  TIMSK |= (1<<OCIEO); 

  TCCR0 |=  (1<<WGM01) | (1<<CS00);  //CTC mode, no prescale
  OCR0  |=  0x07f;                   //compare at 128
}

//***********************************************************************

//***********************************************************************
//                              main
//***********************************************************************
int main() {
  DDRB = 0xFF;                     //set all port B pins to output
  init_tcnt0();                    //initalize timer counter zero
  
  sei; 

  while(1){
    if(ext_count == 255){          //blink light at 1sec intervals
      PORTB++;                     //increment count pattern
      ext_count = 0; 		   // reset counter variable
    } 
  }//while
}// main

//explaination of the line just after the PORTB++; statement:

// you need to reset the external counter variable or else it will 
// keep growing forever and never be == 255 again 
_______________________________________________________________________

_______________________________________________________________________
