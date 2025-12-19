#include "Buzzer.h"
#include "GPTM_TIMER0.h"
#include "tm4c123gh6pm.h"

/* Buzzer pin: PA7 */
#define BUZZER_PORT 0
#define BUZZER_PIN  7
#define TIMER0_1MS_RELOAD 16000

void Buzzer_Init(void)
{
    volatile uint32_t delay;

    SYSCTL_RCGCGPIO_R |= (1 << BUZZER_PORT);
    delay = SYSCTL_RCGCGPIO_R;

    GPIO_PORTA_DIR_R |= (1 << BUZZER_PIN);
    GPIO_PORTA_DEN_R |= (1 << BUZZER_PIN);
    GPIO_PORTA_AFSEL_R &= ~(1 << BUZZER_PIN);
    GPIO_PORTA_AMSEL_R &= ~(1 << BUZZER_PIN);

    GPIO_PORTA_DATA_R &= ~(1 << BUZZER_PIN);
}

void Buzzer_On(void)
{
    GPIO_PORTA_DATA_R |= (1 << BUZZER_PIN);
}

void Buzzer_Off(void)
{
    GPIO_PORTA_DATA_R &= ~(1 << BUZZER_PIN);
}

/* ? Buzzer beep using GPTM Timer0 */
void Buzzer_Beep(uint32_t duration_ms)
{
    Buzzer_On();

    GPTM_Timer0A_Init(duration_ms * TIMER0_1MS_RELOAD);

    while (!GPTM_Timer0A_TimeOut());
    GPTM_Timer0A_ClearFlag();

    Buzzer_Off();
}
