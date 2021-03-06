/**
* @file test.h
* @brief test cases for tmp006 driver
*
* @author Zarko Milojicic
*/



#ifndef TEST_H
#define  TEST_H

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif //TEST_H