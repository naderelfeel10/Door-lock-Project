#include "motor.h"
#include "dio.h"
#include "GPTM_TIMER0.h"

/******************************************************************************
 *                              Pin Configuration                              *
 ******************************************************************************/

/*
 * Motor pins connected to Port D
 * IN1 -> PD0
 * IN2 -> PD1
 */
#define MOTOR_PORT      PORTF
#define MOTOR_IN1       PIN0
#define MOTOR_IN2       PIN4

/* Timer configuration */
#define TIMER0_1MS_RELOAD   16000   /* 16 MHz system clock */

/******************************************************************************
 *                          Function Definitions                               *
 ******************************************************************************/

/* Initialize motor pins */
void Motor_Init(void)
{
    DIO_Init(MOTOR_PORT, MOTOR_IN1, OUTPUT);
    DIO_Init(MOTOR_PORT, MOTOR_IN2, OUTPUT);

    Motor_Stop();
}

/* Rotate motor clockwise */
void Motor_RotateCW(void)
{
    DIO_WritePin(MOTOR_PORT, MOTOR_IN1, HIGH);
    DIO_WritePin(MOTOR_PORT, MOTOR_IN2, LOW);
}

/* Rotate motor counter-clockwise */
void Motor_RotateCCW(void)
{
    DIO_WritePin(MOTOR_PORT, MOTOR_IN1, LOW);
    DIO_WritePin(MOTOR_PORT, MOTOR_IN2, HIGH);
}

/* Stop motor */
void Motor_Stop(void)
{
    DIO_WritePin(MOTOR_PORT, MOTOR_IN1, LOW);
    DIO_WritePin(MOTOR_PORT, MOTOR_IN2, LOW);
}

/******************************************************************************
 *                      Timer-Based Motor Control                        *
 ******************************************************************************/

/* Rotate CW for a specific time (ms) */
void Motor_RotateCW_Time(uint32_t time_ms)
{
    Motor_RotateCW();

    GPTM_Timer0A_Init(time_ms * TIMER0_1MS_RELOAD);
    while (!GPTM_Timer0A_TimeOut());

    GPTM_Timer0A_ClearFlag();
    Motor_Stop();
}

/* Rotate CCW for a specific time (ms) */
void Motor_RotateCCW_Time(uint32_t time_ms)
{
    Motor_RotateCCW();

    GPTM_Timer0A_Init(time_ms * TIMER0_1MS_RELOAD);
    while (!GPTM_Timer0A_TimeOut());

    GPTM_Timer0A_ClearFlag();
    Motor_Stop();
}
