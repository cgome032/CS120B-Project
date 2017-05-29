/*
 * CS120B_Project.c
 *
 * Created: 5/28/2017 8:40:42 PM
 * Author : Carlos
 */ 

#include <avr/io.h>
#include "Servo_PWM.h"
#include "Speaker_PWM.h"
#include "timer.h"
#include <util/delay.h>
#include <avr/interrupt.h>
#include "keypad.h"


enum SM_Servo_States{one,two}servo_state;
int pulse = 97;
void SM_Servo_tick(){
	switch(servo_state){ // transitions
		case one:
			servo_state = two;
			break;
		case two:
			servo_state = one;
			break;
	}
	switch(servo_state){ // state actions
		case one:
			servo_set_PWM(1000);
			break;
		case two:
			servo_set_PWM(1500);
			break;
	}
}


enum SM_Lock_States{lock_Start,lock_Locked,lock_lockRelease,lock_Unlocked,lock_unlockRelease}lock_state;
char passCode[6] = {'#','1','2','3','4','#'};
unsigned char lockCounter = 0;
void SM_Lock_Tick(){
	unsigned char x = GetKeypadKey();
	switch(lock_state){		// Transitions
		case lock_Start:
			lockCounter = 0;
			lock_state = lock_Locked;
			servo_set_PWM(1500);
			break;
		case lock_Locked:
			if(x == '\0'){
				lock_state = lock_Locked;
			}
			else{
				lock_state = lock_lockRelease;
				if(x==passCode[lockCounter]){
					lockCounter++;
				}
				else{
					lockCounter = 0;
				}
			}
			break;
		case lock_lockRelease:
			if(x != '\0'){
				lock_state = lock_lockRelease;
			}
			else if(x == '\0' && lockCounter<=5){
				lock_state = lock_Locked;
			}
			else if(x == '\0' && lockCounter > 5){
				servo_set_PWM(1000);
				lock_state = lock_Unlocked;
				lockCounter = 0;
			}
			break;
		case lock_Unlocked:
			if(x == '\0'){
				lock_state = lock_Unlocked;
			}
			else{
				lock_state = lock_unlockRelease;
				if(x==passCode[lockCounter]){
					lockCounter++;
				}
				else{
					lockCounter = 0;
				}
			}
			break;
		case lock_unlockRelease:
			if(x != '\0'){
				lock_state = lock_unlockRelease;
			}
			else if(x == '\0' && lockCounter<=5){
				lock_state = lock_Unlocked;
			}
			else if(x == '\0' && lockCounter > 5){
				servo_set_PWM(1500);
				lock_state = lock_Locked;
				lockCounter = 0;
			}
			break;
	}
	switch(lock_state){		// State actions
		case lock_Start: 
			break;
		case lock_Locked:
			speaker_set_PWM(0);
			break;
		case lock_lockRelease:
			break;
		case lock_Unlocked:
			speaker_set_PWM(329.63);
			break;
		case lock_unlockRelease:
			break;
	}
}



int main(void)
{

	DDRB = 0xFF; PORTB = 0xFF; // Set B to output
	DDRC = 0xF0; PORTC = 0x0F; // Set PC7 to input keypad
	DDRD |= 0xFF;

	servo_PWM_on();
	speaker_PWM_on();
	TimerOn();
	TimerSet(1);
	servo_state = one;
	lock_state = lock_Start;

	while (1)
	{
		//SM_Servo_tick();
		SM_Lock_Tick();
		while(!TimerFlag){}
		TimerFlag=0;
	}
}