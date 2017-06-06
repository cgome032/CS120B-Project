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
#include "nokia5110.c"

/*********************************************************************************
Global Variables


*********************************************************************************/
unsigned char tmpD = 0x00; // Temporary variable to hold D




/**************************************************
State Machine to control locking mechanism using 
keypad


***************************************************/

enum SM_Lock_States{lock_Start,lock_Locked,lock_lockRelease,lock_Unlocked,lock_unlockRelease,lock_Locking}lock_state;
char passCode[6] = {'#','1','2','3','4','#'};
unsigned int lockCounter = 0;
void SM_Lock_Tick(){
	unsigned char x = GetKeypadKey();
	switch(lock_state){		// Transitions
		case lock_Start:
			lockCounter = 0;
			lock_state = lock_Unlocked;
			servo_set_PWM(1000);
			break;
		case lock_Locked:
			if(x == '\0'){
				lock_state = lock_Locked;
			}
			else{
				lock_state = lock_lockRelease;
				if(x==passCode[lockCounter]){
					// Set cursor to write passcode attempt to screen
					nokia_lcd_set_cursor(lockCounter*10,10);
					nokia_lcd_write_char(x,2);
					nokia_lcd_render();
					lockCounter++;
				}
				else{
					nokia_lcd_clear();
					nokia_lcd_render();
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
				nokia_lcd_clear();
				nokia_lcd_render();
				nokia_lcd_set_cursor(2,10);
				nokia_lcd_write_string("Unlocked",1);
				nokia_lcd_render();
				_delay_ms(1000);
				nokia_lcd_clear();
				nokia_lcd_render();
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
					nokia_lcd_set_cursor(lockCounter*10,10);
					nokia_lcd_write_char(x,2);
					nokia_lcd_render();
					lockCounter++;
				}
				else{
					nokia_lcd_clear();
					nokia_lcd_render();
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
				servo_set_PWM(1000);
				lock_state = lock_Locking;
				lockCounter = 0;
				tmpD = 0x03;
			}
			break;
		case lock_Locking:
			if(lockCounter <= 5){
				nokia_lcd_clear();
				nokia_lcd_render();
				nokia_lcd_set_cursor(2,10);
				nokia_lcd_write_string("Locking",1);
				nokia_lcd_set_cursor(2,20);
				nokia_lcd_write_string("Please exit",1);
				nokia_lcd_render();
				//_delay_ms(500);
				//nokia_lcd_clear();
				//nokia_lcd_render();
				lock_state = lock_Locking;
				lockCounter++;
				tmpD = 0x03;
			}
			else{
				nokia_lcd_clear();
				nokia_lcd_render();
				lock_state = lock_Locked;
				lockCounter = 0;
				servo_set_PWM(1500);
				tmpD = 0x00;
			}
			break;			
	}
	switch(lock_state){		// State actions
		case lock_Start: 
			break;
		case lock_Locked:
			nokia_lcd_set_cursor(0,0);
			nokia_lcd_write_string("Enter Passcode",1);
			nokia_lcd_render();
			break;
		case lock_lockRelease:
			break;
		case lock_Unlocked:
			nokia_lcd_set_cursor(0,0);
			nokia_lcd_write_string("Enter Passcode",1);
			nokia_lcd_render();
			break;
		case lock_unlockRelease:
			break;
		case lock_Locking:
			speaker_set_PWM(1000);
			_delay_ms(130);
			speaker_set_PWM(0);
			_delay_ms(130);
			break;
	}
}


/**************************************************
State Machine to control sensor state


***************************************************/

enum SM_Sensor_States{sensorOff,sensorOn}sensor_state;

unsigned char senseCheck = 0x00;

void SM_Sensor_Tick(){
	senseCheck = PINB & 0x01;
	switch(sensor_state){		// Transitions
		case sensorOff:
			if(senseCheck){
				sensor_state = sensorOn;
			}
			else
				sensor_state = sensorOff;
			break;
		case sensorOn:
			if(senseCheck ){
				sensor_state = sensorOn;
			}
			else
				sensor_state = sensorOff;
			break;
	}
	switch(sensor_state){		// State actions
		case sensorOff:
			break;
		case sensorOn:
			break;
	}
}

/**************************************************
State Machine to control alarm noise


***************************************************/

enum SM_Alarm_States{alarmOn,alarmLED,alarmOff}alarm_state;
unsigned char alarmCounter = 0x00;


void SM_Alarm_Tick(){
	switch(alarm_state){		//Transitions
		case alarmOff:
			if((lock_state == lock_Locked || lock_state == lock_lockRelease) && sensor_state == sensorOn){
				alarm_state = alarmOn;
			}
			else
				alarm_state = alarmOff;
			break;
		case alarmOn:
			if(lock_state == lock_Unlocked){
				alarm_state = alarmOff;
			}
			else if(alarmCounter <= 10){
				alarm_state = alarmOn;
				alarmCounter++;
			}
			else if(alarmCounter > 10){
				alarm_state = alarmLED;
				alarmCounter = 0;
			}
			break;
		case alarmLED:
			if(lock_state == lock_Unlocked){
				alarm_state = alarmOff;
			}
			else if(alarmCounter <= 10){
				alarm_state = alarmLED;
				alarmCounter++;
			}
			else if(alarmCounter > 10){
				alarm_state = alarmOn;
				alarmCounter = 0;
			}
			break;
			
			
	}

	switch(alarm_state){		//State actions
		case alarmOff:
			speaker_set_PWM(0);
			PORTB = 0x00;
			break;
		case alarmOn:
			speaker_set_PWM(3000);
			PORTB = 0xFF;
			break;
		case alarmLED:
			speaker_set_PWM(3000);
			PORTB = 0x00;
			break;
	}
}




int main(void)
{
	DDRA = 0xFF; PORTA = 0x00; // Initialize DDRA to outputs
	DDRB = 0x0A; PORTB = 0xF5; // Set B3 to output, everything else to input
	DDRC = 0xF0; PORTC = 0x0F; // Set PC7 to input keypad
	DDRD |= 0xFF; PORTD = 0x00; // Initialize DDRD to outputs

	servo_PWM_on();
	speaker_PWM_on();
	TimerOn();
	TimerSet(1);
	
	nokia_lcd_init();
	nokia_lcd_clear();
	nokia_lcd_set_cursor(0,15);
	nokia_lcd_write_string("Welcome!",2);
	nokia_lcd_render();
	_delay_ms(1000);
	nokia_lcd_clear();
	nokia_lcd_render();

	lock_state = lock_Start;
	sensor_state = sensorOff;
	alarm_state = alarmOff;
	while (1)
	{
		SM_Lock_Tick();
		SM_Sensor_Tick();
		SM_Alarm_Tick();
		PORTD = tmpD;
		while(!TimerFlag){}
		TimerFlag=0;
	}
}