/******************************************************************************
 * File: uart.c
 * Module: UART (Mapped to UART5 Hardware)
 * Description: TM4C123GH6PM UART5 Driver
 * NOTE: API names are UART0_... but control UART5 (PE4, PE5)
 ******************************************************************************/

#include "uart.h"
#include "tm4c123gh6pm.h"

/* ================= UART5 Register Abstraction ================= */
#define UART_DR_R     UART5_DR_R
#define UART_FR_R     UART5_FR_R
#define UART_CTL_R    UART5_CTL_R
#define UART_IBRD_R   UART5_IBRD_R
#define UART_FBRD_R   UART5_FBRD_R
#define UART_LCRH_R   UART5_LCRH_R

/* =============================================================== */

void UART0_Init(void)
{
    volatile uint32_t delay;

    /* 1. Enable clocks */
    SYSCTL_RCGCUART_R |= (1U << 5);    /* UART5 */
    SYSCTL_RCGCGPIO_R |= (1U << 4);    /* GPIOE */
    delay = SYSCTL_RCGCGPIO_R;

    /* 2. Configure GPIOE PE4, PE5 */
    GPIO_PORTE_AFSEL_R |= 0x30;        /* PE4, PE5 AF */
    GPIO_PORTE_PCTL_R = (GPIO_PORTE_PCTL_R & 0xFF00FFFF) | 0x00110000;
    GPIO_PORTE_DEN_R  |= 0x30;
    GPIO_PORTE_AMSEL_R &= ~0x30;

    /* 3. Disable UART before config */
    UART_CTL_R &= ~UART_CTL_UARTEN;

    /* 4. Baud rate: 115200 @ 16 MHz */
    UART_IBRD_R = 8;
    UART_FBRD_R = 44;

    /* 5. 8N1 + FIFO */
    UART_LCRH_R = UART_LCRH_WLEN_8 | UART_LCRH_FEN;

    /* 6. Enable UART, TX, RX */
    UART_CTL_R = UART_CTL_UARTEN | UART_CTL_TXE | UART_CTL_RXE;
}

/* ================= Blocking APIs ================= */

void UART0_SendChar(char data)
{
    while (UART_FR_R & UART_FR_TXFF);
    UART_DR_R = data;
}

char UART0_ReceiveChar(void)
{
    while (UART_FR_R & UART_FR_RXFE);
    return (char)(UART_DR_R & 0xFF);
}

void UART0_SendString(const char *str)
{
    while (*str)
    {
        UART0_SendChar(*str++);
    }
}

uint8_t UART0_IsDataAvailable(void)
{
    return ((UART_FR_R & UART_FR_RXFE) == 0);
}

/* ================= Utilities ================= */

void UART0_SendUInt(uint32_t num)
{
    char buf[11];
    int i = 0;

    if (num == 0)
    {
        UART0_SendChar('0');
        UART0_SendChar('\n'); // ???? ????? ???????
        return;
    }

    while (num)
    {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }

    while (i--)
    {
        UART0_SendChar(buf[i]);
    }

    UART0_SendChar('\n'); // ???? ????? ???????
}


uint32_t UART0_ReceiveUInt(void)
{
    char c;
    uint32_t val = 0;

    while (1)
    {
        c = UART0_ReceiveChar();
        if (c == '\r' || c == '\n')
            break;

        if (c >= '0' && c <= '9')
            val = (val * 10) + (c - '0');
    }
    return val;
}

void UART0_ReceiveString(char *buffer)
{
    char c;
    uint32_t i = 0;

    while (1)
    {
        c = UART0_ReceiveChar();
        if (c == '\r' || c == '\n')
            break;

        buffer[i++] = c;
    }
    buffer[i] = '\0';
}
