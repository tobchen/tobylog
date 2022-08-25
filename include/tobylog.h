#ifndef TLOG_INCLUDE_TOBYLOG_H
#define TLOG_INCLUDE_TOBYLOG_H

#include <stdlib.h>

#include "../include/widget.h"

typedef enum tlog_result {
    TLOG_RESULT_SUCCESS = 0,
    TLOG_RESULT_CANCEL,
    TLOG_RESULT_FAIL
} TLog_Result;

TLog_Result TLog_Init(void);
void TLog_Terminate(void);

TLog_Result TLog_Run(void** widgets, size_t widgetCount);

#endif