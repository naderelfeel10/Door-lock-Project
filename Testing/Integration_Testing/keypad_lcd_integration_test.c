
#include "lcd.h"
#include "keypad.h"

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