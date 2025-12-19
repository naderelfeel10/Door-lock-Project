

#ifndef MOTOR_H_
#define MOTOR_H_

#include <stdint.h>

/******************************************************************************
 * Function Prototypes
 * API for Motor control.
 ******************************************************************************/

/*
 * Motor_Init
 * Initializes the motor control pins (PF0 and PF4) as outputs.
 * Must be called before using other motor functions.
 */
void Motor_Init(void);

/*
 * Motor_RotateCW
 * Rotates the motor clockwise.
 */
void Motor_RotateCW(void);

/*
 * Motor_RotateCCW
 * Rotates the motor counter-clockwise.
 */
void Motor_RotateCCW(void);

/*
 * Motor_Stop
 * Stops the motor completely.
 */
void Motor_Stop(void);

/* Timer-based control */
void Motor_RotateCW_Time(uint32_t time_ms);
void Motor_RotateCCW_Time(uint32_t time_ms);

#endif /* MOTOR_H_ */