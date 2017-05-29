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
#include <avr/interrupt.h>

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
			set_PWM(1000);
			break;
		case two:
			set_PWM(1500);
			//OCR1A = 1500;
			break;
	}
}

int main(void)
{
	DDRD |= 0xFF;

	PWM_on();
	TimerOn();
	TimerSet(100);
	state = one;

	while (1)
	{
		SM_tick();
		while(!TimerFlag){}
		TimerFlag=0;
	}
}