/**
 * @file tobylog.c
 * @author Tobias Heukäufer
 * @brief Implementation of general Tobylog management.
 */

#include "../include/tobylog.h"

#include <ncurses.h>
#include <string.h>

/** @brief Number of times @ref TLog_Init() was called. */
static uint32_t initCount = 0;

/**
 * @brief Draws some of widget's lines.
 * 
 * @param widget The widget whose lines to draw
 * @param buffer The buffer to be used to temporarily store each line
 * @param widgetY The widget's Y position in screen space
 * @param fromY The index of the first line to draw
 * @param toY The index after the last line to draw
 * @param screenHeight The screen's height
 * @return 0 if all lines fit the screen, or 1 else
 */
static int drawLines(TLog_Widget* widget, uint32_t widgetY, uint32_t fromY, uint32_t toY, uint32_t screenHeight);

/**
 * @brief Derives an action value from an ncurses input.
 * 
 * @param input ncurses input
 * @param action Where to put the action value
 * @return 1 if an action value was derived, or 0 else 
 */
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
    return TLOG_RESULT_OK;
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

    /************** Initial Draw **************/

    attrset(A_NORMAL);
    clear();
    for (uint32_t i = 0, widgetY = 0; i < widgetCount; widgetY += heights[i], ++i) {
        if (drawLines((TLog_Widget*) widgets[i], widgetY, 0, heights[i], screenHeight)) {
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
            widget->data->setFocus(widget, 1, &cursorX, &cursorY);
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

        drawLines(widget, currentWidgetY, dirtyStart, dirtyEnd, screenHeight);

        move(currentWidgetY + cursorY, cursorX);
        refresh();
        continue;

        take_action:
        if (action == TLOG_WIDGET_ACTION_RETURN) {
            goto finished_success;
        } else if (action == TLOG_WIDGET_ACTION_ESC) {
            goto finished_cancel;
        }
    }

    finished_success:
    free(heights);
    immediate_success:
    return TLOG_RESULT_OK;

    finished_cancel:
    free(heights);
    return TLOG_RESULT_CANCEL;

    fail_widget_height:
    fail_heights:
    fail_init:
    free(heights);
    return TLOG_RESULT_FAIL;
}

static int drawLines(TLog_Widget* widget, uint32_t widgetY, uint32_t fromY, uint32_t toY, uint32_t screenHeight) {
    for (uint32_t y = fromY; y < toY; ++y) {
        uint32_t screenY = widgetY + y;

        if (screenY >= screenHeight) {
            return 1;
        }

        attrset(A_NORMAL);
        move(screenY, 0);
        widget->data->drawLine(widget, y);

        attrset(A_NORMAL);
        clrtoeol();
    }

    return 0;
}

static int getAction(int input, TLog_Widget_Action* action) {
    if (input == '\n') {
        *action = TLOG_WIDGET_ACTION_RETURN;
    } else if (input == KEY_BACKSPACE) {
        *action = TLOG_WIDGET_ACTION_BACKSPACE;
    } else {
        return 0;
    }
    return 1;
}
