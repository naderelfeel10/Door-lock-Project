/*************************************************
 * File: GPTM_TIMER0.c
 * Description: GPTM Timer0A driver implementation
 *************************************************/

#include "GPTM_TIMER0.h"
#include "tm4c123gh6pm.h"

void GPTM_Timer0A_Init(uint32_t reloadValue)
{
    volatile uint32_t delay;

    /* Enable Timer0 clock */
    SYSCTL_RCGCTIMER_R |= 0x01;
    delay = SYSCTL_RCGCTIMER_R;

    /* Disable Timer0A */
    TIMER0_CTL_R &= ~0x01;

    /* Configure as 32-bit timer */
    TIMER0_CFG_R = 0x00;

    /* Periodic mode, count up */
    TIMER0_TAMR_R = 0x12;

    /* Load interval value */
    TIMER0_TAILR_R = reloadValue;
    TIMER0_TAV_R   = 0;

    /* Clear timeout flag */
    TIMER0_ICR_R = 0x01;

    /* Enable Timer0A */
    TIMER0_CTL_R |= 0x01;
}

uint8_t GPTM_Timer0A_TimeOut(void)
{
    return (TIMER0_RIS_R & 0x01);
}

void GPTM_Timer0A_ClearFlag(void)
{
    TIMER0_ICR_R = 0x01;
}
