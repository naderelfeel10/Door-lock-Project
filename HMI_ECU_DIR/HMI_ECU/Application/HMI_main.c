
/*HMI_ECU main file*/

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "tm4c123gh6pm.h"
#include "dio.h"
#include "lcd.h"
#include "keypad.h"
#include "systick.h"
#include "adc.h"
#include "potentiometer.h"
#include "uart.h"


/******************************************************************************
 *                              Definitions                                    *
 ******************************************************************************/
#define PASSWORD_LENGTH         5       /* 5-digit password */
#define PASSWORD_EEPROM_BLOCK   0       /* Store password in block 0 */
#define PASSWORD_EEPROM_OFFSET  0       /* Starting at offset 0 */
#define TIMEOUT_EEPROM_OFFSET   4       /* Store timeout at offset 4 */

#define MIN_TIMEOUT             5       /* Minimum timeout in seconds */
#define MAX_TIMEOUT             30      /* Maximum timeout in seconds */
#define MAX_ATTEMPTS            3       /* Maximum password attempts */
#define LOCKOUT_DURATION        10      /* Lockout duration in seconds */

/* Door LED Pins */
#define DOOR_LED_RED            PIN1    /* PF1 - Red (Locked) */
#define DOOR_LED_GREEN          PIN3    /* PF3 - Green (Unlocked) */
#define STATUS_LED_BLUE         PIN2    /* PF2 - Status/Feedback */

/* Application States */
typedef enum {
    STATE_INIT,
    STATE_SETUP_PASSWORD,
    STATE_CONFIRM_PASSWORD,
    STATE_MAIN_MENU,
    STATE_OPEN_DOOR_PASSWORD,
    STATE_CHANGE_OLD_PASSWORD,
    STATE_CHANGE_NEW_PASSWORD,
    STATE_CHANGE_CONFIRM_PASSWORD,
    STATE_ADJUST_TIMEOUT,
    STATE_TIMEOUT_PASSWORD,
    STATE_DOOR_UNLOCKED,
    STATE_LOCKOUT
} AppState;


/******************************************************************************
 *                          Global Variables                                   *
 ******************************************************************************/

static char password[PASSWORD_LENGTH + 1] = {0};          /* Current password buffer */
static char temp_password[PASSWORD_LENGTH + 1] = {0};      /* Temp password for confirmation */
//static char stored_password[PASSWORD_LENGTH + 1] = {0};    /* Password from EEPROM */
static char stored_password[PASSWORD_LENGTH + 1] = {0};    /* Password from EEPROM */
static uint8_t password_index = 0;
//static AppState current_state = STATE_INIT;
static AppState current_state = STATE_SETUP_PASSWORD;
static uint8_t attempt_count = 0;
static uint8_t auto_lock_timeout = 10;  /* Default 10 seconds */
static uint8_t pending_timeout = 10;     /* Temp value when adjusting */

/******************************************************************************
 *                          Function Prototypes                                *
 ******************************************************************************/

void System_Init(void);
//void Door_Init(void);
//void Door_Lock(void);
//void Door_Unlock(void);
void StatusLED_On(void);
void StatusLED_Off(void);
void StatusLED_Blink(uint8_t times);

void DisplayMainMenu(void);
void ClearPasswordBuffer(void);
//uint8_t StorePassword(const char* pwd);
//uint8_t RetrievePassword(char* pwd);
//uint8_t StoreTimeout(uint8_t timeout);
//uint8_t RetrieveTimeout(uint8_t* timeout);
uint8_t VerifyPassword(const char* input, const char* stored);
void ProcessKey(char key);
//void HandleDoorOperation(void);
//void HandleLockout(void);
uint8_t ReadPotentiometerTimeout(void);
void DisplayTimeoutValue(uint8_t timeout_val);

void System_Init(void)
{
    /* Initialize SysTick for delays */
    SysTick_Init(16000, SYSTICK_NOINT);
    
    /* Initialize LCD */
    LCD_Init();
    LCD_Clear();
    
    /* Initialize Keypad */
    Keypad_Init();
    //Motor_Init();
   
    /* Initialize Potentiometer */
    POT_Init();
      DIO_Init(PORTF, DOOR_LED_RED, OUTPUT);      /* Red LED - Locked */
    DIO_Init(PORTF, DOOR_LED_GREEN, OUTPUT);    /* Green LED - Unlocked */
    DIO_Init(PORTF, STATUS_LED_BLUE, OUTPUT);   /* Blue LED - Status */
}

