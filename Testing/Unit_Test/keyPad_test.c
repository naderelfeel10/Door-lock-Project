
#include "keypad.h"

char keypad_target[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','*','#'};


void keypad_test(void){
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
      if(keypad_output[i]!=keypad_target[i]){printf("FAIL");break;}
    }
    printf("SUCCESS");

}