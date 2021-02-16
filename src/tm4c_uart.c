/**
* @file tm4c_uart.h
* @brief init file for uart
*
* @author Zarko Milojicic
*/

#include "tm4c_uart.h"
#include <errno.h>

static void myUSART_callback(uint32_t event)
{
  uint32_t mask;
  mask = ARM_USART_EVENT_RECEIVE_COMPLETE  |
         ARM_USART_EVENT_TRANSFER_COMPLETE |
         ARM_USART_EVENT_SEND_COMPLETE     |
         ARM_USART_EVENT_TX_COMPLETE       ;
//  if (event & mask) {
//    /* Success: Wakeup Thread */
//    osSignalSet(tid_myUART_Thread, 0x01);
//  }
  if (event & ARM_USART_EVENT_RX_TIMEOUT) {
    //__breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
  }
  if (event & (ARM_USART_EVENT_RX_OVERFLOW | ARM_USART_EVENT_TX_UNDERFLOW)) {
    //__breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
  }
}

int uart_init(void)
{
    /*Initialize the USART driver */
    USARTdrv->Initialize(myUSART_callback);
    /*Power up the USART peripheral */
    USARTdrv->PowerControl(ARM_POWER_FULL);
    /*Configure the USART to 9600 Bits/sec */
    USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
                      ARM_USART_DATA_BITS_8 |
                      ARM_USART_PARITY_NONE |
                      ARM_USART_STOP_BITS_1 |
                      ARM_USART_FLOW_CONTROL_NONE, 9600);
     
    /* Enable Receiver and Transmitter lines */
    USARTdrv->Control (ARM_USART_CONTROL_TX, 1);
    USARTdrv->Control (ARM_USART_CONTROL_RX, 1);
    
    //USARTdrv->Send("\nPress Enter to receive a message", 34);
    //USARTdrv->Receive(&cmd, 1);          /* Get byte from UART */
    return 0;
}

int uart_transmit(uint16_t *data)
{
    uint8_t value[2];
    if(data == NULL)
    {
        return -EINVAL;
    }
 
    value[0] = (uint8_t)(*data >> 8);
    value[1] = (uint8_t)(*data & 0x00FF);
    
    int status = USARTdrv->Send(data, 2);
    if(status != 0)
    {
        return status;
    }
    return 0;
}