void StatusLED_On(void)
{
    DIO_WritePin(PORTF, STATUS_LED_BLUE, HIGH);
}
/*
 * DisplayMainMenu
 * Displays the main menu on LCD.
 */
void DisplayMainMenu(void)
{
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_WriteString("A:Open B:Change");
    LCD_SetCursor(1, 0);
    LCD_WriteString("C:Time D:Reset");
}
/*
 * StatusLED_Off
 * Turns off the status LED (Blue).
 */
void StatusLED_Off(void)
{
    DIO_WritePin(PORTF, STATUS_LED_BLUE, LOW);
}
void StatusLED_Blink(uint8_t times)
{
    uint8_t i;
    for(i = 0; i < times; i++)
    {
        StatusLED_On();
        DelayMs(200);
        StatusLED_Off();
        DelayMs(200);
    }
}
/*
 * ClearPasswordBuffer
 * Clears the password buffers.
 */
void ClearPasswordBuffer(void)
{
    uint8_t i;
    for(i = 0; i < PASSWORD_LENGTH + 1; i++)
    {
        password[i] = 0;
        temp_password[i] = 0;
    }
    password_index = 0;
}
uint8_t VerifyPassword(const char* input, const char* stored);

/*
 * VerifyPassword
 * Compares input password with stored password.
 */
uint8_t VerifyPassword(const char* input, const char* stored)
{
    uint8_t i;
    for(i = 0; i < PASSWORD_LENGTH; i++)
    {
        if(input[i] != stored[i])
        {
            return 0;  /* Password mismatch */
        }
    }
    return 1;  /* Password match */
}

void handleDoor_HMI(void){
                                UART0_SendChar('F'); // HandleDoorOperation();
                                
                                    uint8_t countdown = auto_lock_timeout;
                                    char buffer[16];
                                     char handle_lcd_time_tmp_char;
                            
                                while(1){
                                   if (UART0_IsDataAvailable()){
                                      handle_lcd_time_tmp_char =  UART0_ReceiveChar();
                                      break;
                                      } 
                                     }
                                if(handle_lcd_time_tmp_char == 'u'){
                                    LCD_Clear();
                                    LCD_SetCursor(0, 0);
                                    LCD_WriteString("Door Unlocked!");
                                }
                                
                            /* Countdown display */
                              while(countdown > 0)
                               {
                                          sprintf(buffer, "Lock in %d sec", countdown);
                                          LCD_SetCursor(1, 0);
                                          LCD_WriteString("                ");
                                          LCD_SetCursor(1, 0);
                                          LCD_WriteString(buffer);
                                          DelayMs(1000);
                                          countdown--;
                                 }
                                
                                 while(1){
                                   if (UART0_IsDataAvailable()){
                                      handle_lcd_time_tmp_char =  UART0_ReceiveChar();
                                      break;
                                      } 
                                     }
                                if(handle_lcd_time_tmp_char == 'l'){
                                    LCD_Clear();
                                    LCD_SetCursor(0, 0);
                                    LCD_WriteString("Door locked!");
                                }
                                DelayMs(1500);


}


/*
 * ReadPotentiometerTimeout
 * Reads the potentiometer and maps to timeout value (5-30 seconds).
 */
uint8_t ReadPotentiometerTimeout(void)
{
    uint16_t adc_value = POT_ReadRaw();
    uint8_t timeout;
    
    /* Map ADC value (0-4095) to timeout (5-30) */
    timeout = MIN_TIMEOUT + ((adc_value * (MAX_TIMEOUT - MIN_TIMEOUT)) / 4095);
    
    return timeout;
}

/*
 * DisplayTimeoutValue
 * Displays the timeout value on LCD.
 */
