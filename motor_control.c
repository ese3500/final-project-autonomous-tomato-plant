#include "motor_control.h"

void init_motor_pins(void) {
	// Set all motor control pins to output
	DDRD |= (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7);
	DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3);
}

void forward(void) {
	PORTD |= (1 << PD4) | (1 << PD6);
	PORTD &= ~((1 << PD5) | (1 << PD7));

	PORTB |= (1 << PB0) | (1 << PB2);
	PORTB &= ~((1 << PB1) | (1 << PB3));
}

void reverse(void) {
	PORTD |= (1 << PD5) | (1 << PD7);
	PORTD &= ~((1 << PD4) | (1 << PD6));

	PORTB |= (1 << PB1) | (1 << PB3);
	PORTB &= ~((1 << PB0) | (1 << PB2));
}

void left(void) {
	PORTB |= (1 << PB0) | (1 << PB2);
	PORTB &= ~((1 << PB1) | (1 << PB3));
}

void right(void) {
	PORTD |= (1 << PD4) | (1 << PD6);
	PORTD &= ~((1 << PD5) | (1 << PD7));
}
