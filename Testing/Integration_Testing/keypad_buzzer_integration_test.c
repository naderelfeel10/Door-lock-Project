#include "buzzer.h"
#include "keypad.h"


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
