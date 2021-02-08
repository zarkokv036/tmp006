/**
* @file tm4c_init.c
* @brief init file for tm4c123g
*
* @author Zarko Milojicic
*/

#include "tm4c_init.h"

void systemClockInit_40MHz(void)
{
    SysCtlClockSet(SYSCTL_XTAL_16MHZ | SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN );
}


