/* Control_ECU main file */


#include <stdint.h>
#include <stdbool.h>

#include <string.h>
#include <stdio.h>
#include "tm4c123gh6pm.h"
#include "dio.h"
#include "systick.h"
#include "eeprom.h"
#include "adc.h"
#include "Buzzer.h"

void ClearPasswordBuffer(void);
void System_Init(void);
uint8_t VerifyPassword(const char* input, const char* stored);
void Door_Init(void);
void Door_Lock(void);
void Door_Unlock(void);

/**************************
 *                              Definitions                                    *
 **************************/

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
#define TIMER0_1MS_RELOAD 16000
#include "GPTM_TIMER0.h"

static char buffer[PASSWORD_LENGTH + 1];
static uint8_t password_index = 0;
static char stored_password[PASSWORD_LENGTH + 1] = {0};    /* Password from EEPROM */
static uint8_t attempt_count = 0;
static uint8_t auto_lock_timeout = 10;  /* Default 10 seconds */
static uint8_t pending_timeout = 10;     /* Temp value when adjusting */


#define SYSTICK_1MS_RELOAD   16000U
#define DOOR_MOVE_DELAY_MS  3000U
#define TIMER0_1MS_RELOAD 16000U

void System_Init(void)
{
    /* Initialize SysTick for delays */
    SysTick_Init(SYSTICK_1MS_RELOAD, SYSTICK_NOINT);
    
    /* Initialize LCD */
    //LCD_Init();
    //LCD_Clear();
    
    /* Initialize Keypad */
    //Keypad_Init();
    Motor_Init();
    /* Initialize Door (LEDs) */
    Door_Init();
    
    /* Initialize Potentiometer */
    //POT_Init();
    
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
    
    Motor_Stop();  /* Start locked */
    //StatusLED_Off();
}

void ClearPasswordBuffer(void)
{
    uint8_t i;
    for(i = 0; i < PASSWORD_LENGTH + 1; i++)
    {
        buffer[i] = 0;
    }
    password_index = 0;
}


/*
 * StorePassword
 * Stores the password in EEPROM.
 */
uint8_t StorePassword(const char* pwd)
{
    uint8_t tmp_buffer[8] = {0};  /* Initialize with zeros */
    uint8_t i;
    
    /* Convert password string to bytes */
    for(i = 0; i < PASSWORD_LENGTH; i++)
    {
        tmp_buffer[i] = (uint8_t)pwd[i];
    }
    
    /* Write to EEPROM - must be multiple of 4 bytes, so write 8 bytes */
    return EEPROM_WriteBuffer(PASSWORD_EEPROM_BLOCK, PASSWORD_EEPROM_OFFSET, tmp_buffer, 8);
}


/*
 * RetrievePassword
 * Retrieves the stored password from EEPROM.
 */
uint8_t RetrievePassword(char* pwd)
{
    uint8_t local_buffer[8] = {0};  /* Initialize with zeros */
    //uint8_t i;
    uint8_t result;
    
    /* Read from EEPROM - read 8 bytes */
    result = EEPROM_ReadBuffer(PASSWORD_EEPROM_BLOCK, PASSWORD_EEPROM_OFFSET, local_buffer, 8);
    
    if(result == EEPROM_SUCCESS)
    {
        
        /* Convert bytes to password string */
        for(uint8_t i = 0; i < PASSWORD_LENGTH; i++)
        {
            pwd[i] = (char)local_buffer[i];
        }
        pwd[PASSWORD_LENGTH] = '\0';
    }
    
    return result;
}



uint8_t StoreTimeout(uint8_t timeout)
{
    uint8_t timeout_Buffer [4] = {0};  /* Initialize with zeros */
    timeout_Buffer[0] = timeout;
    
    /* Write to EEPROM - write 4 bytes (multiple of 4) */
    return EEPROM_WriteBuffer(PASSWORD_EEPROM_BLOCK, TIMEOUT_EEPROM_OFFSET, timeout_Buffer, 4);
}
/*
 * RetrieveTimeout
 * Retrieves the timeout value from EEPROM.
 */
