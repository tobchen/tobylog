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

#define DEFAULT_WIDGET_COUNT 12

/** @brief TRUE if Tobylog is initialized, or FALSE else. */
static bool isInitialized = false;

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
static bool getPrevFocusableWidget(TLog_Widget** widgets, TLog_Widget** start, TLog_Widget*** prev);

// TODO Document
static void getNextFocusableWidget(TLog_Widget** start, TLog_Widget*** next);

// TODO Document
static void scrollUpToWidget(TLog_Widget** widgets, uint32_t screenHeight,
        TLog_Widget*** currentWidget, uint32_t* currentWidgetY, TLog_Widget** targetWidget);

// TODO Document
static void scrollDownToWidget(TLog_Widget** widgets, uint32_t screenHeight,
        TLog_Widget*** currentWidget, uint32_t* currentWidgetY, TLog_Widget** targetWidget);

TLog_Result TLog_Init(apr_pool_t* pool) {
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

    heights = apr_array_make(pool, DEFAULT_WIDGET_COUNT, sizeof(uint32_t));
    if (!heights) {
        goto fail;
    }

    isInitialized = true;
    apr_pool_cleanup_register(pool, NULL, terminate, apr_pool_cleanup_null);
    success:
    return TLOG_RESULT_OK;

    fail:
    return TLOG_RESULT_FAIL;
}

TLog_Result TLog_Run(TLog_Widget** widgets) {
    uint32_t screenWidth, screenHeight;
    uint32_t maxWidth;
    TLog_Widget** currentWidget;
    TLog_Widget** nextWidget;
    uint32_t currentWidgetY; // in screen space
    uint32_t cursorX, cursorY;

    if (!isInitialized) {
        goto fail;
    }

    if (!widgets) {
        goto immediate_success;
    }

    /************** Widget Size Calculation **************/

    screenWidth = COLS;
    screenHeight = LINES;

    maxWidth = 0;
    for (TLog_Widget** iter = widgets; *iter; ++iter) {
        uint32_t widgetWidth = (*iter)->data->getPreferedWidth(*iter);
        maxWidth = widgetWidth > maxWidth ? widgetWidth : maxWidth;
    }
    maxWidth = screenWidth - 1 < maxWidth ? screenWidth - 1 : maxWidth;

    apr_array_clear(heights);
    for (TLog_Widget** iter = widgets; *iter; ++iter) {
        uint32_t height = (*iter)->data->setMaximumWidth(*iter, maxWidth, screenHeight);
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
    for (currentWidget = widgets, currentWidgetY = 0; *currentWidget; ++currentWidget) {
        uint32_t height = APR_ARRAY_IDX(heights, currentWidget - widgets, uint32_t);
        if (!drawLines(*currentWidget, currentWidgetY, 0, height, screenHeight)) {
            break;
        }
        currentWidgetY += height;
    }

    /************** Find Focusable Widget **************/

    currentWidget = widgets;
    currentWidgetY = 0;
    getNextFocusableWidget(currentWidget, &nextWidget);
    if (nextWidget) {
        scrollDownToWidget(widgets, screenHeight, &currentWidget, &currentWidgetY, nextWidget);
        (*currentWidget)->data->setFocus(*currentWidget, 1, &cursorX, &cursorY);
        move(currentWidgetY + cursorY, cursorX);
    }

    refresh();

    if (!nextWidget) {
        goto finished_success;
    }

    /************** Action **************/

    while (true) {
        uint32_t dirtyStart = 0;
        uint32_t dirtyEnd = 0;

        int input = getch();
        TLog_Widget_Action action;
        if (input >= 32 && input <= 126 && (*currentWidget)->data->putChar) {
            (*currentWidget)->data->putChar(*currentWidget, (char) input, &cursorX, &cursorY, &dirtyStart, &dirtyEnd);
        } else if (getAction(input, &action) && 
                (!(*currentWidget)->data->putAction
                        || !(*currentWidget)->data->putAction(*currentWidget, action, &cursorX, &cursorY, &dirtyStart, &dirtyEnd))) {
            goto take_action;
        }

        drawLines(*currentWidget, currentWidgetY, dirtyStart, dirtyEnd, screenHeight);

        move(currentWidgetY + cursorY, cursorX);
        refresh();
        continue;

        take_action:
        if (action == TLOG_WIDGET_ACTION_RETURN) {
            goto finished_success;
        } else if (action == TLOG_WIDGET_ACTION_ESC) {
            goto finished_cancel;
        } else if (action == TLOG_WIDGET_ACTION_UP) {
            TLog_Widget** prevWidget;
            if (currentWidget > widgets && getPrevFocusableWidget(widgets, currentWidget - 1, &prevWidget)) {
                scrollUpToWidget(widgets, screenHeight,
                        &currentWidget, &currentWidgetY, prevWidget);
            }
            (*currentWidget)->data->setFocus(*currentWidget, 0, &cursorX, &cursorY);
            move(currentWidgetY + cursorY, cursorX);
        } else if (action == TLOG_WIDGET_ACTION_DOWN) {
            getNextFocusableWidget(currentWidget + 1, &nextWidget);
            if (nextWidget) {
                scrollDownToWidget(widgets, screenHeight,
                        &currentWidget, &currentWidgetY, nextWidget);
            }
            (*currentWidget)->data->setFocus(*currentWidget, 1, &cursorX, &cursorY);
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

static bool getPrevFocusableWidget(TLog_Widget** widgets, TLog_Widget** start, TLog_Widget*** prev) {
    while (1) {
        if ((*start)->data->setFocus) {
            *prev = start;
            return true;
        } else if (start == widgets) {
            return false;
        }
        --start;
    }
}

static void getNextFocusableWidget(TLog_Widget** start, TLog_Widget*** next) {
    for (*next = start;
            **next && !(**next)->data->setFocus;
            ++(*next));
}

static void scrollUpToWidget(TLog_Widget** widgets, uint32_t screenHeight,
        TLog_Widget*** currentWidget, uint32_t* currentWidgetY, TLog_Widget** targetWidget) {
    while (*currentWidget > targetWidget) {
        --(*currentWidget);
        uint32_t height = APR_ARRAY_IDX(heights, *currentWidget - widgets, uint32_t);

        if (height > *currentWidgetY) {
            int todo = height - *currentWidgetY;
            scrl(-todo);
            *currentWidgetY = 0;
            drawLines(**currentWidget, *currentWidgetY, 0, height, screenHeight);
        } else {
            *currentWidgetY -= height;
        }
    }
}

static void scrollDownToWidget(TLog_Widget** widgets, uint32_t screenHeight,
        TLog_Widget*** currentWidget, uint32_t* currentWidgetY, TLog_Widget** targetWidget) {
    while (targetWidget > *currentWidget) {
        *currentWidgetY += APR_ARRAY_IDX(heights, *currentWidget - widgets, uint32_t);
        ++(*currentWidget);
        uint32_t height = APR_ARRAY_IDX(heights, *currentWidget - widgets, uint32_t);

        if (*currentWidgetY + height > screenHeight) {
            uint32_t todo = *currentWidgetY + height - screenHeight;
            scrl(todo);
            *currentWidgetY = screenHeight - height;
            drawLines(**currentWidget, *currentWidgetY, 0, height, screenHeight);
        }
    }
}
