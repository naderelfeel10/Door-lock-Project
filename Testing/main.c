#include "keypad.h"
#include "dio.h"
#include <stdio.h>
#include "systick.h"
#include "Buzzer.h"
#include "motor.h"
#include "uart.h"


#include "keyPad_test.h"
#include "LCD_test.h"
#include "buzzer_test.h"
#include "motor_test.h"
#include "uart_test.h"

#include "keypad_motor_integration_test.h"
#include "keypad_lcd_integration_test.h"
#include "keypad_buzzer_integration_test.h"

// unit testing 
#define keypad_unit;
#define lcd_unit;
#define potentiometer_unit;
#define buzzer_unit;
#define motor_unit;

#define uart_send_unit;
#define uart_receive_unit;


// integration testing :

#define keypad_lcd_integration;
#define keypad_motor_integration;
#define keypad_buzzer_integration





int main()
{
      SysTick_Init(16000, SYSTICK_NOINT);  
      UART0_Init();

#ifdef keypad_unit
    keypad_test();
#endif
    

#ifdef lcd_unit
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
 
#ifdef uart_send_unit
 uart_send_test();
#endif
 
#ifdef uart_receive_unit
 uart_receive_test();
#endif
    while(1); 
    
}