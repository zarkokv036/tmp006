/**
* @file tm4c_init.c
* @brief init file for tm4c123g
*
* @author Zarko Milojicic
*/
#define PART_TM4C123GH6PM

#include "tm4c_init.h"
#include "../inc/hw_i2c.h"
#include "../inc/hw_types.h"
//#include "../inc/hw_gpio.h"
#include "../driverlib/pin_map.h"


void initSystemClock_40MHz(void)
{
    SysCtlClockSet(SYSCTL_XTAL_16MHZ | SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN);
}

void enablePeripheralsClock(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
   // SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0))
    {
    }
}

void initPorts(void (*portA2IntHandler)(void))
{
    GPIOUnlockPin(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, (GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3));
    
    GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_2);
    
    // Register the port-level interrupt handler. This handler is the first
    // level interrupt handler for all the pin interrupts.
    GPIOIntRegister(GPIO_PORTA_BASE, portA2IntHandler);
    
    // Make pin 2 falling edge triggered interrupts.
    GPIOIntTypeSet(GPIO_PORTA_BASE, GPIO_PIN_2, GPIO_FALLING_EDGE);
    GPIOIntEnable(GPIO_PORTA_BASE, GPIO_PIN_2);   
}

void initTimer1mSec(void (*pfnHandler)(void))
{
    // Enable the Timer0 peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    // Wait for the Timer0 module to be ready.
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0))
    {
    }

    // Configure Timer0 as a full-width periodic timer
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);    

    // Set the count time for the the periodic timer (TimerA).
    //1ms sec
    TimerLoadSet(TIMER0_BASE, TIMER_A, 40000);
    
    TimerIntRegister(TIMER0_BASE, TIMER_A, pfnHandler);
    
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    TimerEnable(TIMER0_BASE, TIMER_A); 
}

void initI2c() //izbaci slave addr
{
    //enable I2C module 1
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);
 
    //reset module
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C1);
     
    //enable GPIO peripheral that contains I2C 1
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA))
    {
    }
    
    // Configure the pin muxing for I2C1 functions on port pa6 and pa7.
    GPIOPinConfigure(GPIO_PA6_I2C1SCL );
    GPIOPinConfigure(GPIO_PA7_I2C1SDA);
    
    // Select the I2C function for these pins.
    GPIOPinTypeI2CSCL(GPIO_PORTA_BASE, GPIO_PIN_6);
    GPIOPinTypeI2C(GPIO_PORTA_BASE, GPIO_PIN_7);

    //100kbs
    I2CMasterInitExpClk(I2C1_BASE, SysCtlClockGet(), false);

    HWREG(I2C1_BASE + I2C_O_FIFOCTL) = 80008000;
}

int i2cRead(uint8_t slaveAddr, uint8_t reg, uint8_t *data, uint16_t length)
{
     I2CMasterSlaveAddrSet(I2C1_BASE, slaveAddr, false);  // true -> initiate read from slave, false -> write to slave 
     I2CMasterDataPut(I2C1_BASE, reg);
     I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);
     while(I2CMasterBusy(I2C1_BASE))
     {
     }
     
     I2CMasterSlaveAddrSet(I2C1_BASE, slaveAddr, true);
     I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
     
     //wait for MCU to finish transaction
     while(I2CMasterBusy(I2C1_BASE))
     {
     }
     
     for(uint16_t i = 0; i < length; i++)
      {
          *(data + i) = I2CMasterDataGet(I2C1_BASE);
          if(i == (length - 1))
          {
              I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
          }
          else
          {
              I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
          }
          while(I2CMasterBusy(I2C1_BASE))
          {
          }  
      }
    
      return 0;
}

int i2cWrite(uint8_t slaveAddr, uint8_t reg, uint8_t *data, uint16_t length)
{
    I2CMasterSlaveAddrSet(I2C1_BASE, slaveAddr, false);
    I2CMasterDataPut(I2C1_BASE, reg);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(I2C1_BASE))
    {
    }
    for(uint16_t i = 0; i < length; i++)
    {
        I2CMasterDataPut(I2C1_BASE, data[i]);
        if(i == (length - 1))
        {
            I2CMasterControl(I2C1_BASE,I2C_MASTER_CMD_BURST_SEND_FINISH);
        }
        else
        {
            I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
        }
        while(I2CMasterBusy(I2C1_BASE))
        {
        }
    }
    return 0;
}

void initUartPrintf(void)
{
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    //
    // Enable UART1 functionality on GPIO Port A pins 0 and 1.
    //
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    
    UARTStdioConfig(0, 115200, SysCtlClockGet());
}

