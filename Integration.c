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

// Enumeration for face types
typedef enum {
	FROWNY = 0,
	SMILEY = 1,
	DROWNING = 2
} FaceType;

FaceType face = FROWNY;  // Set the initial face to frowny

int moisture_value_ADC = 0; // The moisture is measured by ADC

// Metrics to display on OLED (dim, normal, bright)
typedef enum {
	DIM = 0,
	NORMAL = 1,
	BRIGHT = 2,
} LightType;

LightType light_metric_display = NORMAL;  // Set the initial light metric to normal

//typedef enum {
	//DRY = 0,
	//NORMAL = 1
	//SOGGY = 2,
//} MoistureType;
//
//LightType moisture_metric_display = NORMAL;  // Set the initial moisture metric to normal

void Initialize()
{
	cli(); // Disable global interrupts

	// Initialize input pins for ESP motor driving functionality
	DDRE &= ~(1<<DDRE0); // Virtual pin 0 maps to MCU PE0 and calls Forward()
	DDRE &= ~(1<<DDRE1); // Virtual pin 0 maps to MCU PE1 and calls Reverse()
	DDRE &= ~(1<<DDRE2); // Virtual pin 0 maps to MCU PE2 and calls Left()
	DDRE &= ~(1<<DDRE3); // Virtual pin 0 maps to MCU PE3 and calls Right()
	DDRD &= ~(1<<DDRD0); // Virtual pin 0 maps to MCU PD0 and calls Drift_Left()
	DDRD &= ~(1<<DDRD1); // Virtual pin 0 maps to MCU PD1 and calls Drift_Right()

	// Initialize input pins for collision bumpers 
	DDRD &= ~(1<<DDRD0); // Front of the car (PCINT16)
	DDRD &= ~(1<<DDRD1); // Bottom of the car (PCINT17)
	DDRD &= ~(1<<DDRD2); // Left of the car (PCINT18)
	DDRC &= ~(1<<DDRC4); // Right of the car (PCINT12)

	// Enable pin change interrupt for PCINT[14:8] and PCINT[23:16]
	PCICR |= (1<<PCIE1);
	PCICR |= (1<<PCIE2);
	
	// Enable pin change interrupts for collision bumper input pins
	PCMSK1 |= (1<<PCINT12);
	PCMSK2 |= (1<<PCINT16);
	PCMSK2 |= (1<<PCINT17);
	PCMSK2 |= (1<<PCINT18);
	
	// Moisture sensor input pin
	DDRD &= ~(1<<DDRD0);

	lcd_init();
	
	// Setup for ADC (10bit = 0-1023)
	// Clear power reduction bit for ADC
	PRR0 &= ~(1 << PRADC);

	// Select Vref = AVcc
	ADMUX |= (1 << REFS0);
	ADMUX &= ~(1 << REFS1);

	// Set the ADC clock div by 128
	// 16M/128=125kHz
	ADCSRA |= (1 << ADPS0);
	ADCSRA |= (1 << ADPS1);
	ADCSRA |= (1 << ADPS2);

	// Select Channel ADC0 (pin C0)
	ADMUX &= ~(1 << MUX0);
	ADMUX &= ~(1 << MUX1);
	ADMUX &= ~(1 << MUX2);
	ADMUX &= ~(1 << MUX3);

	ADCSRA |= (1 << ADATE);   // Autotriggering of ADC

	// Free running mode ADTS[2:0] = 000
	ADCSRB &= ~(1 << ADTS0);
	ADCSRB &= ~(1 << ADTS1);
	ADCSRB &= ~(1 << ADTS2);

	// Disable digital input buffer on ADC pin
	DIDR0 |= (1 << ADC0D);

	// Enable ADC
	ADCSRA |= (1 << ADEN);

	// Start conversion
	ADCSRA |= (1 << ADSC);
	
	sei(); // Enable global interrupts
}

ISR(PCINT12_vect) { 
	Stop();
}

ISR(PCINT16_vect) { 
	Stop();
}

ISR(PCINT17_vect) { 
	Stop();
}

ISR(PCINT18_vect) { 
	Stop();
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

void Smiley() {
	LCD_drawString(1, 1, "I'm happy and growing some basil", 0x0000, 0xFFFF);
	LCD_drawCircle(80, 73, 50, 0xFF0F); // Face
	LCD_drawCircle(55, 55, 6, 0x0000); // Eyes
	LCD_drawCircle(105, 55, 6, 0x0000);
	LCD_drawCircle(80, 90, 20, 0xF00F); // Mouth
	LCD_drawBlock(60, 70, 100, 90, 0xFF0F);
}

void Frowny() {
	LCD_drawString(1, 1, "Give me water!", 0x0000, 0xFFFF);
	LCD_drawCircle(80, 73, 50, 0xFF0F); // Face
	LCD_drawCircle(55, 55, 6, 0x0000); // Eyes
	LCD_drawCircle(105, 55, 6, 0x0000);
	LCD_drawCircle(80, 95, 20, 0xF00F); // Mouth
	LCD_drawBlock(60, 95, 100, 115, 0xFF0F);
}

void Drowning() {
	LCD_drawString(1, 1, "I'm drowning! Help!!", 0x0000, 0xFFFF);
	LCD_drawCircle(80, 73, 50, 0x00FF); // Face
	LCD_drawCircle(55, 55, 6, 0x0000); // Eyes
	LCD_drawCircle(105, 55, 6, 0x0000);
	LCD_drawCircle(80, 95, 20, 0xF00F); // Mouth
	LCD_drawBlock(60, 95, 100, 115, 0x00FF);
}

void Update_Light_Metric() {
	if (light_metric_display == DIM) {
		LCD_drawString(1, 10, "Light: Dim", 0x0000, 0xFFFF);
	} else if (light_metric_display == NORMAL) {
		LCD_drawString(1, 10, "Light: Normal", 0x0000, 0xFFFF);
	} else {
		LCD_drawString(1, 10, "Light: High", 0x0000, 0xFFFF);
	}
}

//void Update_Moisture_Metric() {
	//if (moisture_metric_display == DRY) {
		//LCD_drawString(1, 10, "Moisture Level: Dry", 0x0000, 0xFFFF);
	//} else if (moisture_metric_display == NORMAL) {
		//LCD_drawString(1, 10, "Moisture Level: Normal", 0x0000, 0xFFFF);
	//} else {
		//LCD_drawString(1, 10, "Moisture Level: Soggy", 0x0000, 0xFFFF);
	//}
//}

int main(void)
{
	Initialize();
	LCD_setScreen(0xFFFF); 

  ADC_init();
	Init_motor_pins();
		
	// UART_init(BAUD_PRESCALER);
	// sprintf(msg, "Initialized!\r\n");
	// UART_putstring(msg);

	while(1){
    		ReadADCForLDR();
		Update_Light_Metric();
		MoveCar();
		Wifi_Manual_Motor_Control();

		moisture_value_ADC = ADC;

		// We want the moisture value to be between 530-570. Anything above is too dry, and anything below is too wet.
		if (moisture_value_ADC > 570) {
			if (face == FROWNY) {
				// do nothing
			} else {
				face = FROWNY;
				Frowny();
			}
		} else if (moisture_value_ADC > 530) {
			if (face == SMILEY) {
				// do nothing
			} else {
				face = SMILEY;
				Smiley();
			}
		} else {
			if (face == DROWNING) {
				// do nothing
			} else {
				face = DROWNING;
				Drowning();
			}
		}
	}
}
