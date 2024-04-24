#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <avr/io.h>

extern void init_motor_pins(void);
extern void forward(void);
extern void reverse(void);
extern void left(void);
extern void right(void);
extern void stop(void);
extern void drift_left(void);
extern void drift_right(void);

#endif // MOTOR_CONTROL_H
