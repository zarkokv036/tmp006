/**
* @file tm4c_uart.h
* @brief init file for uart
*
* @author Zarko Milojicic
*/

#ifndef TM4C123_UART_H
#define  TM4C123_UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Driver_USART.h"

/* USART Driver */
ARM_DRIVER_USART Driver_USART0;
static ARM_DRIVER_USART * USARTdrv = &Driver_USART0;

/**
* @brief init of uart
*/
int uart_init(void);

/**
* @brief uart receive operation
*/
int uart_transmit (uint16_t *data);

#ifdef __cplusplus
}
#endif

#endif //TM4C123_UART_H