uint8_t RetrieveTimeout(uint8_t* timeout)
{
    uint8_t timeout_Buffer[4] = {0};  /* Initialize with zeros */
    uint8_t result;
    
    /* Read from EEPROM - read 4 bytes */
    result = EEPROM_ReadBuffer(PASSWORD_EEPROM_BLOCK, TIMEOUT_EEPROM_OFFSET, timeout_Buffer, 4);
    
    if(result == EEPROM_SUCCESS)
    {
        *timeout = timeout_Buffer[0];
        /* Validate timeout range */
        if(*timeout < MIN_TIMEOUT || *timeout > MAX_TIMEOUT)
        {
            *timeout = 10; 
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


//////////////////////////////////////////
void UART_SavePasswordCase()
{
       char received ;
     //UART0_SendChar(received);
     
  while(1){
     if(UART0_IsDataAvailable())
        {
      received = UART0_ReceiveChar();
     //UART0_SendChar(received);
    if(received >= '0' && received <= '9')
    {
        if(password_index < PASSWORD_LENGTH)
        {
            buffer[password_index++] = received;
        }
        break;
    }
    else if(received == '\0')
    {
      
        buffer[password_index] = '\0'; 
        DelayMs(300);
        //UART0_SendString("\r\n");
        if(StorePassword(buffer) == EEPROM_SUCCESS)
        {
            UART0_SendChar('1'); 
        }
        else
        {
            UART0_SendChar('0'); 
        }
        ClearPasswordBuffer();
        break;
    }
     else if(received == '#')
            {
                ClearPasswordBuffer();
                //UART0_SendChar('#'); 
                break;
            }
  }

  }
}








//////////////////////////////////////////////////////////////////////////////////////////////////

void UART_EEPROM_Init(void){
    /* Initialize EEPROM */
    if(EEPROM_Init() != EEPROM_SUCCESS)
    {
        UART0_SendChar('0');
    }
    else {
      UART0_SendChar('1');
    }
  }

void UART_RetrievePassword(void){

      if(RetrievePassword(stored_password) == EEPROM_SUCCESS)
    {
        /* Check if EEPROM is in erased state (all 0xFF) or uninitialized (all 0x00) */
        if(stored_password[0] == 0xFF || stored_password[0] == 0x00)
        {
            UART0_SendChar('1');
        }
        /* Check if valid password exists (ASCII digits '0'-'9') */
        else if(stored_password[0] >= '0' && stored_password[0] <= '9')
        {
            //password_exists = 1;
            UART0_SendChar('2');
        }
    }
    
}


void UART_RetrieveTimeout(void){
    /* Retrieve timeout (use default if not found) */
    if(RetrieveTimeout(&auto_lock_timeout) != EEPROM_SUCCESS)
    {
        auto_lock_timeout = 10;  /* Default */
        UART0_SendChar('1');
    }
    else{UART0_SendChar('0');}
 }

/*
void UART_verifyPassword(void)
{
    char rx_password[PASSWORD_LENGTH + 1];

    if(RetrievePassword(stored_password) == EEPROM_SUCCESS)
    {
        UART0_SendChar('1');   // ready
        UART0_ReceiveString(rx_password);

        if(VerifyPassword(rx_password, stored_password))
            UART0_SendChar('2');   // correct
        else
            UART0_SendChar('3');   // wrong
    }
}
*/
void UART_verifyPassword(void)
{
    //char rx_password[PASSWORD_LENGTH + 1]; --code refactoring

    if(RetrievePassword(stored_password) == EEPROM_SUCCESS)
    {
        char rx_password[PASSWORD_LENGTH + 1];
        UART0_SendChar('1');   // ready

        for(uint8_t i = 0; i < PASSWORD_LENGTH; i++)
        {
            rx_password[i] = UART0_ReceiveChar();
        }
        rx_password[PASSWORD_LENGTH] = '\0';

        if(VerifyPassword(rx_password, stored_password))
            UART0_SendChar('2');   // correct
        else
            UART0_SendChar('3');   // wrong
    }
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

    GPTM_Timer0A_Init(DOOR_MOVE_DELAY_MS * TIMER0_1MS_RELOAD);
    while (!GPTM_Timer0A_TimeOut());
    GPTM_Timer0A_ClearFlag();

    Motor_Stop();

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

    GPTM_Timer0A_Init(DOOR_MOVE_DELAY_MS * TIMER0_1MS_RELOAD);
    while (!GPTM_Timer0A_TimeOut());
    GPTM_Timer0A_ClearFlag();

    Motor_Stop();
}

void HandleDoorOperation(void)
{
    
    Door_Unlock();
    UART0_SendChar('u');

    GPTM_Timer0A_Init(auto_lock_timeout * 1000 * TIMER0_1MS_RELOAD);
    while (!GPTM_Timer0A_TimeOut());
    GPTM_Timer0A_ClearFlag();

    Door_Lock();
    UART0_SendChar('l');

}

int main(void)
{
    System_Init();
    UART0_Init();
    //EEPROM_Init();
    while(1)
    {
    if(UART0_IsDataAvailable())
    {
            char receivedChar = UART0_ReceiveChar();
        //UART0_SendChar(receivedChar);
        //UART0_SendString("\r\n");  /* New line for better readability */
         switch(receivedChar){
         
         // save the password then send the result 
         case 'A':
                UART_SavePasswordCase();
                //UART0_SendChar('S');
                break;
         case 'B':
           UART_EEPROM_Init();
                break;
           
         case 'C':
           UART_RetrievePassword();
                break;
           
         case 'D':
           UART_RetrieveTimeout();
                break;

         case 'E':
           UART_verifyPassword();
                break;

         case 'F':
               HandleDoorOperation();
                break;

         case 'G':
           Buzzer_Beep(3000);
                break;

         case 'H':
           char password_to_store[PASSWORD_LENGTH+1];
             while(1){
                  if (UART0_IsDataAvailable()){
              UART0_ReceiveString(password_to_store);
                     break;
                       } 
                      }
              if(StorePassword(password_to_store) == EEPROM_SUCCESS)
                {
                  UART0_SendChar('1');
                }else {    UART0_SendChar('0'); }  
           
                             break;

         case 'I':
               uint8_t new_timeout = UART0_ReceiveUInt();
               DelayMs(20);
              if(StoreTimeout(new_timeout) == EEPROM_SUCCESS)
                {
                  auto_lock_timeout = new_timeout;
                  UART0_SendChar('1');
                }else {  UART0_SendChar('0'); }
                break;

         case 'J':
              if(EEPROM_MassErase() == EEPROM_SUCCESS)
                    {
                      UART0_SendChar('1');
                    }
                    else { UART0_SendChar('0');}    
            break;
         default : break;
         }

    }
}
}