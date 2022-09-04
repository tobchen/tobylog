/**
 * @file tobylog.c
 * @author Tobias Heukäufer
 * @brief Implementation of general Tobylog management.
 */

#include "../include/tobylog.h"

#include <ncurses.h>
#include <string.h>

#include <apr_tables.h>

/* Thanks! https://stackoverflow.com/a/3599170 */
/** @brief Marks unused function parameters to prevent unused warnings. */
#define UNUSED(x) (void)(x)

/** @brief TRUE if Tobylog is initialized, or FALSE else. */
static bool isInitialized = false;

/** @brief Memory pool. */
static apr_pool_t* pool = NULL;

/** @brief Widget heights. */
static apr_array_header_t* heights = NULL;

/**
 * @brief Terminates Tobylog.
 * 
 * @param data Dummy
 * @return Always APR_SUCCESS 
 */
static apr_status_t terminate(void* data);

/**
 * @brief Draws some of widget's lines.
 * 
 * @param widget The widget whose lines to draw
 * @param buffer The buffer to be used to temporarily store each line
 * @param widgetY The widget's Y position in screen space
 * @param fromY The index of the first line to draw
 * @param toY The index after the last line to draw
 * @param screenHeight The screen's height
 * @return TRUE if all lines fit the screen, or FALSE else
 */
static bool drawLines(TLog_Widget* widget, uint32_t widgetY, uint32_t fromY, uint32_t toY, uint32_t screenHeight);

/**
 * @brief Derives an action value from an ncurses input.
 * 
 * @param input ncurses input
 * @param action Where to put the action value
 * @return TRUE if an action value was derived, or FALSE else 
 */
static bool getAction(int input, TLog_Widget_Action* action);

// TODO Document
static bool getPrevFocusableWidget(void** widgets, size_t start, size_t* prev);

// TODO Document
static void getNextFocusableWidget(void** widgets, size_t widgetCount, size_t start, size_t* next);

// TODO Document
static void scrollUpToWidget(void** widgets, uint32_t* heights, uint32_t screenHeight,
        size_t* currentWidget, uint32_t* currentWidgetY, size_t targetWidget);

// TODO Document
static void scrollDownToWidget(void** widgets, uint32_t* heights, uint32_t screenHeight,
        size_t* currentWidget, uint32_t* currentWidgetY, size_t targetWidget);

TLog_Result TLog_Init(apr_pool_t* poolArg) {
    if (isInitialized) {
        goto success;
    }

    if (!initscr()) {
        goto fail;
    }

    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    scrollok(stdscr, TRUE);

    pool = poolArg;
    isInitialized = true;
    apr_pool_cleanup_register(pool, NULL, terminate, apr_pool_cleanup_null);
    success:
    return TLOG_RESULT_OK;

    fail:
    return TLOG_RESULT_FAIL;
}

TLog_Result TLog_Run(void** widgets, size_t widgetCount) {
    uint32_t screenWidth, screenHeight;
    uint32_t maxWidth;
    size_t currentWidget;
    size_t nextWidget;
    uint32_t currentWidgetY; // in screen space
    uint32_t cursorX, cursorY;

    if (!isInitialized) {
        goto fail;
    }

    if (widgetCount == 0 || !widgets) {
        goto immediate_success;
    }

    if (!heights) {
        heights = apr_array_make(pool, widgetCount, sizeof(uint32_t));
        if (!heights) {
            goto fail;
        }
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

    apr_array_clear(heights);
    for (size_t i = 0; i < widgetCount; ++i) {
        TLog_Widget* widget = (TLog_Widget*) widgets[i];

        size_t height = widget->data->setMaximumWidth(widgets[i], maxWidth, screenHeight);
        if (height == 0) {
            goto fail;
        } else if (height > screenHeight) {
            goto finished_cancel;
        }

        APR_ARRAY_PUSH(heights, uint32_t) = height;
    }

    /************** Initial Draw **************/

    attrset(A_NORMAL);
    clear();
    for (uint32_t i = 0, widgetY = 0; i < widgetCount; widgetY += APR_ARRAY_IDX(heights, i, uint32_t), ++i) {
        if (!drawLines((TLog_Widget*) widgets[i], widgetY, 0, APR_ARRAY_IDX(heights, i, uint32_t), screenHeight)) {
            break;
        }
    }

    /************** Find Focusable Widget **************/

    currentWidget = 0;
    currentWidgetY = 0;
    getNextFocusableWidget(widgets, widgetCount, currentWidget, &nextWidget);
    if (nextWidget < widgetCount) {
        scrollDownToWidget(widgets, (uint32_t*) heights->elts, screenHeight, &currentWidget, &currentWidgetY, nextWidget);
        TLog_Widget* widget = (TLog_Widget*) widgets[currentWidget];
        widget->data->setFocus(widget, 1, &cursorX, &cursorY);
        move(currentWidgetY + cursorY, cursorX);
    }

    refresh();

    if (nextWidget >= widgetCount) {
        goto finished_success;
    }

    /************** Action **************/

    while (true) {
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
                scrollUpToWidget(widgets, (uint32_t*) heights->elts, screenHeight,
                        &currentWidget, &currentWidgetY, prevWidget);
                widget = (TLog_Widget*) widgets[currentWidget];
            }
            widget->data->setFocus(widget, 0, &cursorX, &cursorY);
            move(currentWidgetY + cursorY, cursorX);
        } else if (action == TLOG_WIDGET_ACTION_DOWN) {
            getNextFocusableWidget(widgets, widgetCount, currentWidget + 1, &nextWidget);
            if (nextWidget < widgetCount) {
                scrollDownToWidget(widgets, (uint32_t*) heights->elts, screenHeight,
                        &currentWidget, &currentWidgetY, nextWidget);
                widget = (TLog_Widget*) widgets[currentWidget];
            }
            widget->data->setFocus(widget, 1, &cursorX, &cursorY);
            move(currentWidgetY + cursorY, cursorX);
        }
        refresh();
    }

    finished_success:
    immediate_success:
    return TLOG_RESULT_OK;

    finished_cancel:
    return TLOG_RESULT_CANCEL;

    fail:
    return TLOG_RESULT_FAIL;
}

static apr_status_t terminate(void* data) {
    UNUSED(data);
    isInitialized = false;
    pool = NULL;
    heights = NULL;
    endwin();
    return APR_SUCCESS;
}

static bool drawLines(TLog_Widget* widget, uint32_t widgetY, uint32_t fromY, uint32_t toY, uint32_t screenHeight) {
    for (uint32_t y = fromY; y < toY; ++y) {
        uint32_t screenY = widgetY + y;

        if (screenY >= screenHeight) {
            return false;
        }

        attrset(A_NORMAL);
        move(screenY, 0);
        widget->data->drawLine(widget, y);

        attrset(A_NORMAL);
        clrtoeol();
    }

    return true;
}

static bool getAction(int input, TLog_Widget_Action* action) {
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
        return false;
    }
    return true;
}

static bool getPrevFocusableWidget(void** widgets, size_t start, size_t* prev) {
    while (1) {
        if (((TLog_Widget*) widgets[start])->data->setFocus) {
            *prev = start;
            return true;
        } else if (start == 0) {
            return false;
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
