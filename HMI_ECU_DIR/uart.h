/******************************************************************************
 * File: uart.h
 * Module: UART (Universal Asynchronous Receiver/Transmitter)
 * Description: Header file for TM4C123GH6PM UART0 Driver (Register Level)
 * Author: Ahmedhh
 * Date: December 10, 2025
 * 
 * Configuration:
 *   - UART0 (PA0: RX, PA1: TX)
 *   - Baud Rate: 115200
 *   - Data: 8 bits
 *   - Parity: None
 *   - Stop: 1 bit
 ******************************************************************************/

#ifndef UART_H_
#define UART_H_

#include <stdint.h>

/******************************************************************************
 *                          Function Prototypes                                *
 ******************************************************************************/

/*
 * UART0_Init
 * Initializes UART0 with 115200 baud rate, 8N1 configuration.
 * Uses PA0 (RX) and PA1 (TX).
 * System clock is assumed to be 16 MHz.
 */
void UART0_Init(void);

/*
 * UART0_SendChar
 * Transmits a single character through UART0.
 * Blocks until the transmit FIFO is ready.
 * 
 * Parameters:
 *   data - Character to transmit
 */
void UART0_SendChar(char data);

/*
 * UART0_ReceiveChar
 * Receives a single character from UART0.
 * Blocks until a character is available in the receive FIFO.
 * 
 * Returns:
 *   Received character
 */
char UART0_ReceiveChar(void);

/*
 * UART0_SendString
 * Transmits a null-terminated string through UART0.
 * 
 * Parameters:
 *   str - Pointer to null-terminated string to transmit
 */
void UART0_SendString(const char *str);

void UART0_ReceiveString(char *buffer);

void UART0_SendUInt(uint32_t num);

uint32_t UART0_ReceiveUInt(void);
/*
 * UART0_IsDataAvailable
 * Checks if data is available in the receive FIFO.
 * 
 * Returns:
 *   1 if data is available, 0 otherwise
 */
uint8_t UART0_IsDataAvailable(void);

#endif /* UART_H_ */