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


enum dispStates{zero,one,two,three,four,five,wait}dispState;

// Temporary variables to hold values for PORTA and PORTB
unsigned char tmpA = 0x00;
unsigned char tmpB = 0x00;
unsigned char tmpD = 0x00;

// Variables to keep track of vertical and horizontal locations
unsigned char arrayCount = 0x00;



// Display two LED's on
unsigned int countInc = 0;
unsigned int timeCount = 1000; // Time count between states on LED Matrix

/*************************************************************
State arrays to hold values for LED matrix to show 5,4,3,2,1



*************************************************************/

unsigned char arrayFive[17][2] = {
	{0x20,0x01},
	{0x10,0x01},
	{0x08,0x01},
	{0x04,0x01},
	{0x04,0x02},
	{0x04,0x04},
	{0x04,0x08},
	{0x04,0x10},
	{0x08,0x10},
	{0x10,0x10},
	{0x20,0x10},
	{0x20,0x20},
	{0x20,0x40},
	{0x20,0x80},
	{0x10,0x80},
	{0x08,0x80},
	{0x04,0x80}
};

unsigned char arrayFour[8][2] = {
	{0x04,0x01},
	{0x04,0x02},
	{0x04,0x04},
	{0x04,0x08},
	{0x04,0x10},
	{0x08,0x10},
	{0x10,0x10},
	{0x20,0xFF}
};

unsigned char arrayThree[10][2] = {
	{0x04,0x01},
	{0x08,0x01},
	{0x10,0x01},
	{0x04,0x08},
	{0x08,0x08},
	{0x10,0x08},
	{0x04,0x80},
	{0x08,0x80},
	{0x10,0x80},
	{0x20,0xFF}
};

unsigned char arrayTwo[17][2] = {
	{0x04,0x01},
	{0x08,0x01},
	{0x10,0x01},
	{0x20,0x01},
	{0x20,0x02},
	{0x20,0x04},
	{0x20,0x08},
	{0x10,0x08},
	{0x08,0x08},
	{0x04,0x08},
	{0x04,0x10},
	{0x04,0x20},
	{0x04,0x40},
	{0x04,0x80},
	{0x08,0x80},
	{0x10,0x80},
	{0x20,0x80}
};

unsigned char arrayOne[1][2] = {
	{0x20,0xFF}
};

unsigned char arrayZero[5][2] = {
	{0x24,0xFF},
	{0x08,0x01},
	{0x10,0x01},
	{0x08,0x80},
	{0x10,0x80}
};





/*************************************************************
disp_countDown_Tick()

Display Countdown function to countdown visually on LED matrix
from 5 to 0



*************************************************************/

void disp_countDown_Tick(){
	tmpD = PIND & 0x03;
	switch(dispState){ // State transitions
		case five:
			if(countInc == timeCount){
				dispState = four;
				arrayCount = 0;
				countInc = 0;
			}
			else{
				dispState = five;
				countInc++;
			}	
			break;
		case four:
			if(countInc == timeCount){
				dispState = three;
				arrayCount = 0;
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
				arrayCount = 0;
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
				arrayCount = 0;
				countInc = 0;
			}
			else{
				dispState = two;
				countInc++;
			}

			break;
		case one:
			if(countInc == timeCount){
				dispState = zero;
				arrayCount = 0;
				countInc = 0;
			}
			else{
				dispState = one;
				countInc++;
			}
			break;
		case zero:
			if(!tmpD){
				dispState = wait;
			}
			else{
				dispState = zero;
				countInc++;
			}
			break;
		case wait:
			if(tmpD){
				dispState = five;
				arrayCount = 0;
				countInc = 0;
			}
			else{
				dispState = wait;
				countInc++;
			}
	}
	switch(dispState){ // State actions
		case five:
			tmpA = arrayFive[arrayCount][0];
			tmpB = ~arrayFive[arrayCount][1];
			arrayCount++;
			if(arrayCount == 17){
				arrayCount = 0;
			}
			break;
		case four:
			tmpA = arrayFour[arrayCount][0];
			tmpB = ~arrayFour[arrayCount][1];
			arrayCount++;
			if(arrayCount == 8){
				arrayCount = 0;
			}
			break;
		case three:
			tmpA = arrayThree[arrayCount][0];
			tmpB = ~arrayThree[arrayCount][1];
			arrayCount++;
			if(arrayCount == 10){
				arrayCount = 0;
			}
			break;
		case two:
			tmpA = arrayTwo[arrayCount][0];
			tmpB = ~arrayTwo[arrayCount][1];
			arrayCount++;
			if(arrayCount == 17){
				arrayCount = 0;
			}
			break;
		case one:
			tmpA = arrayOne[arrayCount][0];
			tmpB = ~arrayOne[arrayCount][1];
			arrayCount++;
			if(arrayCount == 1){
				arrayCount = 0;
			}
			break;
		case zero:
			tmpA = arrayZero[arrayCount][0];
			tmpB = ~arrayZero[arrayCount][1];
			arrayCount++;
			if(arrayCount == 5){
				arrayCount = 0;
			}
			break;
		case wait:
			tmpA = arrayZero[arrayCount][0];
			tmpB = ~arrayZero[arrayCount][1];
			arrayCount++;
			if(arrayCount == 5){
				arrayCount = 0;
			}
			break;

	}
}

int main()
{
	DDRA = 0xFF; PORTA = 0x00; // Initialize DDRA to outputs
	DDRB = 0xFF; PORTB = 0x00; // Initialize DDRB to outputs
	DDRD = 0x00; PORTD = 0xFF; // Initialize DDRD to inputs

	TimerSet(1);
	TimerOn();
	dispState = wait;

	while(1) {
		

			
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