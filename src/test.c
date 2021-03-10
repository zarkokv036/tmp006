/**
* @file test.c
* @brief test cases for tmp006 driver
*
* @author Zarko Milojicic
*/

#include "tmp006/tmp006.h"
#include "test.h"

void test_run(void)
{
    tmp006_init(&senzor, TMP006_PIN_LOW, TMP006_PIN_LOW);
    
    PRINTF("~~~TEST~~~ \n");
    
    RUN_TEST("Read manufacturer ID", test_readManufId);
    
    RUN_TEST("Writing into config register", test_writeIntoConfig);
    
    //test conversion rate with interrupt enabled
    RUN_TEST("Check 1 conversion per second rate with interrupt enabled (wait)", test_customConvRateIntOn, TMP006_CONVERSION_RATE_1_CONV_PER_SEC);
    RUN_TEST("Check 2 conversion per second rate with interrupt enabled (wait)", test_customConvRateIntOn, TMP006_CONVERSION_RATE_2_CONV_PER_SEC);
    RUN_TEST("Check 4 conversion per second rate with interrupt enabled (wait)", test_customConvRateIntOn, TMP006_CONVERSION_RATE_4_CONV_PER_SEC);
    RUN_TEST("Check 0.5 conversion per second rate with interrupt enabled (wait)", test_customConvRateIntOn, TMP006_CONVERSION_RATE_0_5_CONV_PER_SEC);
    RUN_TEST("Check 0.25 conversion per second rate with interrupt enabled (wait)", test_customConvRateIntOn, TMP006_CONVERSION_RATE_0_25_CONV_PER_SEC);
    
    //test conversion rate with interrupt disabled
    RUN_TEST("Check 1 conversion per second rate with interrupt disabled (wait)", test_customConvRateIntOff, TMP006_CONVERSION_RATE_1_CONV_PER_SEC);
    RUN_TEST("Check 2 conversion per second rate with interrupt disabled (wait)", test_customConvRateIntOff, TMP006_CONVERSION_RATE_2_CONV_PER_SEC);
    RUN_TEST("Check 4 conversion per second rate with interrupt disabled (wait)", test_customConvRateIntOff, TMP006_CONVERSION_RATE_4_CONV_PER_SEC);
    RUN_TEST("Check 0.5 conversion per second rate with interrupt disabled (wait)", test_customConvRateIntOff, TMP006_CONVERSION_RATE_0_5_CONV_PER_SEC);
    RUN_TEST("Check 0.25 conversion per second rate with interrupt disabled (wait)", test_customConvRateIntOff,TMP006_CONVERSION_RATE_0_25_CONV_PER_SEC);
    
    //test power down operation mode
    RUN_TEST("Check power down mode with interrupt enabled (wait)", test_powerDownModeIntOn);
    RUN_TEST( "Check power down mode with interrupt disabled (wait)", test_powerDownModeIntOff);
    
    tmp006_resetDevice(&senzor);
    
    PRINTF("~~~TEST END~~~ \n");
}

