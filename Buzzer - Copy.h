#include "Buzzer.h"
#include "dio.h"
#include "systick.h"

#define BUZZER_PORT PORTA
#define BUZZER_PIN  PIN7

void Buzzer_Init(void)
{
    DIO_Init(BUZZER_PORT, BUZZER_PIN, OUTPUT);
    DIO_WritePin(BUZZER_PORT, BUZZER_PIN, LOW);
}

void Buzzer_On(void)
{
    DIO_WritePin(BUZZER_PORT, BUZZER_PIN, HIGH);
}

void Buzzer_Off(void)
{
    DIO_WritePin(BUZZER_PORT, BUZZER_PIN, LOW);
}

void Buzzer_Beep(uint32_t duration_ms)
{
    Buzzer_On();
    DelayMs(duration_ms);
    Buzzer_Off();
}