void DisplayTimeoutValue(uint8_t timeout_val)
{
    char buffer[16];
    sprintf(buffer, "Timeout: %d sec", timeout_val);
    LCD_SetCursor(1, 0);
    LCD_WriteString("                ");  /* Clear line */
    LCD_SetCursor(1, 0);
    LCD_WriteString(buffer);
}
void ProcessKey(char key)
{
  static uint8_t numberOfAttempts = 0;
  static uint8_t numberOfAttemptsch = 0;

    switch(current_state)
    {
        case STATE_SETUP_PASSWORD:
          UART0_SendChar('A');
          //StatusLED_On();
            /* Initial password setup */
            if(key >= '0' && key <= '9')
            {
                if(password_index < PASSWORD_LENGTH)
                {
                    password[password_index] = key;
                    password_index++;
                    LCD_WriteChar('*');
                    //UART0_SendChar(key);
                    
                    if(password_index == PASSWORD_LENGTH)
                    {
                        password[PASSWORD_LENGTH] = '\0';
                        //UART0_SendChar('\n');
                        
                        DelayMs(300);
                        
                        /* Move to confirmation */
                        current_state = STATE_CONFIRM_PASSWORD;
                        password_index = 0;
                        LCD_Clear();
                        LCD_SetCursor(0, 0);
                        LCD_WriteString("Confirm Password");
                        LCD_SetCursor(1, 0);
                    }
                }
            }
            else if(key == '#')  /* Clear */
            {
                ClearPasswordBuffer();
                LCD_Clear();
                LCD_SetCursor(0, 0);
                LCD_WriteString("Enter Password:");
                LCD_SetCursor(1, 0);
                //UART0_SendChar('#');
            }
            break;
            
        case STATE_CONFIRM_PASSWORD:
            /* Confirm password */
            if(key >= '0' && key <= '9')
            {
                if(password_index < PASSWORD_LENGTH)
                {
                    temp_password[password_index] = key;
                    password_index++;
                    LCD_WriteChar('*');
                    UART0_SendChar(key);
                    
                    if(password_index == PASSWORD_LENGTH)
                    {
                        temp_password[PASSWORD_LENGTH] = '\0';
                        //DelayMs(300);
                        
                        
                        /* Verify passwords match */
                        if(VerifyPassword(password, temp_password))
                        {
                          //UART0_SendChar('\n'); // \n
                          
                          //char received = UART0_ReceiveChar();
                          UART0_SendChar('H');
                          UART0_SendString(password);
                          DelayMs(20);
                          char received;
                           while(1)
                             {
                          if(UART0_IsDataAvailable()){
                              received = UART0_ReceiveChar();
                                             break;
                              
                                }
                            }
                           
                          
                            // Save to EEPROM 
                            if(received == '1')
                            {
                                LCD_Clear();
                                LCD_SetCursor(0, 0);
                                LCD_WriteString("Password Saved!");
                                StatusLED_Blink(2);
                                DelayMs(1500);
                                
                                // Go to main menu
                                current_state = STATE_MAIN_MENU;
                                DisplayMainMenu();
                            }
                            
                            else if (received == '0')
                            {
                                LCD_Clear();
                                LCD_SetCursor(0, 0);
                                LCD_WriteString("Save Failed!");
                                DelayMs(1500);
                                ClearPasswordBuffer();
                                current_state = STATE_SETUP_PASSWORD;
                                LCD_Clear();
                                LCD_SetCursor(0, 0);
                                LCD_WriteString("Enter Password:");
                                LCD_SetCursor(1, 0);
                            }
                                
                          
                        }
                        
                        else 
                        {
                            /* Passwords don't match - restart */
                            LCD_Clear();
                            LCD_SetCursor(0, 0);
                            LCD_WriteString("Not Match!");
                            LCD_SetCursor(1, 0);
                            LCD_WriteString("Try Again");
                            DelayMs(1500);
                            
                            ClearPasswordBuffer();
                            UART0_SendChar('#'); // to clear password buffer from the Controller 
                            current_state = STATE_SETUP_PASSWORD;
                            LCD_Clear();
                            LCD_SetCursor(0, 0);
                            LCD_WriteString("Enter Password:");
                            LCD_SetCursor(1, 0);
                        }
                    }
                }
            }
            else if(key == '#')  /* Cancel */
            {
                UART0_SendChar('#');
                DelayMs(300);
                ClearPasswordBuffer();
                current_state = STATE_SETUP_PASSWORD;
                LCD_Clear();
                LCD_SetCursor(0, 0);
                LCD_WriteString("Enter Password:");
                LCD_SetCursor(1, 0);
            }
            break;
            
            
      case STATE_MAIN_MENU:
            if(key == 'A')  /* Open Door */
            {
                ClearPasswordBuffer();
                current_state = STATE_OPEN_DOOR_PASSWORD;
                LCD_Clear();
                LCD_SetCursor(0, 0);
                LCD_WriteString("Enter Password:");
                LCD_SetCursor(1, 0);
            }
            else if(key == 'B')  /* Change Password */
            {
                ClearPasswordBuffer();
                current_state = STATE_CHANGE_OLD_PASSWORD;
                LCD_Clear();
                LCD_SetCursor(0, 0);
                LCD_WriteString("Old Password:");
                LCD_SetCursor(1, 0);
            }
            else if(key == 'C')  /* Set Timeout */
            {
                pending_timeout = auto_lock_timeout;
                current_state = STATE_ADJUST_TIMEOUT;
                LCD_Clear();
                LCD_SetCursor(0, 0);
                LCD_WriteString("Adjust Timeout");
                DisplayTimeoutValue(pending_timeout);
            }
            else if(key == 'D')  /* Factory Reset */
            {
                LCD_Clear();
                LCD_SetCursor(0, 0);
                LCD_WriteString("Erase EEPROM?");
                LCD_SetCursor(1, 0);
                LCD_WriteString("Press # Confirm");
                DelayMs(3000);
                
                char confirm = Keypad_GetKey();
                while(confirm == 0) 
                {
                    confirm = Keypad_GetKey();
                    DelayMs(50);
                }
                
                if(confirm == '#')
                {
                    LCD_Clear();
                    LCD_SetCursor(0, 0);
                    LCD_WriteString("Erasing...");
                    
                    UART0_SendChar('J');
                    char EEPROM_MassErase_receive;
                    while(1){
                      if(UART0_IsDataAvailable()){
                         EEPROM_MassErase_receive = UART0_ReceiveChar();
                        break;
                      }
                    }
                    if(EEPROM_MassErase_receive == '1')
                    {
                        LCD_Clear();
                        LCD_SetCursor(0, 0);
                        LCD_WriteString("EEPROM Erased!");
                        LCD_SetCursor(1, 0);
                        LCD_WriteString("Restarting...");
                        DelayMs(2000);
                        
                        /* Software reset by jumping to reset vector */
                        NVIC_APINT_R = 0x05FA0004;
                    }
                    else
                    {
                        LCD_Clear();
                        LCD_SetCursor(0, 0);
                        LCD_WriteString("Erase Failed!");
                        DelayMs(2000);
                        DisplayMainMenu();
                    }
                }
                else
                {
                    LCD_Clear();
                    LCD_SetCursor(0, 0);
                    LCD_WriteString("Cancelled");
                    DelayMs(1000);
                    DisplayMainMenu();
                }
            }
            break;
            
            
            
            
     case STATE_OPEN_DOOR_PASSWORD:
            /* Enter password to open door */
            if(key >= '0' && key <= '9')
            {
                if(password_index < PASSWORD_LENGTH)
                {
                    password[password_index] = key;
                    password_index++;
                    LCD_WriteChar('*');
                    
                    if(password_index == PASSWORD_LENGTH)
                    {
                        password[PASSWORD_LENGTH] = '\0';
                        //DelayMs(300);
                        
UART0_SendChar('E');

/* wait ready */
while(!UART0_IsDataAvailable());
if(UART0_ReceiveChar() == '1')
{
    UART0_SendString(password);

    while(!UART0_IsDataAvailable());
    char result = UART0_ReceiveChar();

    if(result == '2')
    {
        handleDoor_HMI();
        ClearPasswordBuffer();
        current_state = STATE_MAIN_MENU;
        numberOfAttempts = 0;
        DisplayMainMenu();
    }
    else
    {
                                /* Incorrect password */
/* Incorrect password */
LCD_Clear();
LCD_SetCursor(0, 0);
LCD_WriteString("Wrong Password!");

numberOfAttempts ++;
StatusLED_Blink(3);
DelayMs(1500);

ClearPasswordBuffer();

if (numberOfAttempts  >= 3)
{
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_WriteString("Attempts Exceeded");
    LCD_SetCursor(1, 0);
    LCD_WriteString("System Locked");

    DelayMs(3000);

    numberOfAttempts  = 0;  
    UART0_SendChar('G');    //Buzzer_Beep(3000);
    current_state = STATE_MAIN_MENU; 
    DisplayMainMenu();
}
else
{
    current_state = STATE_MAIN_MENU;
    DisplayMainMenu();
}
                            }
                        }
                    }
                }
            }
            else if(key == '#')  /* Cancel */
            {
                ClearPasswordBuffer();
                current_state = STATE_MAIN_MENU;
                DisplayMainMenu();
            }
            break;
            
            
            
     case STATE_CHANGE_OLD_PASSWORD:
    if(key >= '0' && key <= '9')
    {
        if(password_index < PASSWORD_LENGTH)
        {
            password[password_index++] = key;
            LCD_WriteChar('*');

            if(password_index == PASSWORD_LENGTH)
            {
                password[PASSWORD_LENGTH] = '\0';
                DelayMs(300);

                  UART0_SendChar('E');
                    char tmp_stored_password_Received2;
                     while(1){
                      if (UART0_IsDataAvailable()){
                        tmp_stored_password_Received2 =  UART0_ReceiveChar();
                         break;
                         } 
                       } 
                
                        if(tmp_stored_password_Received2 == '1')
                        {
                          UART0_SendString(password);
                          DelayMs(200);
                        while(1){
                          if (UART0_IsDataAvailable()){
                                tmp_stored_password_Received2 =  UART0_ReceiveChar();
                                 break;
                                } 
                      } 
                                   
                   if(tmp_stored_password_Received2 == '2')
                     {
                    /* Password correct - move to new password */
                    ClearPasswordBuffer();
                    current_state = STATE_CHANGE_NEW_PASSWORD;
                    LCD_Clear();
                    LCD_SetCursor(0, 0);
                    LCD_WriteString("New Password:");
                    LCD_SetCursor(1, 0);
                }
                else
                {
                    /* Wrong password */
                    LCD_Clear();
                    LCD_SetCursor(0, 0);
                    LCD_WriteString("Wrong Password!");
                    StatusLED_Blink(3);
                    DelayMs(1500);
                    ClearPasswordBuffer();
                    current_state = STATE_CHANGE_OLD_PASSWORD;
                    DisplayMainMenu();
                }
            }
        }
    }
    else if(key == '#')  /* Cancel */
    {
        ClearPasswordBuffer();
        current_state = STATE_MAIN_MENU;
        DisplayMainMenu();
    }
    break;
    
    
        case STATE_CHANGE_NEW_PASSWORD:
           
           
            /* Enter new password */
            if(key >= '0' && key <= '9')
            {
                if(password_index < PASSWORD_LENGTH)
                {
                    password[password_index] = key;
                    password_index++;
                    LCD_WriteChar('*');
                    
                    if(password_index == PASSWORD_LENGTH)
                    {
                        password[PASSWORD_LENGTH] = '\0';
                        DelayMs(300);
                        
                        /* Move to confirmation */
                        current_state = STATE_CHANGE_CONFIRM_PASSWORD;
                        password_index = 0;
                        LCD_Clear();
                        LCD_SetCursor(0, 0);
                        LCD_WriteString("Confirm Password");
                        LCD_SetCursor(1, 0);
                    }
                }
            }
            else if(key == '#')  /* Cancel */
            {
                ClearPasswordBuffer();
                current_state = STATE_MAIN_MENU;
                DisplayMainMenu();
            }
            break;
            
        case STATE_CHANGE_CONFIRM_PASSWORD:
          
         
          
            /* Confirm new password */
            if(key >= '0' && key <= '9')
            {
                if(password_index < PASSWORD_LENGTH)
                {
                    temp_password[password_index] = key;
                    password_index++;
                    LCD_WriteChar('*');
                    
                    if(password_index == PASSWORD_LENGTH)
                    {
                        temp_password[PASSWORD_LENGTH] = '\0';
                        DelayMs(300);
                        
                        /* Verify passwords match */
                        if(VerifyPassword(password, temp_password))
                        {
                            /* Save to EEPROM */
                          char store_new_pass_received ;
                            UART0_SendChar('H');
                            
                           // while(1){
                             // if(UART0_IsDataAvailable()){
                               UART0_SendString(password);
                               DelayMs(200);
                              // break;
                              //}
                            //}
                            
                            while(1){
                              if(UART0_IsDataAvailable()){
                               store_new_pass_received = UART0_ReceiveChar();
                               break;
                              }
                            }
                            if(store_new_pass_received == '1')
                            {
                                LCD_Clear();
                                LCD_SetCursor(0, 0);
                                LCD_WriteString("Password Changed");
                                StatusLED_Blink(2);
                                DelayMs(1500);
                                
                                ClearPasswordBuffer();
                                current_state = STATE_MAIN_MENU;
                                DisplayMainMenu();
                            }
                            else
                            {
                                LCD_Clear();
                                LCD_SetCursor(0, 0);
                                LCD_WriteString("Save Failed!");
                                DelayMs(1500);
                                current_state = STATE_MAIN_MENU;
                                DisplayMainMenu();
                            }
                        }
                        else
                        {
                            /* Passwords don't match */
                            LCD_Clear();
                            LCD_SetCursor(0, 0);
                            LCD_WriteString("Not Match!");
                            DelayMs(1500);
                            
                            ClearPasswordBuffer();
                            current_state = STATE_CHANGE_NEW_PASSWORD;
                            LCD_Clear();
                            LCD_SetCursor(0, 0);
                            LCD_WriteString("New Password:");
                            LCD_SetCursor(1, 0);
                        }
                    }
                }
            }
            else if(key == '#')  /* Cancel */
            {
                ClearPasswordBuffer();
                current_state = STATE_MAIN_MENU;
                DisplayMainMenu();
            }
            break;
            
        case STATE_ADJUST_TIMEOUT:
            /* Adjust timeout using potentiometer */
            if(key == 'D')  /* Save timeout */
            {
                current_state = STATE_TIMEOUT_PASSWORD;
                ClearPasswordBuffer();
                LCD_Clear();
                LCD_SetCursor(0, 0);
                LCD_WriteString("Enter Password:");
                LCD_SetCursor(1, 0);
            }
            else if(key == '#')  /* Cancel */
            {
                current_state = STATE_MAIN_MENU;
                DisplayMainMenu();
            }
            break;            
    
        case STATE_TIMEOUT_PASSWORD:
            /* Enter password to save timeout */
            if(key >= '0' && key <= '9')
            {
                if(password_index < PASSWORD_LENGTH)
                {
                    password[password_index] = key;
                    password_index++;
                    LCD_WriteChar('*');
                    
            if(password_index == PASSWORD_LENGTH)
            {
                password[PASSWORD_LENGTH] = '\0';

UART0_SendChar('E');

while(!UART0_IsDataAvailable());
if(UART0_ReceiveChar() == '1')
{
    for(uint8_t i = 0; i < PASSWORD_LENGTH; i++)
    {
        UART0_SendChar(password[i]);
    }



    while(!UART0_IsDataAvailable());
    char result = UART0_ReceiveChar();

    if(result == '2')
    {
                                /* Correct password - save timeout */
                                auto_lock_timeout = pending_timeout;
UART0_SendChar('I');         
UART0_SendUInt(auto_lock_timeout);  

while(!UART0_IsDataAvailable());   
char StoreTimeoutReceived = UART0_ReceiveChar();
                                
                                if(StoreTimeoutReceived == '1')
                                {
                                    LCD_Clear();
                                    LCD_SetCursor(0, 0);
                                    LCD_WriteString("Timeout Saved!");
                                    StatusLED_Blink(2);
                                    DelayMs(1500);
                                }
                                
                                ClearPasswordBuffer();
                                current_state = STATE_MAIN_MENU;
                                DisplayMainMenu();
                            }
                            else
                            {
                                /* Incorrect password */
                                LCD_Clear();
                                LCD_SetCursor(0, 0);
                                LCD_WriteString("Wrong Password!");
                                StatusLED_Blink(3);
                                DelayMs(1500);
                                
                                ClearPasswordBuffer();
                                current_state = STATE_MAIN_MENU;
                                DisplayMainMenu();
                            }
                        }
                    }
                }
            }
            else if(key == '#')  /* Cancel */
            {
                ClearPasswordBuffer();
                current_state = STATE_MAIN_MENU;
                DisplayMainMenu();
            }
            break;
            
        default:
            break;            
            
            
    }
    }
}



