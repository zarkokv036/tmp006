/**
* @file main.c
* @brief Test of tmp006 driver
*
* @author Zarko Milojicic
*/
#include "platform.h"
#include "test.h"


int main(void)
{
    platform_init();
    test_init();
    test_run();

    return 0;
} //MAIN
