/*****************************************************************************
 * File: main.c
 * Description: Smart Door Lock System with EEPROM (Register Level)
 * Author: Ahmedhh
 * Date: December 14, 2025
 * 
 * Application Features:
 *   1. Initial Password Setup (5-digit password with confirmation)
 *   2. Main Menu:
 *      A → Open Door
 *      B → Change Password
 *      C → Set Auto-Lock Timeout
 *   3. Door Control (LED mimics door lock)
 *   4. Password verification (3 attempts max)
 *   5. Lockout on 3 failed attempts
 *   6. Adjustable timeout using potentiometer (5-30 seconds)
 * 
 * Hardware Connections:
 *   - LCD: Port B (PB0-PB5)
 *   - Keypad: Port C (PC4-PC7) and Port E (PE0-PE3)
 *   - Door LED: PF1 (Red - Locked), PF3 (Green - Unlocked)
 *   - Status LED: PF2 (Blue - for feedback)
 *   - Potentiometer: PE3 (AIN0)
 *****************************************************************************/

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "tm4c123gh6pm.h"
#include "dio.h"
#include "lcd.h"
#include "keypad.h"
#include "systick.h"
#include "eeprom.h"
#include "adc.h"
#include "potentiometer.h"
#include "Buzzer.h"
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
static char stored_password[PASSWORD_LENGTH + 1] = {0};    /* Password from EEPROM */
static uint8_t password_index = 0;
static AppState current_state = STATE_INIT;
static uint8_t attempt_count = 0;
static uint8_t auto_lock_timeout = 10;  /* Default 10 seconds */
static uint8_t pending_timeout = 10;     /* Temp value when adjusting */

/******************************************************************************
 *                          Function Prototypes                                *
 ******************************************************************************/

void System_Init(void);
void Door_Init(void);
void Door_Lock(void);
void Door_Unlock(void);
void StatusLED_On(void);
void StatusLED_Off(void);
void StatusLED_Blink(uint8_t times);

void DisplayMainMenu(void);
void ClearPasswordBuffer(void);
uint8_t StorePassword(const char* pwd);
uint8_t RetrievePassword(char* pwd);
uint8_t StoreTimeout(uint8_t timeout);
uint8_t RetrieveTimeout(uint8_t* timeout);
uint8_t VerifyPassword(const char* input, const char* stored);
void ProcessKey(char key);
void HandleDoorOperation(void);
void HandleLockout(void);
uint8_t ReadPotentiometerTimeout(void);
void DisplayTimeoutValue(uint8_t timeout_val);

/******************************************************************************
 *                          Function Implementations                           *
 ******************************************************************************/

/*
 * System_Init
 * Initializes all system components.
 */

void System_Init(void)
{
    /* Initialize SysTick for delays */
    SysTick_Init(16000, SYSTICK_NOINT);
    
    /* Initialize LCD */
    LCD_Init();
    LCD_Clear();
    
    /* Initialize Keypad */
    Keypad_Init();
    Motor_Init();
    /* Initialize Door (LEDs) */
    Door_Init();
    
    /* Initialize Potentiometer */
    POT_Init();
    
    Buzzer_Init();
}

/*
 * Door_Init
 * Initializes the door LED indicators.
 */
void Door_Init(void)
{
    DIO_Init(PORTF, DOOR_LED_RED, OUTPUT);      /* Red LED - Locked */
    DIO_Init(PORTF, DOOR_LED_GREEN, OUTPUT);    /* Green LED - Unlocked */
    DIO_Init(PORTF, STATUS_LED_BLUE, OUTPUT);   /* Blue LED - Status */
    
    Door_Lock();  /* Start locked */
    StatusLED_Off();
}

/*
 * Door_Lock
 * Locks the door (Red LED on, Green LED off).
 */
void Door_Lock(void)
{
    DIO_WritePin(PORTF, DOOR_LED_RED, HIGH);
    DIO_WritePin(PORTF, DOOR_LED_GREEN, LOW);
    Motor_RotateCCW();
}

