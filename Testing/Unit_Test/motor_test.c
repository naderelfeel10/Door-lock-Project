#include "systick.h"
#include "motor.h"


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
