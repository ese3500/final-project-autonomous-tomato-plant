#define F_CPU 16000000UL

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "uart.h"
#include "ldr.h"
#include "motor_control.h"


//stuff for uart print
#define BAUD_RATE 9600
#define BAUD_PRESCALER (((F_CPU / (BAUD_RATE * 16UL))) - 1)

char msg[50];
uint16_t adc0;
uint16_t adc1;
uint16_t adc2;
uint16_t adc3;

void adc(void) {
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


int main(void)
{
	ADC_init();
	init_motor_pins();
		
	UART_init(BAUD_PRESCALER);
	sprintf(msg, "Initialized!\r\n");
	UART_putstring(msg);

    while (1) 
    {
		adc();
		forward();

    }
}