/*
 * Door_Unlock
 * Unlocks the door (Green LED on, Red LED off).
 */
void Door_Unlock(void)
{
    DIO_WritePin(PORTF, DOOR_LED_RED, LOW);
    DIO_WritePin(PORTF, DOOR_LED_GREEN, HIGH);
    Motor_RotateCW();
}

/*
 * StatusLED_On
 * Turns on the status LED (Blue).
 */
void StatusLED_On(void)
{
    DIO_WritePin(PORTF, STATUS_LED_BLUE, HIGH);
}

/*
 * StatusLED_Off
 * Turns off the status LED (Blue).
 */
void StatusLED_Off(void)
{
    DIO_WritePin(PORTF, STATUS_LED_BLUE, LOW);
}

/*
 * StatusLED_Blink
 * Blinks the status LED.
 */
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

/*
 * StorePassword
 * Stores the password in EEPROM.
 */
uint8_t StorePassword(const char* pwd)
{
    uint8_t buffer[8] = {0};  /* Initialize with zeros */
    uint8_t i;
    
    /* Convert password string to bytes */
    for(i = 0; i < PASSWORD_LENGTH; i++)
    {
        buffer[i] = (uint8_t)pwd[i];
    }
    
    /* Write to EEPROM - must be multiple of 4 bytes, so write 8 bytes */
    return EEPROM_WriteBuffer(PASSWORD_EEPROM_BLOCK, PASSWORD_EEPROM_OFFSET, buffer, 8);
}

/*
 * RetrievePassword
 * Retrieves the stored password from EEPROM.
 */
uint8_t RetrievePassword(char* pwd)
{
    uint8_t buffer[8] = {0};  /* Initialize with zeros */
    uint8_t i;
    uint8_t result;
    
    /* Read from EEPROM - read 8 bytes */
    result = EEPROM_ReadBuffer(PASSWORD_EEPROM_BLOCK, PASSWORD_EEPROM_OFFSET, buffer, 8);
    
    if(result == EEPROM_SUCCESS)
    {
        
        /* Convert bytes to password string */
        for(i = 0; i < PASSWORD_LENGTH; i++)
        {
            pwd[i] = (char)buffer[i];
        }
        pwd[PASSWORD_LENGTH] = '\0';
    }
    
    return result;
}

/*
 * StoreTimeout
 * Stores the timeout value in EEPROM.
 */
uint8_t StoreTimeout(uint8_t timeout)
{
    uint8_t buffer[4] = {0};  /* Initialize with zeros */
    buffer[0] = timeout;
    
    /* Write to EEPROM - write 4 bytes (multiple of 4) */
    return EEPROM_WriteBuffer(PASSWORD_EEPROM_BLOCK, TIMEOUT_EEPROM_OFFSET, buffer, 4);
}
/*
 * RetrieveTimeout
 * Retrieves the timeout value from EEPROM.
 */
uint8_t RetrieveTimeout(uint8_t* timeout)
{
    uint8_t buffer[4] = {0};  /* Initialize with zeros */
    uint8_t result;
    
    /* Read from EEPROM - read 4 bytes */
    result = EEPROM_ReadBuffer(PASSWORD_EEPROM_BLOCK, TIMEOUT_EEPROM_OFFSET, buffer, 4);
    
    if(result == EEPROM_SUCCESS)
    {
        *timeout = buffer[0];
        /* Validate timeout range */
        if(*timeout < MIN_TIMEOUT || *timeout > MAX_TIMEOUT)
        {
            *timeout = 10;  /* Default to 10 seconds */
        }
    }
    
    return result;
}

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

/*
 * HandleDoorOperation
 * Handles the door unlock/lock sequence.
 */
