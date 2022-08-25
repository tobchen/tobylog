#ifndef TLOG_INCLUDE_WIDGET_H
#define TLOG_INCLUDE_WIDGET_H

#include <stdint.h>

typedef enum tlog_widget_action {
    TLOG_WIDGET_ACTION_ENTER,
    TLOG_WIDGET_ACTION_ESC,
    TLOG_WIDGET_ACTION_BACKSPACE,
    TLOG_WIDGET_ACTION_UP,
    TLOG_WIDGET_ACTION_DOWN,
    TLOG_WIDGET_ACTION_LEFT,
    TLOG_WIDGET_ACTION_RIGHT,
    TLOG_WIDGET_ACTION_TAB
} TLog_Widget_Action;

typedef uint32_t (*TLog_Widget_GetPreferedWidth) (void* widget);
typedef uint32_t (*TLog_Widget_SetMaximumWidth) (void* widget, uint32_t maxWidth, uint32_t screenHeight);
typedef void (*TLog_Widget_GetLine) (void* widget, uint32_t lineY, char* buffer,
        uint32_t* writtenLength, int* isReversed);
typedef void (*TLog_Widget_SetFocus) (void* widget, uint32_t* cursorX, uint32_t* cursorY);
typedef void (*TLog_Widget_PutChar) (void* widget, char ch,
        uint32_t* cursorX, uint32_t* cursorY, uint32_t* dirtyStart, uint32_t* dirtyEnd);
typedef int (*TLog_Widget_PutAction) (void* widget, TLog_Widget_Action action,
        uint32_t* cursorX, uint32_t* cursorY, uint32_t* dirtyStart, uint32_t* dirtyEnd);

typedef struct tlog_widget_data {
    TLog_Widget_GetPreferedWidth getPreferedWidth;
    TLog_Widget_SetMaximumWidth setMaximumWidth;
    TLog_Widget_GetLine getLine;
    TLog_Widget_SetFocus setFocus;
    TLog_Widget_PutChar putChar;
    TLog_Widget_PutAction putAction;
} TLog_Widget_Data;

typedef struct tlog_widget {
    TLog_Widget_Data* data;
} TLog_Widget;

#endif