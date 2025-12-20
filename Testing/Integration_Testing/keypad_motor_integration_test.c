#include "motor.h"
#include "keypad.h"


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
