#include "systick.h"
#include "lcd.h"


void LCD_test(){
      printf("LCD test Results \n");

    /* Initialize SysTick for delays (16MHz system clock) */
    SysTick_Init(16000, SYSTICK_NOINT);  /* 1ms delay */
    
    /* Initialize peripherals */
    LCD_Init();
    
    /* Display welcome message */
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_WriteString("TEST!");
    if(LCD_VerifyString(0,0,"TEST!"))
    printf("Success\n");
       else
    printf("Fail\n");
    DelayMs(3000);
    printf("LCD unit test completed\n");

}
