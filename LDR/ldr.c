#include "ldr.h"
#include <avr/io.h>

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