void HandleDoorOperation(void)
{
    uint8_t countdown = auto_lock_timeout;
    char buffer[16];
    
    /* Unlock the door */
    Door_Unlock();
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_WriteString("Door Unlocked!");
    
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
    
    /* Lock the door */
    Door_Lock();
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_WriteString("Door Locked!");
    DelayMs(1500);
}

/*
 * ProcessKey
 * Processes keypad input based on current state.
 */
void ProcessKey(char key)
{
  static uint8_t numberOfAttempts = 0;
  static uint8_t numberOfAttemptsch = 0;

    switch(current_state)
    {
        case STATE_SETUP_PASSWORD:
            /* Initial password setup */
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
                    
                    if(password_index == PASSWORD_LENGTH)
                    {
                        temp_password[PASSWORD_LENGTH] = '\0';
                        DelayMs(300);
                        
                        /* Verify passwords match */
                        if(VerifyPassword(password, temp_password))
                        {
                            /* Save to EEPROM */
                            if(StorePassword(password) == EEPROM_SUCCESS)
                            {
                                LCD_Clear();
                                LCD_SetCursor(0, 0);
                                LCD_WriteString("Password Saved!");
                                StatusLED_Blink(2);
                                DelayMs(1500);
                                
                                /* Go to main menu */
                                current_state = STATE_MAIN_MENU;
                                DisplayMainMenu();
                            }
                            else
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
                    
                    if(EEPROM_MassErase() == EEPROM_SUCCESS)
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
                        DelayMs(300);
                        
                        /* Retrieve stored password */
                        if(RetrievePassword(stored_password) == EEPROM_SUCCESS)
                        {
                            if(VerifyPassword(password, stored_password))
                            {
                                /* Correct password */
                                HandleDoorOperation();
                                ClearPasswordBuffer();
                                current_state = STATE_MAIN_MENU;
                                numberOfAttempts  = 0;
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
    Buzzer_Beep(2000);
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

                if(RetrievePassword(stored_password) == EEPROM_SUCCESS &&
                   VerifyPassword(password, stored_password))
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
                            if(StorePassword(password) == EEPROM_SUCCESS)
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
                        DelayMs(300);
                        
                        /* Retrieve stored password */
                        if(RetrievePassword(stored_password) == EEPROM_SUCCESS)
                        {
                            if(VerifyPassword(password, stored_password))
                            {
                                /* Correct password - save timeout */
                                auto_lock_timeout = pending_timeout;
                                if(StoreTimeout(auto_lock_timeout) == EEPROM_SUCCESS)
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

/******************************************************************************
 *                          Main Function                                      *
 ******************************************************************************/

int main(void)
{
    char key;
    uint8_t password_exists = 0;
    
    /* Initialize system */
    System_Init();
    
    /* Display initialization message */
    LCD_SetCursor(0, 0);
    LCD_WriteString("Smart Door Lock");
    LCD_SetCursor(1, 0);
    LCD_WriteString("Initializing...");
    StatusLED_On();
    
    /* Initialize EEPROM */
    if(EEPROM_Init() != EEPROM_SUCCESS)
    {
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_WriteString("EEPROM Error!");
        while(1)
        {
            StatusLED_Blink(1);
        }
    }
    
    DelayMs(1000);
    

    /* Try to retrieve existing password */
    if(RetrievePassword(stored_password) == EEPROM_SUCCESS)
    {
        /* Check if EEPROM is in erased state (all 0xFF) or uninitialized (all 0x00) */
        if(stored_password[0] == 0xFF || stored_password[0] == 0x00)
        {
            LCD_Clear();
            LCD_SetCursor(0, 0);
            LCD_WriteString("EEPROM Empty");
            DelayMs(1000);
        }
        /* Check if valid password exists (ASCII digits '0'-'9') */
        else if(stored_password[0] >= '0' && stored_password[0] <= '9')
        {
            password_exists = 1;
        }
    }
    
    /* Retrieve timeout (use default if not found) */
    if(RetrieveTimeout(&auto_lock_timeout) != EEPROM_SUCCESS)
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
