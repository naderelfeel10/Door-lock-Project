
#define DOOR_LED_RED            PIN1    
#define DOOR_LED_GREEN          PIN3    
#define STATUS_LED_BLUE         PIN2    


#include "uart.h"
#include "dio.h"

void uart_send_test(void ){
     UART0_SendChar('A');

} 
void uart_receive_test(void ){
     printf("Testing uart communication to turn on a led on the tiva \n");
     while(!UART0_IsDataAvailable());
     
     char received = UART0_ReceiveChar();
     if (received == 'A'){
       printf("UART send/receive test PASS\n");
       DIO_Init(PORTF, STATUS_LED_BLUE, OUTPUT); 
       DIO_WritePin(PORTF, STATUS_LED_BLUE, HIGH);
     }
     else {printf("UART send/receive test FAIL\n"); }
  } 
