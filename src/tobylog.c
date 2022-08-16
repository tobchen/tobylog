#include "../include/tobylog.h"

#include <stdlib.h>

#include <ncurses.h>

static uint32_t initCount = 0;

TLog_Result TLog_Init(void) {
    TLog_Result failure;

    if (initCount > 0) {
        goto success;
    }

    if (!initscr()) {
        failure = TLOG_RESULT_FAIL;
        goto fail_curses;
    }
    cbreak();
    keypad(stdscr, TRUE);
    noecho();

    success:
    ++initCount;
    return TLOG_RESULT_SUCCESS;

    endwin();
    fail_curses:
    return failure;
}

void TLog_Terminate(void) {
    if (initCount == 0) {
        return;
    }

    --initCount;
    if (initCount > 0) {
        return;
    }

    endwin();
}

TLog_Result TLog_Run(void** widgets, uint32_t widgetCount) {
    uint32_t screenWidth, screenHeight;
    uint32_t* heights = NULL;
    char* buffer = NULL;
    size_t bufferLength = 0;
    char* newBuffer;
    TLog_Widget* widget;
    uint32_t widgetWidth;
    uint32_t maxWidth;
    /* The index currently at line 0 */
    uint32_t upperWidgetIndex;
    uint32_t writtenLength;
    int isReversed;
    size_t i, j, y;

    if (initCount == 0) {
        goto fail_init;
    }

    if (widgetCount == 0 || !widgets) {
        goto success;
    }

    heights = malloc(sizeof(uint32_t) * widgetCount);
    if (!heights) {
        goto fail_heights;
    }

    /************** Widget Size Calculation **************/

    screenWidth = COLS;
    screenHeight = LINES;

    maxWidth = 0;
    for (i = 0; i < widgetCount; ++i) {
        widget = (TLog_Widget*) widgets[i];
        widgetWidth = widget->data->getPreferedWidth(widgets[i]);
        maxWidth = widgetWidth > maxWidth ? widgetWidth : maxWidth;
    }
    maxWidth = screenWidth - 1 < maxWidth ? screenWidth - 1 : maxWidth;

    for (i = 0; i < widgetCount; ++i) {
        widget = (TLog_Widget*) widgets[i];
        heights[i] = widget->data->setMaximumWidth(widgets[i], maxWidth, screenHeight);
        if (heights[i] == 0) {
            goto fail_widget_height;
        }
    }

    if (maxWidth > bufferLength) {
        newBuffer = realloc(buffer, sizeof(char) * maxWidth);
        if (!newBuffer) {
            goto fail_buffer;
        }
        buffer = newBuffer;
        bufferLength = maxWidth;
    }

    /************** Initial Draw **************/

    attrset(A_NORMAL);
    clear();
    for (i = 0, y = 0; i < widgetCount && y < screenHeight; ++i) {
        widget = (TLog_Widget*) widgets[i];
        for (j = 0; j < heights[i] && y < screenHeight; ++j, ++y) {
            widget->data->getLine(widgets[i], j, buffer, maxWidth, &writtenLength, &isReversed);
            writtenLength = writtenLength <= maxWidth ? writtenLength : maxWidth;
            attrset(isReversed ? A_REVERSE : A_NORMAL);
            mvaddnstr(0, y, buffer, writtenLength);
        }
    }
    refresh();

    /************** Action **************/

    upperWidgetIndex = 0;

    free(buffer);
    free(heights);
    success:
    return TLOG_RESULT_SUCCESS;

    fail_widget_height:
    fail_buffer:
    fail_heights:
    fail_init:
    free(buffer);
    free(heights);
    return TLOG_RESULT_FAIL;
}
