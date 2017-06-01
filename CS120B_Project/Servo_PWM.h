#ifndef SERVO_PWM_H_
#define SERVO_PWM_H_

#include <avr/io.h>
// NOTE*** THIS NEW CODE TARGETS PB6 NOT PB3

void servo_set_PWM(int cycle) {
	OCR1A = cycle;

}

void servo_PWM_on() {
	TCCR1A |= 1<<WGM11 | 1<<COM1A1 | 1<<COM1A0;
	TCCR1B |= 1<<WGM13 | 1<<WGM12 | 1<<CS10;
	servo_set_PWM(1000);


}

void servo_PWM_off() {
	TCCR0A = 0x00;
	TCCR0B = 0x00;
}

#endif // SERVO_PWM_H_
