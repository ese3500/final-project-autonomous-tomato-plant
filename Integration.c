#define F_CPU 16000000UL
#define BAUD_RATE 9600

#include <avr/io.h>
#include "ST7735.h"
#include "LCD_GFX.h"
#include <stdlib.h>
#include <avr/interrupt.h>
#include <math.h>
#include <stdio.h>
#include <util/delay.h>
#include "uart.h"
#include "ldr.h"
#include "motor_control.h"

// Initialize variables for the ADC readings of the LDRs
uint16_t adc0; // Bottom left LDR
uint16_t adc1; // Bottom right LDR
uint16_t adc2; // Top right LDR
uint16_t adc3; // Top left LDR

uint16_t adc_max;// LDR value with max ADC reading - need for MoveCar()
uint16_t adc_max;// LDR value with min ADC reading - need for MoveCar()

void Initialize()
{
	cli(); // Disable global interrupts

	// Initialize input pins for ESP motor driving functionality
	DDRE &= ~(1<<DDRE0); // Virtual pin 0 maps to MCU PE0 and calls Forward()
	DDRE &= ~(1<<DDRE1); // Virtual pin 0 maps to MCU PE1 and calls Reverse()
	DDRE &= ~(1<<DDRE2); // Virtual pin 0 maps to MCU PE2 and calls Left()
	DDRE &= ~(1<<DDRE3); // Virtual pin 0 maps to MCU PE3 and calls Right()
	DDRB &= ~(1<<DDRB4); // Virtual pin 0 maps to MCU PB4 and calls Drift_Left()
	DDRB &= ~(1<<DDRB5); // Virtual pin 0 maps to MCU PB5 and calls Drift_Right()

	// Initialize input pins for collision bumpers 
	DDRC &= ~(1<<DDRC0); // Front of the car (PCINT8)
	DDRC &= ~(1<<DDRC1); // Bottom of the car (PCINT9)
	DDRC &= ~(1<<DDRC2); // Left of the car (PCINT10)
	DDRC &= ~(1<<DDRC3); // Right of the car (PCINT11)

	// Enable pin change interrupt for PCINT[14:8]
	PCICR |= (1<<PCIE1);
	
	// Enable pin change interrupts for collision bumper input pins
	PCMSK1 |= (1<<PCINT8);
	PCMSK2 |= (1<<PCINT9);
	PCMSK2 |= (1<<PCINT10);
	PCMSK2 |= (1<<PCINT11);
	
	sei(); // Enable global interrupts
}

void Collision() {
	Stop();
	LCD_setScreen(0xFF0F); 
	LCD_drawString(1, 1, "Collision! Please move me.", 0x0000, 0xFF0F);
}

ISR(PCINT8_vect) { 
	Collision();
}

ISR(PCINT9_vect) { 
	Collision();
}

ISR(PCINT10_vect) { 
	Collision();
}

ISR(PCINT11_vect) { 
	Collision();
}

void Check_Bumpers() {
	DDRD |= (1<<DDRD2); // Front of the car
	DDRD |= (1<<DDRD3); // Bottom of the car
	DDRB |= (1<<DDRB4); // Left of the car
	DDRB |= (1<<DDRB5); // Right of the car

	if (!(PINE & (1<<PINE0))) { // If any of the pins 
		Forward();
	} else if (PINE & (1<<PINE1))
}

void Wifi_Manual_Motor_Control() {
	if (PINE & (1<<PINE0)) { // If PE0 is high, move the car forward
		Forward();
	} else if (PINE & (1<<PINE1)) { // If PE1 is high, move the car reverse
		Reverse();
	} else if (PINE & (1<<PINE2)) { // If PE2 is high, move the car left
		Left();
	} else if (PINE & (1<<PINE3)) { // If PE3 is high, move the car right
		Right();
	} else if (PINB & (1<<PINB0)) { // If PB0 is high, drift the car left
		Drift_Left();
	} else if (PINB & (1<<PINB1)) { // If PB1 is high, drift the car right
		Drift_Right();
	} else {
		Stop();
	}
}

