/**
 * @file  s1.h
 * @brief S1 Module Core API
 *        
 *        Use these functions and macros to configure the S1
 *        Module. All APIs are documented here, and organized
 *        by category.
 * 
 * @attention (c) 2021 Silicon Witchery 
 *            (info@siliconwitchery.com)
 *
 *        Licensed under a Creative Commons Attribution 
 *        4.0 International License. This code is provided
 *        as-is and no warranty is given.
*/

#ifndef _S1_H_
#define _S1_H_
/*******************************************************/

#include "SEGGER_RTT.h"
#include "sdk_config.h"

/**
 * @brief Release version of this SDK
 */
#define __S1_SDK_VERSION__ "0.1"



/*******************************************************
 * Power related APIs
 *******************************************************/

/**
 * @brief Set Vaux voltage. Can be set from 0.8V to 5.5V
 *        in 50mV steps.
 * 
 * @param voltage: Voltage level. Automatically rounded
 *                 to the nearest 50mV. A value of 0V
 *                 shuts down the rail.
 * 
 * @note  Will return false if Vio is set to Load Switch
 *        mode, and Vaux is set to a value greater than
 *        3.46V. This is to protect the FPGA.
 * 
 * @retval true if successful, false otherwise
 */
bool s1_pmic_set_vaux(float voltage);

/**
 * @brief Set Vio voltage. Can be set from 0.8V to 
 *        3.975V in 25mV steps. 
 * 
 * @param voltage: Voltage level. Automatically rounded
 *                 to the nearest 25mV. A value of 0V
 *                 shuts down the rail.
 * 
 * @retval true if successful, false otherwise
 */
bool s1_pmic_set_vio(float voltage);

/**
 * @brief Switches Vio between Low Dropout Regulator mode
 *        and Load Switch mode. In Load switch mode, Vio
 *        outputs the same voltage as Vaux.
 * 
 * @note  Vio cannot be set to load switch mode if Vaux
 *        greater than 3.46V. This is to protect the
 *        FPGA.
 * 
 * @param enable: True enables load switch mode, false
 *                switches back to regulated mode.
 * 
 * @retval true if successful, false otherwise
 */
bool s1_pmic_set_vio_lsw(bool enable);

/**
 * @brief Set Vio voltage. Can be set from 1.7V to 
 *        2.0V in 50mV steps.
 * 
 * @param voltage: Voltage level. Automatically rounded
 *                 to the nearest 50mV.
 * 
 * @retval true if successful, false otherwise
 */
bool s1_pmic_set_vadc(float voltage);

/**
 * @brief Enable FPGA core and FPGA PLL voltage to 1.2V.
 *
 * @param enable: Enable the core voltage.
 */
void s1_pimc_fpga_vcore(bool enable);


/*******************************************************
 * Basic Logging Macros
 *******************************************************/

/**
 * @brief Clears the terminal screen of any previous logs.
 */ 
#define LOG_CLEAR() SEGGER_RTT_printf(0, RTT_CTRL_CLEAR "\r");

/**
 * @brief The same logging macro as LOG_RAW, but with
 * "\r\n" appended at the start.
 */

#define LOG(format, ...) LOG_RAW("\r\n" format, ##__VA_ARGS__)

/**
 * @brief Logging macro which outputs printf style logs
 *        over a JLink debugger. To view the terminal,
 *        run JLinkRTTViewer.exe. If a debug session is
 *        already active, JLinkRTTClient.exe should be
 *        used.
 * 
 * @note  The protocol used is called RTT. Read how it
 *        works here: https://wiki.segger.com/RTT
 * 
 *        RTT has a built in printf, however it doesn't
 *        support %f or %d. sprnitf is therefore used to 
 *        get around that limitation.
 *         
 * @param format: printf style format string
 * @param ...: Variadic argument list for printf data
 */
#define LOG_RAW(format, ...)                                                                                    \
    do                                                                                                          \
    {                                                                                                           \
        char _debug_log_buffer[SEGGER_RTT_CONFIG_BUFFER_SIZE_UP-1] = "";                                        \
        snprintf(_debug_log_buffer, SEGGER_RTT_CONFIG_BUFFER_SIZE_UP-1, format, ##__VA_ARGS__);                 \
        SEGGER_RTT_Write(0, _debug_log_buffer, strnlen(_debug_log_buffer, SEGGER_RTT_CONFIG_BUFFER_SIZE_UP));   \
    } while(0)


/*******************************************************
 * Error Handling
 *******************************************************/

/**
 * @brief Macro to log errors. Prints file, 
 *        line number and a return value.
 * 
 * @param ret_value: Value to show in the log
 */ 
#define s1_app_error(ret_value) \
    LOG("Error at "__FILE__":%u - Returned value: %u\r\n",\
        __LINE__, ret_value)

/**
 * @brief Macro to trigger a software breakpoint
 *        and stop the program.
 */
#define s1_app_assert() do{__asm__("BKPT"); while(1);}while(0);


/*******************************************************/
#endif