/******************************************************************************
 *                          Main Function                                      *
 ******************************************************************************/

int main(void)
{
    char key;
    uint8_t password_exists = 0;
    
    /* Initialize system */
    System_Init();
    UART0_Init();
    /* Display initialization message */
    LCD_SetCursor(0, 0);
    LCD_WriteString("Smart Door Lock");
    LCD_SetCursor(1, 0);
    LCD_WriteString("Initializing...");
     StatusLED_On();

      /* Initialize EEPROM */
     
     UART0_SendChar('B');
     char EEPROM_SUCCESS_Received;
       while(1){
     if (UART0_IsDataAvailable()){
       EEPROM_SUCCESS_Received =  UART0_ReceiveChar();
       break;
     } 
       }
     //printf(EEPROM_SUCCESS_Received);
     if(EEPROM_SUCCESS_Received == '0'){
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_WriteString("EEPROM Error!");
        while(1)
        {
            StatusLED_Blink(1);
        }
     }
     
    
    DelayMs(1000);
    
     UART0_SendChar('C');
     char RetrievePassword_Received;
       while(1){
     if (UART0_IsDataAvailable()){
       RetrievePassword_Received =  UART0_ReceiveChar();
       break;
     } 
       }
      
        /* Check if EEPROM is in erased state (all 0xFF) or uninitialized (all 0x00) */
        if(RetrievePassword_Received == '1')
        {
            LCD_Clear();
            LCD_SetCursor(0, 0);
            LCD_WriteString("EEPROM Empty");
            DelayMs(1000);
        }
        /* Check if valid password exists (ASCII digits '0'-'9') */
        else if(RetrievePassword_Received == '2')
        {
            password_exists = 1;
        }
    
    
       
       
      UART0_SendChar('D');
     char RetrieveTimeout_Received;
       while(1){
     if (UART0_IsDataAvailable()){
       RetrieveTimeout_Received =  UART0_ReceiveChar();
       break;
     } 
       }
       
    /* Retrieve timeout (use default if not found) */
    if(RetrieveTimeout_Received == '1')
    {
        auto_lock_timeout = 10;  /* Default */
    }
    
    StatusLED_Off();
    
    /* Check if password setup is needed */
    if(!password_exists)
    {
        /* First time setup */
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_WriteString("First Time Setup");
        DelayMs(1500);
        
        current_state = STATE_SETUP_PASSWORD;
        ClearPasswordBuffer();
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_WriteString("Enter Password:");
        LCD_SetCursor(1, 0);
    }
    else
    {
        /* Password exists - go to main menu */
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_WriteString("Welcome!");
        DelayMs(1000);
        
        current_state = STATE_MAIN_MENU;
        DisplayMainMenu();
    }
    
    
    
    /* Main loop */
    while(1)
    {
        key = Keypad_GetKey();
        //StatusLED_Off();
        if(key != 0)  /* Key pressed */
        {
            /* Brief status LED flash */
            StatusLED_On();
            DelayMs(50);
            StatusLED_Off();
            /* Process the key */
            ProcessKey(key);
        }
                /* Update potentiometer value in adjust timeout state */
        if(current_state == STATE_ADJUST_TIMEOUT)
        {
            uint8_t new_timeout = ReadPotentiometerTimeout();
            if(new_timeout != pending_timeout)
            {
                pending_timeout = new_timeout;
                DisplayTimeoutValue(pending_timeout);
            }
            DelayMs(200);
        }
        else
        {
            DelayMs(100);  /* Debounce delay */
        }
    }
}
