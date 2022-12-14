/**
 * @file tobylog.h
 * @author Tobias Heukäufer
 * @brief General Tobylog management. 
 */

#ifndef TLOG_INCLUDE_TOBYLOG_H
#define TLOG_INCLUDE_TOBYLOG_H

#include <stdlib.h>

#include <apr_pools.h>

#include "../include/widget.h"

/** @brief Tobylog function results. */
typedef enum tlog_result {
    /** @brief OK or success. */
    TLOG_RESULT_OK = 0,
    /** @brief Cancel. */
    TLOG_RESULT_CANCEL,
    /** @brief Failure. **/
    TLOG_RESULT_FAIL
} TLog_Result;

/**
 * @brief Initializes Tobylog.
 * 
 * For every call this function, @ref TLog_Terminate() must be called.
 * 
 * @param pool Memory pool
 * @return @ref TLog_Result::TLOG_RESULT_OK on success, or @ref TLog_Result::TLOG_RESULT_FAIL else
 */
TLog_Result TLog_Init(apr_pool_t* pool);

/**
 * @brief Runs Tobylog with a list of widgets.
 * 
 * This function will run until the user presses Return or Esc (and the currently
 * focused widget won't consume the input).
 * 
 * @param widgets NULL-terminated array of widgets to be displayed from top to bottom
 * @return @ref TLog_Result::TLOG_RESULT_OK on Return, @ref TLog_Result::TLOG_RESULT_CANCEL on Esc, or @ref TLog_Result::TLOG_RESULT_FAIL on failure
 */
TLog_Result TLog_Run(TLog_Widget** widgets);

#endif