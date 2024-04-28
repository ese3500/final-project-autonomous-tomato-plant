#include "motor_control.h"

void init_motor_pins(void) {
	// Set all motor control pins to output
	DDRD |= (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7);
	DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3);
	
	// Initialize input pins for ESP motor driving functionality
	DDRE &= ~(1<<DDRE0); // Virtual pin 0 maps to MCU PE0 and calls Forward()
	DDRE &= ~(1<<DDRE1); // Virtual pin 0 maps to MCU PE1 and calls Reverse()
	DDRE &= ~(1<<DDRE2); // Virtual pin 0 maps to MCU PE2 and calls Left()
	DDRE &= ~(1<<DDRE3); // Virtual pin 0 maps to MCU PE3 and calls Right()
	DDRD &= ~(1<<DDRB4); // Virtual pin 0 maps to MCU PD0 and calls Drift_Left()
	DDRD &= ~(1<<DDRB5); // Virtual pin 0 maps to MCU PD1 and calls Drift_Right()

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
	
	PORTD &= ~((1 << PD4) | (1 << PD6));
	PORTD &= ~((1 << PD5) | (1 << PD7));
}

void right(void) {
	PORTD |= (1 << PD4) | (1 << PD6);
	PORTD &= ~((1 << PD5) | (1 << PD7));
	
	PORTB &= ~((1 << PB0) | (1 << PB2));
	PORTB &= ~((1 << PB1) | (1 << PB3));
}

void stop(void) {
	PORTD &= ~((1 << PD4) | (1 << PD6));
	PORTD &= ~((1 << PD5) | (1 << PD7));

	PORTB &= ~((1 << PB0) | (1 << PB2));
	PORTB &= ~((1 << PB1) | (1 << PB3));
}

void drift_left(void) {
	PORTD |= (1 << PD4) | (1 << PD7);
	PORTD &= ~((1 << PD6) | (1 << PD5));

	PORTB |= (1 << PB1) | (1 << PB2);
	PORTB &= ~((1 << PB0) | (1 << PB3));
}

void drift_right(void) {
	PORTD |= (1 << PD6) | (1 << PD5);
	PORTD &= ~((1 << PD4) | (1 << PD7));

	PORTB |= (1 << PB0) | (1 << PB3);
	PORTB &= ~((1 << PB1) | (1 << PB2));
}
