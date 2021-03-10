/**
* @file test.h
* @brief test cases for tmp006 driver
*
* @author Zarko Milojicic
*/

#include <stdint.h>
#include "tmp006/tmp006.h"
#include "platform.h"

#ifndef TEST_H
#define  TEST_H

#ifdef __cplusplus
extern "C" {
#endif


/**
* @brief Helper macro for parametar checking
*/    
#define TEST_ASSERT(expr)                 \
    do                                   \
    {                                    \
        bool e = expr;                   \
        if (!e)                          \
        {                                \
            return false;                \
        }                                \
                                         \
    } while (0)
    

/**
* @brief Helper macro for test running
* Print PASS if test was success or FAIL if not
*/
#define RUN_TEST(testName, testCase, ...)                    \
    do                                                       \
    {                                                        \
        PRINTF("Test case: %s", (testName));                 \
        bool success = (testCase)(__VA_ARGS__);              \
        PRINTF(success ? " -> PASS \n" : " -> FAIL \n");     \
    } while (0)                                              \


/** @brief counter of miliseconds, incremented in timer handler*/    
extern volatile uint32_t msCounter; 
/** @brief counter of received results */    
extern volatile uint32_t resultCounter;
/** @brief when set calculation can be performed*/
extern volatile uint8_t resultReadyFlag;

extern TMP006_Device senzor;


/**
* @brief init of interrupt handler that are used in tests.
*
* @note Must be called before  tests_run().
* @note pinInterruptHandler() and timerHandler() in test.c file 
* need to be modified due to use of different platform
*/
void test_init(void);

/**
* @brief Tests performing
*
* Test of reading manufacturer ID
* Test of Writing config register
* Test of Conversion rates with interrupt enabled
* Test of Conversion rates with interrupt disabled
* Test of Power-Down operation mode with interrupt enabled and disabled
* Device is reset at the end of test.
*/
void test_run(void);
    
/**
* @brief function for system set up before every test
*/      
void setUp(void);
    
/**
* @brief handler for edge interrupt on pin.
* Announce that result is ready via resultReadyFlag and counts the results.
*/   
void pinInterruptHandler(void);

/**
* @brief handler for timer, 1 ms timer
*/
void timerHandler(void);

/**
* @brief reading manufacturer id from device
*
* @return true if test success or false if not
*/
bool test_readManufId(void);

/**
* @brief test if values of commands are written into config register
*
* @return true if test success or false if not
*/
bool test_writeIntoConfig(void);


/**
* @brief test of different conversion rate with disabled interrupt pin
*
* @param convRate speed of conversion
* @return true if test success or false if not
*/
bool test_customConvRateIntOff(enum TMP006_ConversionRate convRate);


/**
* @brief test of different conversion rate with enabled interrupt pin
*
* @param convRate speed of conversion 
* @return true if test success or false if not
*/
bool test_customConvRateIntOn(enum TMP006_ConversionRate convRate);

/**
* @brief test power-down operation mode with interrupt enabled
*
* @return true if test success or false if not
*/
bool test_powerDownModeIntOn(void);

/**
* @brief test power-down operation mode with interrupt disabled
*
* @return true if test success or false if not
*/
bool test_powerDownModeIntOff(void);

#ifdef __cplusplus
}
#endif

#endif //TEST_H