void Init_motor_pins(void) {
	// Set all motor control pins to output
	DDRD |= (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7);
	DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3);
}

void Forward(void) {
	PORTD |= (1 << PD4) | (1 << PD6);
	PORTD &= ~((1 << PD5) | (1 << PD7));

	PORTB |= (1 << PB0) | (1 << PB2);
	PORTB &= ~((1 << PB1) | (1 << PB3));
}

void Reverse(void) {
	PORTD |= (1 << PD5) | (1 << PD7);
	PORTD &= ~((1 << PD4) | (1 << PD6));

	PORTB |= (1 << PB1) | (1 << PB3);
	PORTB &= ~((1 << PB0) | (1 << PB2));
}

void Left(void) {
	PORTB |= (1 << PB0) | (1 << PB2);
	PORTB &= ~((1 << PB1) | (1 << PB3));
}

void Right(void) {
	PORTD |= (1 << PD4) | (1 << PD6);
	PORTD &= ~((1 << PD5) | (1 << PD7));
}

void Stop(void) {
	PORTD &= ~((1 << PD4) | (1 << PD6));
	PORTD &= ~((1 << PD5) | (1 << PD7));

	PORTB &= ~((1 << PB0) | (1 << PB2));
	PORTB &= ~((1 << PB1) | (1 << PB3));
}


void ADC_init() {
	ADMUX = (1 << REFS0);  // Select AVCC as the reference

	// Enable ADC and set prescaler to 128 for adequate conversion speed
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t ADC_read(uint8_t channel) {
	// Select ADC channel with safety mask
	ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);

	// Start single conversion
	ADCSRA |= (1 << ADSC);

	// Wait for conversion to complete
	while (ADCSRA & (1 << ADSC));

	// Get the ADC result
	return ADC;
}

void ReadADCForLDR(void) {
	adc0 = ADC_read(0);
	adc1 = ADC_read(1);
	adc2 = ADC_read(2);
	adc3 = ADC_read(3);
	sprintf(msg, "ADC0: %d \r\n", adc0);
	UART_putstring(msg);
	sprintf(msg, "ADC1: %d \r\n", adc1);
	UART_putstring(msg);	
	sprintf(msg, "ADC2: %d \r\n", adc2);
	UART_putstring(msg);	
	sprintf(msg, "ADC3: %d \r\n \r\n", adc3);
	UART_putstring(msg);
		
	_delay_ms(1000);	
}

void MoveCar() {
	adc_max = max(adc0, adc1, adc2, adc3);
	adc_min = min(adc0, adc1, adc2, adc3);

	if (abs((adc_max - adc_min) <= 75)) {
		// Falls in the deadzone range: if the ADC readings of the LDRs are all around the same range, don't move the car	
		Stop();
	} else if (adc_max == 0) { // Bottom left LDR
		Reverse();
		Left();
		Reverse();
		Left();
		Reverse();
		Left();
	} else if (adc_max == 1) { // Bottom right LDR
		Reverse();
		Right();
		Reverse();
		Right();
		Reverse();
		Right();
	} else if (adc_max == 2) { // Top right LDR
		Forward();
		Right();
		Forward();
		Right();
		Forward();
		Right();
	} else if (adc_max == 3) { // Top left LDR
		Forward();
		Left();
		Forward();
		Left();
		Forward();
		Left();
	}
}

int main(void)
{
	Initialize();

  	ADC_init();
	Init_motor_pins();
		
	// UART_init(BAUD_PRESCALER);
	// sprintf(msg, "Initialized!\r\n");
	// UART_putstring(msg);

	while(1){
    		ReadADCForLDR();
		MoveCar();
		Wifi_Manual_Motor_Control();

		moisture_value_ADC = ADC;

	}
}
