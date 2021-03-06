/**
* @file main.c
* @brief Test of tmp006 driver
*
* @author Zarko Milojicic
*/

#include "test/test.h"
#include "hal/hal.h"

int main(void)
{
    hal_init();
    test_init();
    test_run();

    return 0;
} //MAIN
