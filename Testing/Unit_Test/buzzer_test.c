
#include "Buzzer.h"
#include "systick.h"
void buzzer_test(void)
{
  printf("buzzer unit test : \n");

    Buzzer_Init();

   
    Buzzer_On();
    DelayMs(200); 
    Buzzer_Off();

    printf("Buzzer ON/OFF -> PASS\n");

    Buzzer_Beep(300);
    printf("Buzzer Beep 300ms -> PASS\n");

    // toggle 
    
    Buzzer_Off(); 
    Buzzer_Toggle();
    DelayMs(100);
    Buzzer_Toggle(); 
    printf("Buzzer Toggle -> PASS\n");

    printf("Buzzer Unit Test Completed \n");
}

