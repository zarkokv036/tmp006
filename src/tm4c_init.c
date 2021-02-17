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
#include "../inc/hw_gpio.h"
#include "../driverlib/pin_map.h"

void initSystemClock_40MHz(void)
{
    SysCtlClockSet(SYSCTL_XTAL_16MHZ | SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN );
}

void enablePeripheralsClock(void)
{
    //SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
   // SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0))
    {
    }
}

void initPorts(void)
{
    GPIOUnlockPin(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, (GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3));
   
    
//    GPIODirModeSet(GPIO_PORTF_BASE, (GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3), GPIO_DIR_MODE_OUT);
//    GPIODirModeSet(GPIO_PORTA_BASE, GPIO_PIN_2, GPIO_DIR_MODE_IN);
//    GPIOPadConfigSet(GPIO_PORTF_BASE,(GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3), GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
}

void initTimer(void (*pfnHandler)(void))
{
    // Enable the Timer0 peripheral
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    //
    // Wait for the Timer0 module to be ready.
    //
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0))
    {
    }
    //
    // Configure Timer0 as a full-width periodic timer
    //
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);    
    
    //
    // Set the count time for the the periodic timer (TimerA).
    //1 sec
    TimerLoadSet(TIMER0_BASE, TIMER_A, 40000000);
    
    TimerIntRegister(TIMER0_BASE, TIMER_A, pfnHandler);
    
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);///nisam siguran
    
    
    TimerEnable(TIMER0_BASE, TIMER_A);
    
}


/* Initialize I2C connected EEPROM */
void initI2c(uint8_t slaveAddr) 
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
   // GPIOPinConfigure(GPIO_PCTL_PA6_I2C1SCL);
   // GPIOPinConfigure(GPIO_PCTL_PA7_I2C1SDA);
    GPIOPinConfigure(GPIO_PA6_I2C1SCL );
    GPIOPinConfigure(GPIO_PA7_I2C1SDA);
    // Select the I2C function for these pins.
    GPIOPinTypeI2CSCL(GPIO_PORTA_BASE, GPIO_PIN_6);
    GPIOPinTypeI2C(GPIO_PORTA_BASE, GPIO_PIN_7);

    //100kbs
    I2CMasterInitExpClk(I2C1_BASE, SysCtlClockGet(), false);
   //I2CMasterSlaveAddrSet(I2C1_BASE, slaveAddr, bool bReceive); // true -> initiate read from slave, false -> write to slave 
    HWREG(I2C1_BASE + I2C_O_FIFOCTL) = 80008000;
}

int i2cRead(uint8_t slaveAddr, uint8_t reg, uint8_t *data, uint16_t length)
{
     I2CMasterSlaveAddrSet(I2C1_BASE, slaveAddr, false);
    
     I2CMasterDataPut(I2C1_BASE, reg);
 
     I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);
     while(I2CMasterBusy(I2C1_BASE))
     {
     }
     
     I2CMasterSlaveAddrSet(I2C1_BASE, slaveAddr, true);
     
    //send control byte and read from the register we
    //specified
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
     
    //wait for MCU to finish transaction
    while(I2CMasterBusy(I2C1_BASE))
    {
    }
     
    //return data pulled from the specified register
//     I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
//     while(I2CMasterBusBusy(I2C1_BASE))
//     {
//     }
     for(uint16_t i = 0; i < length; i++)
     {
         *(data + i) = I2CMasterDataGet(I2C1_BASE);
         I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
         while(I2CMasterBusy(I2C1_BASE))
         {
         }  
     }
//     I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
//     while(I2CMasterBusBusy(I2C1_BASE))
//     {
//     }
     return 0;
}

int i2cWrite(uint8_t slaveAddr, uint8_t reg, uint8_t *data, uint16_t length)
{
    I2CMasterSlaveAddrSet(I2C1_BASE, slaveAddr, false);
    
    I2CMasterDataPut(I2C1_BASE, reg);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusBusy(I2C1_BASE))
    {
    }
    //moze da se desi da greska bude ovde, ako bude ubaci ovde dataPut(reg) i burst cont
    
    for(uint16_t i = 0; i < length; i++)
    {
        
        I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
        while(I2CMasterBusBusy(I2C1_BASE))
        {
        }
        I2CMasterDataPut(I2C1_BASE, data[i]);
    }
//    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
//    while(I2CMasterBusBusy(I2C1_BASE))
//    {
//    }
    return 0;
}


