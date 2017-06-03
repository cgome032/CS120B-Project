#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;// bit3 = 0: CTC mode (clear timer on compare)
	// bit2bit1bit0=011: pre-scaler /64
	// 00001011: 0x0B
	// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
	// Thus, TCNT1 register will count at 125,000 ticks/s

	// AVR output compare register OCR1A.
	OCR1A = 125;	// Timer interrupt will be generated when TCNT1==OCR1A
	// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
	// So when TCNT1 register equals 125,
	// 1 ms has passed. Thus, we compare to 125.
	// AVR timer interrupt mask register
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;
	// TimerISR will be called every _avr_timer_cntcurr milliseconds

	//Enable global interrupts
	SREG |= 0x80; // 0x80: 1000000
}

void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}

void TimerISR() {
	TimerFlag = 1;
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}


enum dispStates{zero,one,two,three,four,five}dispState;

// Temporary variables to hold values for PORTA and PORTB
unsigned char tmpA = 0x00;
unsigned char tmpB = 0x00;

// Variables to keep track of vertical and horizontal locations
unsigned char locY = 10;
unsigned char locX = 10;



// Display two LED's on
unsigned char countInc = 0x00;
unsigned char timeCount = 100;
void disp_countDown_Tick(){
	switch(dispState){
		case five:	
			break;
		case four:
			if(countInc == timeCount){
				dispState = three;
				countInc = 0;
			}
			else{
				dispState = four;
				countInc++;
			}
			
			break;
		case three:
			if(countInc == timeCount){
				dispState = two;
				countInc = 0;
			}
			else{
				dispState = three;
				countInc++;
			}
		
			break;
		case two:
			if(countInc == timeCount){
				dispState = one;
				countInc = 0;
			}
			else{
				dispState = two;
				countInc++;
			}

			break;
		case one:
			if(countInc == timeCount){
				dispState = 0;
				countInc = 0;
			}
			else{
				dispState = one;
				countInc++;
			}
			break;
		case zero:
			break;
	}
	switch(dispState){
		case five:
			if(locX == 10 && locY ==10){
				tmpA = 0x20;
				tmpB = 0xFE;
				locX = 0;
				locY = 0;
			}
			else if(locX <3 && locY ==0){
				tmpA = tmpA >> 1;
				tmpB = tmpB;
				locX++;
			}
			else if(locX == 3 && locY < 4){
				tmpA = tmpA;
				tmpB = ~(~tmpB << 1);
				locY++;
			}
			else if(locX >0 && locY == 4){
				tmpA = tmpA << 1;
				tmpB = tmpB;
				locX--;
			}
			else if(locX == 0 && locY >1){
				tmpA = tmpA;
				tmpB = ~(~tmpB << 1);
				locY--;
			}
			else if(locX <= 2 && locY ==1){
				tmpA = tmpA >> 1;
				tmpB = tmpB;
				locX++;
				if(locX == 3){
					locX = 10;
					locY = 10;
				}
			}

			
			break;
		case four:
			break;
		case three:
			break;
		case two:
			break;
		case one:
			break;
		case zero:
			break;

	}
}



enum successStates{chOne,chTwo,chThree,chFour,chFive,chSix}success_state;
void disp_sucess_Tick(){
	switch(success_state){
		case chOne:
			success_state = chTwo;
			break;
		case chTwo:
			success_state = chThree;
			break;
		case chThree:
			success_state = chFour;
			break;
		case chFour:
			success_state = chFive;
			break;
		case chFive:
			success_state = chSix;
			break;
		case chSix:
			success_state = chOne;
			break;
	}

	switch(success_state){
		case chOne:
			PORTA = 0xE0;
			PORTB = 0x40;
			break;
		case chTwo:
			PORTA = 0x00;
			PORTB = 0x00;
			break;
		case chThree:
			PORTA = 0x00;
			PORTB = 0x00;
			break;
		case chFour:
			PORTA = 0x00;
			PORTB = 0x00;
			break;
		case chFive:
			PORTA = 0x00;
			PORTB = 0x00;
			break;
		case chSix:
			PORTA = 0x00;
			PORTB = 0x00;
			break;
	}
}


int main()
{
	DDRA = 0xFF; PORTA = 0x00; // Initialize DDRA to outputs
	DDRB = 0xFF; PORTB = 0x00; // Initialize DDRB to outputs

	TimerSet(1);
	TimerOn();
	dispState = five;
	tmpA=0x20;
	tmpB = 0xFE;

	while(1) {
		// User code (i.e. synchSM calls)
		disp_countDown_Tick();
		PORTA = tmpA;
		PORTB = tmpB;
		while (!TimerFlag);	// Wait 1 sec
		TimerFlag = 0;
		
		// Note: For the above a better style would use a synchSM with TickSM()
		// This example just illustrates the use of the ISR and flag
		
	}
	return 0;
}