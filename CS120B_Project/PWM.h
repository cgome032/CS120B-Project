#ifndef PWM_H_
#define PWM_H_

#include <avr/io.h>
// NOTE*** THIS NEW CODE TARGETS PB6 NOT PB3

void set_PWM(int cycle) {
	OCR1A = cycle;
	
}

void PWM_on() {
	TCCR1A |= 1<<WGM11 | 1<<COM1A1 | 1<<COM1A0;
	TCCR1B |= 1<<WGM13 | 1<<WGM12 | 1<<CS10;
}

void PWM_off() {
	TCCR0A = 0x00;
	TCCR0B = 0x00;
}

#endif // PWM_H_