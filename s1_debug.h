#ifndef __S1_DEBUG_H_
#define __S1_DEBUG_H_

/**
 * @file  s1_debug.h
 * @brief Handy debugging and logging macros
 */

#include "SEGGER_RTT.h"
#include "sdk_config.h"

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
#define LOG(format, ...) do {char _debug_log_buffer[SEGGER_RTT_CONFIG_BUFFER_SIZE_UP-1] = ""; \
                snprintf(_debug_log_buffer, SEGGER_RTT_CONFIG_BUFFER_SIZE_UP-1, format, ##__VA_ARGS__); \
                SEGGER_RTT_Write(0, _debug_log_buffer, strnlen(_debug_log_buffer, SEGGER_RTT_CONFIG_BUFFER_SIZE_UP)); \
                } while(0)

/**
 * @brief Clears the terminal screen of any previous logs.
 */ 
#define LOG_CLEAR() SEGGER_RTT_printf(0, RTT_CTRL_CLEAR);

/**
 * @brief Macro to log errors. Prints file, 
 *        line number and a return value.
 * 
 * @param ret_value: Value to show in the log
 */ 
#define app_error(ret_value) LOG("Error at "__FILE__":%u - Returned value: %u\r\n", __LINE__, ret_value)

/**
 * @brief Macro to trigger a software breakpoint
 *        and stop the program.
 */
#define app_assert() do{__asm__("BKPT"); while(1);}while(0);

#endif