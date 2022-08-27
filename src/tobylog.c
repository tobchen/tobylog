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
 * @return True if an action value was derived, or false else 
 */
static int getAction(int input, TLog_Widget_Action* action);

// TODO Document
static int getPrevFocusableWidget(void** widgets, size_t start, size_t* prev);

// TODO Document
static void getNextFocusableWidget(void** widgets, size_t widgetCount, size_t start, size_t* next);

// TODO Document
static void scrollUpToWidget(void** widgets, uint32_t* heights, uint32_t screenHeight,
        size_t* currentWidget, uint32_t* currentWidgetY, size_t targetWidget);

// TODO Document
static void scrollDownToWidget(void** widgets, uint32_t* heights, uint32_t screenHeight,
        size_t* currentWidget, uint32_t* currentWidgetY, size_t targetWidget);

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
    scrollok(stdscr, TRUE);

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
    size_t currentWidget;
    size_t nextWidget;
    uint32_t currentWidgetY; // in screen space
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
        } else if (heights[i] > screenHeight) {
            goto finished_cancel;
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

    /************** Find Focusable Widget **************/

    currentWidget = 0;
    currentWidgetY = 0;
    getNextFocusableWidget(widgets, widgetCount, currentWidget, &nextWidget);
    if (nextWidget < widgetCount) {
        scrollDownToWidget(widgets, heights, screenHeight, &currentWidget, &currentWidgetY, nextWidget);
        TLog_Widget* widget = (TLog_Widget*) widgets[currentWidget];
        widget->data->setFocus(widget, 1, &cursorX, &cursorY);
        move(currentWidgetY + cursorY, cursorX);
    }

    refresh();

    if (nextWidget >= widgetCount) {
        goto finished_success;
    }

    /************** Action **************/

    while (1) {
        TLog_Widget* widget = (TLog_Widget*) widgets[currentWidget];

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
        } else if (action == TLOG_WIDGET_ACTION_UP) {
            size_t prevWidget;
            if (currentWidget > 0 && getPrevFocusableWidget(widgets, currentWidget - 1, &prevWidget)) {
                scrollUpToWidget(widgets, heights, screenHeight, &currentWidget, &currentWidgetY, prevWidget);
                widget = (TLog_Widget*) widgets[currentWidget];
            }
            widget->data->setFocus(widget, 0, &cursorX, &cursorY);
            move(currentWidgetY + cursorY, cursorX);
        } else if (action == TLOG_WIDGET_ACTION_DOWN) {
            getNextFocusableWidget(widgets, widgetCount, currentWidget + 1, &nextWidget);
            if (nextWidget < widgetCount) {
                scrollDownToWidget(widgets, heights, screenHeight, &currentWidget, &currentWidgetY, nextWidget);
                widget = (TLog_Widget*) widgets[currentWidget];
            }
            widget->data->setFocus(widget, 1, &cursorX, &cursorY);
            move(currentWidgetY + cursorY, cursorX);
        }
        refresh();
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
    } else if (input == 0x1b) {
        *action = TLOG_WIDGET_ACTION_ESC;
    } else if (input == KEY_BACKSPACE) {
        *action = TLOG_WIDGET_ACTION_BACKSPACE;
    } else if (input == KEY_UP) {
        *action = TLOG_WIDGET_ACTION_UP;
    } else if (input == KEY_DOWN) {
        *action = TLOG_WIDGET_ACTION_DOWN;
    } else if (input == KEY_LEFT) {
        *action = TLOG_WIDGET_ACTION_LEFT;
    } else if (input == KEY_RIGHT) {
        *action = TLOG_WIDGET_ACTION_RIGHT;
    } else {
        return 0;
    }
    return 1;
}

static int getPrevFocusableWidget(void** widgets, size_t start, size_t* prev) {
    while (1) {
        if (((TLog_Widget*) widgets[start])->data->setFocus) {
            *prev = start;
            return 1;
        } else if (start == 0) {
            return 0;
        }
        --start;
    }
}

static void getNextFocusableWidget(void** widgets, size_t widgetCount, size_t start, size_t* next) {
    for (*next = start;
            *next < widgetCount && !((TLog_Widget*) widgets[*next])->data->setFocus;
            ++(*next));
}

static void scrollUpToWidget(void** widgets, uint32_t* heights, uint32_t screenHeight,
        size_t* currentWidget, uint32_t* currentWidgetY, size_t targetWidget) {
    while (*currentWidget > targetWidget) {
        --(*currentWidget);
        if (heights[*currentWidget] > *currentWidgetY) {
            int todo = heights[*currentWidget] - *currentWidgetY;
            scrl(-todo);
            *currentWidgetY = 0;
            drawLines((TLog_Widget*) widgets[*currentWidget], *currentWidgetY, 0, heights[*currentWidget], screenHeight);
        } else {
            *currentWidgetY -= heights[*currentWidget];
        }
    }
}

static void scrollDownToWidget(void** widgets, uint32_t* heights, uint32_t screenHeight,
        size_t* currentWidget, uint32_t* currentWidgetY, size_t targetWidget) {
    while (targetWidget > *currentWidget) {
        *currentWidgetY += heights[(*currentWidget)++];

        if (*currentWidgetY + heights[*currentWidget] > screenHeight) {
            uint32_t todo = *currentWidgetY + heights[*currentWidget] - screenHeight;
            scrl(todo);
            *currentWidgetY = screenHeight - heights[*currentWidget];
            drawLines((TLog_Widget*) widgets[*currentWidget], *currentWidgetY, 0, heights[*currentWidget], screenHeight);
        }
    }
}
