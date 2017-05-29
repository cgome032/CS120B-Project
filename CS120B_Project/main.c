/*
 * CS120B_Project.c
 *
 * Created: 5/28/2017 8:40:42 PM
 * Author : Carlos
 */ 

#include <avr/io.h>
#include "PWM.h"
#include "timer.h"
#include <util/delay.h>

enum States{one,two}state;
int pulse = 97;
void SM_tick(){
	switch(state){ // transitions
		case one:
			state = two;
			break;
		case two:
			state = one;
			break;
	}
	switch(state){ // state actions
		case one:
			for(pulse; pulse <=425; pulse += 1){
				//set_PWM(pulse);
				OCR3A = pulse;
				_delay_loop_2(100);
			}
			break;
		case two:
			for(pulse;pulse >= 97; pulse -=1){
				//set_PWM(pulse);
				OCR3A = pulse;
				_delay_loop_2(100);
			}
			break;
	}
}

int main(void)
{
	//DDRA = 0x00; PORTA = 0xFF; // A input initialized to 0xFF
	DDRB = 0xFF; PORTB = 0x00; // B output initialized to 0x00

	TimerSet(1);
	TimerOn();
	PWM_on();
	state = one;
	while(1){
		SM_tick();
		while(!TimerFlag){}
		TimerFlag = 0;
	}
}