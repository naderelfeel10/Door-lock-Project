/******************************************************************************
 * File: uart5.c
 * Module: UART5 (Hardware) mapped to UART0 (Software Naming)
 * Description: Source file for TM4C123GH6PM UART5 Driver
 * * NOTE: Functions are named UART0_... as requested, but control UART5 (PE4/PE5)
 ******************************************************************************/

#include "uart.h" // Ensure this header has the UART0_... prototypes
#include "tm4c123gh6pm.h"

void UART0_Init(void)
{
    volatile uint32_t delay;
    
    /* 1. Enable clock for UART5 */
    SYSCTL_RCGCUART_R |= 0x20;      /* Enable UART5 clock (bit 5) */
    delay = SYSCTL_RCGCUART_R;      /* Wait for clock to stabilize */
    
    /* 2. Enable clock for GPIOE */
    SYSCTL_RCGCGPIO_R |= 0x10;      /* Enable PORTE clock (bit 4) */
    delay = SYSCTL_RCGCGPIO_R;      /* Wait for clock to stabilize */
    
    /* 3. Disable UART5 before configuration */
    UART5_CTL_R &= ~UART_CTL_UARTEN;
    
    /* 4. Configure baud rate: 115200 @ 16MHz */
    /* BRD = 16,000,000 / (16 * 115,200) = 8.680555 */
    UART5_IBRD_R = 8;               
    UART5_FBRD_R = 44;              
    
    /* 5. Configure Line Control (LCRH) */
    /* 8-bit, no parity, 1 stop, FIFOs enabled (0x10) */
    UART5_LCRH_R = (UART_LCRH_WLEN_8 | 0x10);  
    
    /* 6. Configure UART5 Control Register */
    /* Enable UART, TX, and RX */
    UART5_CTL_R = (UART_CTL_UARTEN | UART_CTL_TXE | UART_CTL_RXE);
    
    /* 7. Configure GPIO pins PE4 (RX) and PE5 (TX) */
    GPIO_PORTE_AFSEL_R |= 0x30;     /* Bits 4 and 5 */ 
    
    /* Configure PCTL for UART function (PMC = 1) */
    /* PE4 is bits 19:16, PE5 is bits 23:20 */
    GPIO_PORTE_PCTL_R = (GPIO_PORTE_PCTL_R & 0xFF00FFFF) | 0x00110000;
    
    GPIO_PORTE_DEN_R   |= 0x30;     /* Digital Enable PE4, PE5 */
    GPIO_PORTE_AMSEL_R &= ~0x30;    /* Disable Analog PE4, PE5 */
}

void UART0_SendChar(char data)
{
    /* Wait until transmit FIFO is not full */
    while ((UART5_FR_R & UART_FR_TXFF) != 0);
    UART5_DR_R = data;
}

char UART0_ReceiveChar(void)
{
    /* Wait until receive FIFO is not empty */
    while ((UART5_FR_R & UART_FR_RXFE) != 0);
    return (char)(UART5_DR_R & 0xFF);
}

void UART0_SendString(const char *str)
{
    while (*str)
    {
        UART0_SendChar(*str++);
    }
}

void UART0_ReceiveString(char *buffer)
{
    char c;
    uint32_t i = 0;
    while (1)
    {
        c = UART0_ReceiveChar();
        if (c == '\r' || c == '\n')
        {
            buffer[i] = '\0';
            break;
        }
        buffer[i++] = c;
    }
}

void UART0_SendUInt(uint32_t num)
{
    char buffer[11];
    int i = 0;
    if (num == 0) { UART0_SendChar('0'); return; }
    while (num > 0)
    {
        buffer[i++] = (num % 10) + '0';
        num /= 10;
    }
    while (i--) UART0_SendChar(buffer[i]);
}

uint32_t UART0_ReceiveUInt(void)
{
    char c;
    uint32_t value = 0;
    while (1)
    {
        c = UART0_ReceiveChar();
        if (c == '\r' || c == '\n') break;
        if (c >= '0' && c <= '9') value = (value * 10) + (c - '0');
    }
    return value;
}

uint8_t UART0_IsDataAvailable(void)
{
    /* Checks UART5 Flag Register */
    return ((UART5_FR_R & UART_FR_RXFE) == 0) ? 1 : 0;
}