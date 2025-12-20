
#include "tm4c123gh6pm.h"
#include "systick.h"

/* Buzzer pin: PA7 */
#define BUZZER_PORT 0
#define BUZZER_PIN  7

void Buzzer_Init(void)
{
    volatile uint32_t delay;
    
    /* Enable clock for PORTA */
    SYSCTL_RCGCGPIO_R |= (1 << BUZZER_PORT);
    delay = SYSCTL_RCGCGPIO_R; // Delay for stabilization
    delay = SYSCTL_RCGCGPIO_R;
    
    /* Unlock PA7 if needed */
    GPIO_PORTA_LOCK_R = 0x4C4F434B;
    GPIO_PORTA_CR_R |= (1 << BUZZER_PIN);
    
    /* Disable alternate function and analog */
    GPIO_PORTA_AFSEL_R &= ~(1 << BUZZER_PIN);
    GPIO_PORTA_AMSEL_R &= ~(1 << BUZZER_PIN);
    
    /* Set direction to output */
    GPIO_PORTA_DIR_R |= (1 << BUZZER_PIN);
    
    /* Enable digital function */
    GPIO_PORTA_DEN_R |= (1 << BUZZER_PIN);
    
    /* Start with buzzer off */
    GPIO_PORTA_DATA_R &= ~(1 << BUZZER_PIN);
    GPIO_PORTA_LOCK_R = 0; // Lock again
}


void Buzzer_On(void)
{
    GPIO_PORTA_DATA_R |= (1 << BUZZER_PIN);
}


void Buzzer_Off(void)
{
    GPIO_PORTA_DATA_R &= ~(1 << BUZZER_PIN);
}


void Buzzer_Toggle(void)
{
    GPIO_PORTA_DATA_R ^= (1 << BUZZER_PIN);
}


void Buzzer_Beep(uint16_t duration_ms)
{
    Buzzer_On();
    DelayMs(duration_ms);
    Buzzer_Off();
}
