/*************************************************
 * File: GPTM_TIMER0.h
 * Description: GPTM Timer0A driver (32-bit periodic mode)
 *************************************************/

#ifndef GPTM_TIMER0_H
#define GPTM_TIMER0_H

#include <stdint.h>

/* Public APIs */
void GPTM_Timer0A_Init(uint32_t reloadValue);
uint8_t GPTM_Timer0A_TimeOut(void);
void GPTM_Timer0A_ClearFlag(void);

#endif /* GPTM_TIMER0_H */
