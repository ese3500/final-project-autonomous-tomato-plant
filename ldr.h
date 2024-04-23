#ifndef LDR_H
#define LDR_H

#include <stdint.h>

void ADC_init(void);
uint16_t ADC_read(uint8_t channel);

#endif // LDR_H
