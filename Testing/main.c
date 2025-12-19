/*****************************************************************************
 * File: main.c
 * Description: Example main for testing the 4x4 keypad driver
 * Author: Ahmedhh
 * Date: November 27, 2025
 *****************************************************************************/

#include "keypad.h"
#include "dio.h"
#include <stdio.h>
#include "systick.h"
#include "Buzzer.h"
#include "motor.h"


// unit testing 
//#define keypad_unit;
//#define lcd_unit;
//#define potentiometer_unit;
//#define buzzer_unit;
//#define motor_unit;


// integration testing :

//#define keypad_lcd_integration;
//#define keypad_motor_integration;
#define keypad_buzzer_integration


#ifdef keypad_unit
char keypad_target[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','*','#'};
#endif

#ifdef lcd_unit
void LCD_test(){
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
}
#endif

#ifdef potentiometer_unit
   /* Thresholds for PASS/FAIL */
#define RAW_MIN 0
#define RAW_MAX 4095        // Assuming 12-bit ADC
#define PERCENT_MIN 0
#define PERCENT_MAX 100
#define VOLTAGE_MIN 0       // in mV
#define VOLTAGE_MAX 3300
#define MAPPED_MIN 0
#define MAPPED_MAX 9


void Potentiometer_test(){
  
    uint16_t rawValue;
    uint8_t percentage;
    uint32_t voltage;
    uint32_t mapped;
    
    uint32_t mappedMin = 0;
    uint32_t mappedMax = 9;

    printf("Starting Potentiometer Unit Test...\n");


    
    rawValue = POT_ReadRaw();
    if(rawValue > 0) printf("RAW: %u -> PASS\n", rawValue);
    
    else printf("RAW: %u -> FAIL\n", rawValue);


    percentage = POT_ReadPercentage();
    
    
    if(percentage <= 100) printf("PERCENTAGE: %u%% -> PASS\n", percentage);
    else printf("PERCENTAGE: %u%% -> FAIL\n", percentage);


    voltage = POT_ReadMillivolts();
    if(voltage <= 3300) printf("VOLTAGE: %lu mV -> PASS\n", voltage);
    else printf("VOLTAGE: %lu mV -> FAIL\n", voltage);

    

    mapped = mappedMin + ((uint32_t)rawValue * (mappedMax - mappedMin)) / 1023;
    if(mapped >= mappedMin && mapped <= mappedMax)
        printf("MAPPED: %lu -> PASS\n", mapped);
    
    else {
      printf("MAPPED: %lu -> FAIL\n", mapped);}

    printf("Potentiometer Unit Test Complete.\n");
}
#endif


#ifdef buzzer_unit
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
#endif


#ifdef motor_unit
void motor_test(){
  printf("motor unit test  \n");
  
  Motor_Init();
  
  Motor_RotateCW();
  DelayMs(600);
  Motor_Stop();
  
  printf("motor Rotate CW for 600 ms  -> PASS\n");
    printf("motor stop  -> PASS\n");
    
  Motor_RotateCCW();
  DelayMs(600);
  Motor_Stop();  
    printf("motor Rotate CCW  for 600 ms  -> PASS\n");
  
    

  printf("motor unit test completed .  \n");

       
}
#endif




#ifdef keypad_lcd_integration
 void keypad_lcd_test(){
   printf("keypad integration with lcd \n");
    char key;
    uint8_t cursor_col = 0;
    uint8_t cursor_row = 0;
        
    Keypad_Init();
    LCD_Init();
    

    LCD_Clear();
    LCD_SetCursor(0, 0);
    char keypad_buffer[16] = {0};
    int i = 0;
    while(i<=15)
    {
        key = Keypad_GetKey();
        if (key != 0)
        {
                keypad_buffer[i] = key;

                /* Display the character */
                LCD_WriteChar(key);
                cursor_col++;
                
                /* Handle line wrapping */
                if (cursor_col >= 16)
                {
                    if (cursor_row == 0)
                    {
                        /* Move to second line */
                        cursor_row = 1;
                        cursor_col = 0;
                        LCD_SetCursor(cursor_row, cursor_col);
                    }
                    else
                    {
                        /* Wrap to first line */
                        cursor_row = 0;
                        cursor_col = 0;
                        LCD_SetCursor(cursor_row, cursor_col);
                    }
                }
                i++;
            }
            
            /* Debounce delay */
            DelayMs(300);
       
    }
    
    printf("Keypad input : %s\n", keypad_buffer);
    
    
    if(LCD_VerifyString(0,0,keypad_buffer))
    printf("Success\n");
       else
    printf("Fail\n");
}
#endif


#ifdef keypad_motor_integration
 void keypad_motor_test(){
   printf("keypad integration with motor \n");
    char key;
        
    Keypad_Init();
    Motor_Init();
    
    while(1)
    {
        key = Keypad_GetKey();
        if (key != 0)
        {
          if(key == '1'){
            Motor_RotateCW();
            
            printf("Motor rotates CW when 1 is pressed on keypad >> PASS \n");
            DelayMs(300);
          }
          else if(key == '2'){
            Motor_RotateCCW();
            printf("Motor rotates CCW when 2 is pressed on keypad >> PASS \n");
            DelayMs(300);

          }
           else if(key == '0'){
            Motor_Stop();
            printf("Motor stopped when 0 is pressed on keypad >> PASS\n");
            
          }
        }    

    }
          printf("Motor integration with keypad");

}
#endif

#ifdef keypad_buzzer_integration
 void keypad_buzzer_test(){
   printf("keypad integration with buzzer \n");
    char key;
        
    Keypad_Init();
    Buzzer_Init();
    
    while(1)
    {
        key = Keypad_GetKey();
        if (key != 0)
        {
          if(key == '1'){
            Buzzer_On();
            
            printf("buzzer is On when 1 is pressed on keypad >> PASS \n");
          }
          else if(key == '2'){
            Buzzer_Off();
            printf("buzzer is Off when 2 is pressed on keypad >> PASS \n");
          }
           else if(key == '0'){
            Buzzer_Beep(2000);
            printf("buzzer runs for 2 seconds   when 0 is pressed on keypad >> PASS\n");
            
          }
        }    

    }
          printf("Buzzer integration with keypad");

}
#endif

int main()
{
      SysTick_Init(16000, SYSTICK_NOINT);  
      

#ifdef keypad_unit
  printf("KEYPAD test Results \n");
    Keypad_Init(); // Initialize keypad pins
    char keypad_output[16] = {0};
    int i =0;
    while (i<=15)
    {
        char key = Keypad_GetKey(); // Get pressed key
        if (key)
        {
          keypad_output[i] = key;
          i +=1;
          printf("%c\n", key); // Output key to console
        }
    }
    for(int i=0;i<16;i++){
      if(keypad_output[i]!=keypad_target[i]){printf("FAIL");}
    }
    printf("SUCCESS");
#endif
    

#ifdef lcd_unit
    printf("LCD test Results \n");
    LCD_test();
#endif    
#ifdef potentiometer_unit
    Potentiometer_test();
#endif

    
#ifdef buzzer_unit
    buzzer_test();
#endif  
 
#ifdef motor_unit
    motor_test();
#endif     


#ifdef keypad_lcd_integration
 keypad_lcd_test();
#endif
 
#ifdef keypad_motor_integration
 keypad_motor_test();
#endif

#ifdef keypad_buzzer_integration
 keypad_buzzer_test();
#endif  
    while(1); 
    
}