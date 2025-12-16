/******************************************************************************
 * File: uart.c
 * Module: UART (Universal Asynchronous Receiver/Transmitter)
 * Description: Source file for TM4C123GH6PM UART0 Driver (Register Level)
 * Author: Ahmedhh
 * Date: December 10, 2025
 * 
 * Configuration:
 *   - UART0 (PA0: RX, PA1: TX)
 *   - Baud Rate: 115200
 *   - Data: 8 bits
 *   - Parity: None
 *   - Stop: 1 bit
 *   - System Clock: 16 MHz
 ******************************************************************************/

#include "uart.h"
#include "tm4c123gh6pm.h"

/******************************************************************************
 *                              Definitions                                    *
 ******************************************************************************/

#define SYSTEM_CLOCK    16000000    /* 16 MHz system clock */
#define BAUD_RATE       115200      /* Target baud rate */

/******************************************************************************
 *                          Function Implementations                           *
 ******************************************************************************/

/*
 * UART0_Init
 * Initializes UART0 with 115200 baud rate, 8N1 configuration.
 * 
 * Baud Rate Calculation:
 *   BRD = BRDI + BRDF = UARTSysClk / (16 * Baud Rate)
 *   BRD = 16,000,000 / (16 * 115200) = 8.680555
 *   BRDI = 8
 *   BRDF = 0.680555
 *   UARTFBRD[DIVFRAC] = integer(BRDF * 64 + 0.5) = integer(43.555 + 0.5) = 44
 */
void UART0_Init(void)
{
    volatile uint32_t delay;
    
    /* 1. Enable clock for UART0 */
    SYSCTL_RCGCUART_R |= 0x01;      /* Enable UART0 clock (bit 0) */
    delay = SYSCTL_RCGCUART_R;      /* Wait for clock to stabilize */
    
    /* 2. Enable clock for GPIOA */
    SYSCTL_RCGCGPIO_R |= 0x01;      /* Enable PORTA clock (bit 0) */
    delay = SYSCTL_RCGCGPIO_R;      /* Wait for clock to stabilize */
    
    /* 3. Disable UART0 before configuration */
    UART0_CTL_R &= ~UART_CTL_UARTEN;
    
    /* 4. Configure baud rate */
    /* BRD = 16,000,000 / (16 * 115,200) = 8.680555 */
    UART0_IBRD_R = 8;               /* Integer part of baud rate divisor */
    UART0_FBRD_R = 44;              /* Fractional part: 0.680555 * 64 + 0.5 = 44 */
    
    /* 5. Configure Line Control Register (LCRH) */
    /* 8-bit data (WLEN = 11), no parity (PEN = 0), 1 stop bit (STP2 = 0), FIFOs enabled (FEN = 1) */
    UART0_LCRH_R = (UART_LCRH_WLEN_8 | 0x10);  /* 8-bit, no parity, 1 stop, FIFOs enabled */
    
    /* 6. Configure UART0 Control Register */
    /* Enable UART (UARTEN), transmit (TXE), and receive (RXE) */
    UART0_CTL_R = (UART_CTL_UARTEN | UART_CTL_TXE | UART_CTL_RXE);
    
    /* 7. Configure GPIO pins PA0 (RX) and PA1 (TX) */
    /* Enable alternate function */
    GPIO_PORTA_AFSEL_R |= 0x03;     /* Enable alternate function on PA0, PA1 */
    
    /* Configure PCTL for UART function (PMCx = 1 for UART) */
    GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R & 0xFFFFFF00) | 0x00000011;
    
    /* Enable digital function */
    GPIO_PORTA_DEN_R |= 0x03;       /* Enable digital on PA0, PA1 */
    
    /* Disable analog function */
    GPIO_PORTA_AMSEL_R &= ~0x03;    /* Disable analog on PA0, PA1 */
}

/*
 * UART0_SendChar
 * Transmits a single character through UART0.
 * Blocks until the transmit FIFO is ready.
 */
void UART0_SendChar(char data)
{
    /* Wait until transmit FIFO is not full (TXFF = 0) */
    while ((UART0_FR_R & UART_FR_TXFF) != 0);
    
    /* Write data to data register */
    UART0_DR_R = data;
}

/*
 * UART0_ReceiveChar
 * Receives a single character from UART0.
 * Blocks until a character is available in the receive FIFO.
 */
char UART0_ReceiveChar(void)
{
    /* Wait until receive FIFO is not empty (RXFE = 0) */
    while ((UART0_FR_R & UART_FR_RXFE) != 0);
    
    /* Read and return data from data register */
    return (char)(UART0_DR_R & 0xFF);
}

/*
 * UART0_SendString
 * Transmits a null-terminated string through UART0.
 */
void UART0_SendString(const char *str)
{
    while (*str != '\0')
    {
        UART0_SendChar(*str);
        str++;
    }
}

void UART0_ReceiveString(char *buffer)
{
    char c;
    uint32_t i = 0;

    while (1)
    {
        c = UART0_ReceiveChar();   // blocking receive

        if (c == '\r' || c == '\n')   // Enter from PuTTY
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

    if (num == 0)
    {
        UART0_SendChar('0');
        return;
    }

    while (num > 0)
    {
        buffer[i++] = (num % 10) + '0';
        num /= 10;
    }

    while (i--)
    {
        UART0_SendChar(buffer[i]);
    }
}

uint32_t UART0_ReceiveUInt(void)
{
    char c;
    uint32_t value = 0;

    while (1)
    {
        c = UART0_ReceiveChar();

        if (c == '\r' || c == '\n')
            break;

        if (c >= '0' && c <= '9')
        {
            value = (value * 10) + (c - '0');
        }
    }
    return value;
}

/*
 * UART0_IsDataAvailable
 * Checks if data is available in the receive FIFO.
 * Returns 1 if data is available, 0 otherwise.
 */
uint8_t UART0_IsDataAvailable(void)
{
    /* Check if receive FIFO is not empty */
    return ((UART0_FR_R & UART_FR_RXFE) == 0) ? 1 : 0;
}
