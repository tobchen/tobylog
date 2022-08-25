#include "../include/tobylog.h"

#include <ncurses.h>

static uint32_t initCount = 0;

static int drawLine(TLog_Widget* widget, char* buffer,
        uint32_t widgetY, uint32_t fromY, uint32_t toY, uint32_t screenHeight);
static int getAction(int input, TLog_Widget_Action* action);

TLog_Result TLog_Init(void) {
    if (initCount > 0) {
        goto success;
    }

    if (!initscr()) {
        endwin();
        return TLOG_RESULT_FAIL;
    }

    cbreak();
    keypad(stdscr, TRUE);
    noecho();

    success:
    ++initCount;
    return TLOG_RESULT_SUCCESS;
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

TLog_Result TLog_Run(void** widgets, size_t widgetCount) {
    uint32_t screenWidth, screenHeight;
    char* buffer = NULL;
    size_t bufferLength = 0;
    uint32_t maxWidth;
    /* The index currently at line 0 */
    size_t currentWidgetIndex;
    uint32_t currentWidgetY;
    uint32_t cursorX, cursorY;

    if (initCount == 0) {
        goto fail_init;
    }

    if (widgetCount == 0 || !widgets) {
        goto immediate_success;
    }

    uint32_t* heights = malloc(sizeof(uint32_t) * widgetCount);
    if (!heights) {
        goto fail_heights;
    }

    /************** Widget Size Calculation **************/

    screenWidth = COLS;
    screenHeight = LINES;

    maxWidth = 0;
    for (size_t i = 0; i < widgetCount; ++i) {
        TLog_Widget* widget = (TLog_Widget*) widgets[i];

        uint32_t widgetWidth = widget->data->getPreferedWidth(widgets[i]);
        maxWidth = widgetWidth > maxWidth ? widgetWidth : maxWidth;
    }
    maxWidth = screenWidth - 1 < maxWidth ? screenWidth - 1 : maxWidth;

    for (size_t i = 0; i < widgetCount; ++i) {
        TLog_Widget* widget = (TLog_Widget*) widgets[i];

        heights[i] = widget->data->setMaximumWidth(widgets[i], maxWidth, screenHeight);
        if (heights[i] == 0) {
            goto fail_widget_height;
        }
    }

    if (maxWidth > bufferLength) {
        char* tmpBuffer = realloc(buffer, sizeof(char) * maxWidth);
        if (!tmpBuffer) {
            goto fail_buffer;
        }
        buffer = tmpBuffer;
        bufferLength = maxWidth;
    }

    /************** Initial Draw **************/

    attrset(A_NORMAL);
    clear();
    for (uint32_t i = 0, widgetY = 0; i < widgetCount; widgetY += heights[i], ++i) {
        if (drawLine((TLog_Widget*) widgets[i], buffer, widgetY, 0, heights[i], screenHeight)) {
            break;
        }
    }

    /************** Action **************/

    currentWidgetIndex = 0;
    currentWidgetY = 0;
    for (; currentWidgetIndex < widgetCount; currentWidgetY += heights[currentWidgetIndex], ++currentWidgetIndex) {
        TLog_Widget* widget = (TLog_Widget*) widgets[currentWidgetIndex];

        if (currentWidgetY >= screenHeight) {
            // TODO Scrolling
        }

        if (widget->data->setFocus) {
            widget->data->setFocus(widget, &cursorX, &cursorY);
            move(currentWidgetY + cursorY, cursorX);
            break;
        }
    }

    refresh();

    if (currentWidgetIndex >= widgetCount) {
        goto finished_success;
    }

    while (1) {
        TLog_Widget* widget = (TLog_Widget*) widgets[currentWidgetIndex];

        uint32_t dirtyStart = 0;
        uint32_t dirtyEnd = 0;

        int input = getch();
        TLog_Widget_Action action;
        if (input >= 32 && input <= 126 && widget->data->putChar) {
            widget->data->putChar(widget, (char) input, &cursorX, &cursorY, &dirtyStart, &dirtyEnd);
        } else if (getAction(input, &action) && 
                (!widget->data->putAction
                        || !widget->data->putAction(widget, action, &cursorX, &cursorY, &dirtyStart, &dirtyEnd))) {
            goto take_action;
        }

        drawLine(widget, buffer, currentWidgetY, dirtyStart, dirtyEnd, screenHeight);

        move(currentWidgetY + cursorY, cursorX);
        refresh();
        continue;

        take_action:
        if (action == TLOG_WIDGET_ACTION_ENTER) {
            goto finished_success;
        } else if (action == TLOG_WIDGET_ACTION_ESC) {
            goto finished_cancel;
        }
    }

    finished_success:
    free(buffer);
    free(heights);
    immediate_success:
    return TLOG_RESULT_SUCCESS;

    finished_cancel:
    free(buffer);
    free(heights);
    return TLOG_RESULT_CANCEL;

    fail_widget_height:
    fail_buffer:
    fail_heights:
    fail_init:
    free(buffer);
    free(heights);
    return TLOG_RESULT_FAIL;
}

static int drawLine(TLog_Widget* widget, char* buffer,
        uint32_t widgetY, uint32_t fromY, uint32_t toY, uint32_t screenHeight) {
    for (uint32_t y = fromY; y < toY; ++y) {
        uint32_t screenY = widgetY + y;

        if (screenY >= screenHeight) {
            return 1;
        }

        int isReversed;
        uint32_t writtenLength;
        widget->data->getLine(widget, y, buffer, &writtenLength, &isReversed);

        attrset(isReversed ? A_REVERSE : A_NORMAL);
        mvaddnstr(screenY, 0, buffer, writtenLength);
        attrset(A_NORMAL);
        clrtoeol();
    }

    return 0;
}

static int getAction(int input, TLog_Widget_Action* action) {
    if (input == '\n') {
        *action = TLOG_WIDGET_ACTION_ENTER;
    } else {
        return 0;
    }
    return 1;
}
