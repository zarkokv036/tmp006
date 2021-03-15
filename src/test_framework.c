/**
* @file test_framework.c
* @brief test cases for tmp006
*
* @author Zarko Milojicic
*/

#include <stdint.h>
#include <stdbool.h>
#include "test.h"

/** @brief counter of miliseconds, incremented in timer handler*/    
volatile uint32_t msCounter = 0; 
/** @brief counter of received results */    
volatile uint32_t resultCounter = 0;
/** @brief when set calculation can be performed*/
volatile uint8_t resultReadyFlag = 0;

TMP006_Device senzor = {
        .i2cRead = platform_i2cRead,
        .i2cWrite = platform_i2cWrite   
    };
   
    
void pinInterruptHandler(void)
{
    resultCounter++ ;
    resultReadyFlag = 1;
}

void timerHandler(void)
{
    msCounter++ ;
}    
    
void setUp(void)
{
    msCounter = 0;
    resultCounter = 0;
    resultReadyFlag = 0;
}

bool checkConfigReg(uint16_t mask, uint16_t checkValue)
{
    uint16_t valueOfReg;
    
    int status = tmp006_read(&senzor, TMP006_CONFIG, &valueOfReg);
    
    if (status != 0)                     
    {                                    
        return false;                    
    } 
    
    valueOfReg &= mask;                  
    TEST_ASSERT(valueOfReg == checkValue);
    
    return true;
}



void test_init(void)
{
    platform_configure1msInterrupt(timerHandler);

    platform_configureInterruptPin(pinInterruptHandler);
}