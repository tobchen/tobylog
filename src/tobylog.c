/**
 * @file tobylog.c
 * @author Tobias Heukäufer
 * @brief Implementation of general Tobylog management.
 */

#include "../include/tobylog.h"

#include <ncurses.h>
#include <string.h>

/** @brief Number of times @ref TLog_Init() was called. */
static guint32 initCount = 0;

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
static gboolean drawLines(TLog_Widget* widget, guint32 widgetY, guint32 fromY, guint32 toY, guint32 screenHeight);

/**
 * @brief Derives an action value from an ncurses input.
 * 
 * @param input ncurses input
 * @param action Where to put the action value
 * @return TRUE if an action value was derived, or FALSE else 
 */
static gboolean getAction(int input, TLog_Widget_Action* action);

// TODO Document
static gboolean getPrevFocusableWidget(void** widgets, gsize start, gsize* prev);

// TODO Document
static void getNextFocusableWidget(void** widgets, gsize widgetCount, gsize start, gsize* next);

// TODO Document
static void scrollUpToWidget(void** widgets, guint32* heights, guint32 screenHeight,
        gsize* currentWidget, guint32* currentWidgetY, gsize targetWidget);

// TODO Document
static void scrollDownToWidget(void** widgets, guint32* heights, guint32 screenHeight,
        gsize* currentWidget, guint32* currentWidgetY, gsize targetWidget);

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

TLog_Result TLog_Run(void** widgets, gsize widgetCount) {
    guint32 screenWidth, screenHeight;
    guint32 maxWidth;
    gsize currentWidget;
    gsize nextWidget;
    guint32 currentWidgetY; // in screen space
    guint32 cursorX, cursorY;

    if (initCount == 0) {
        goto fail_init;
    }

    if (widgetCount == 0 || !widgets) {
        goto immediate_success;
    }

    guint32* heights = g_malloc(sizeof(guint32) * widgetCount);
    if (!heights) {
        goto fail_heights;
    }

    /************** Widget Size Calculation **************/

    screenWidth = COLS;
    screenHeight = LINES;

    maxWidth = 0;
    for (gsize i = 0; i < widgetCount; ++i) {
        TLog_Widget* widget = (TLog_Widget*) widgets[i];

        guint32 widgetWidth = widget->data->getPreferedWidth(widgets[i]);
        maxWidth = widgetWidth > maxWidth ? widgetWidth : maxWidth;
    }
    maxWidth = screenWidth - 1 < maxWidth ? screenWidth - 1 : maxWidth;

    for (gsize i = 0; i < widgetCount; ++i) {
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
    for (guint32 i = 0, widgetY = 0; i < widgetCount; widgetY += heights[i], ++i) {
        if (!drawLines((TLog_Widget*) widgets[i], widgetY, 0, heights[i], screenHeight)) {
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

    while (TRUE) {
        TLog_Widget* widget = (TLog_Widget*) widgets[currentWidget];

        guint32 dirtyStart = 0;
        guint32 dirtyEnd = 0;

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
            gsize prevWidget;
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
    g_free(heights);
    immediate_success:
    return TLOG_RESULT_OK;

    finished_cancel:
    g_free(heights);
    return TLOG_RESULT_CANCEL;

    fail_widget_height:
    fail_heights:
    fail_init:
    g_free(heights);
    return TLOG_RESULT_FAIL;
}

static gboolean drawLines(TLog_Widget* widget, guint32 widgetY, guint32 fromY, guint32 toY, guint32 screenHeight) {
    for (guint32 y = fromY; y < toY; ++y) {
        guint32 screenY = widgetY + y;

        if (screenY >= screenHeight) {
            return FALSE;
        }

        attrset(A_NORMAL);
        move(screenY, 0);
        widget->data->drawLine(widget, y);

        attrset(A_NORMAL);
        clrtoeol();
    }

    return TRUE;
}

static gboolean getAction(int input, TLog_Widget_Action* action) {
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
        return FALSE;
    }
    return TRUE;
}

static gboolean getPrevFocusableWidget(void** widgets, gsize start, gsize* prev) {
    while (1) {
        if (((TLog_Widget*) widgets[start])->data->setFocus) {
            *prev = start;
            return TRUE;
        } else if (start == 0) {
            return FALSE;
        }
        --start;
    }
}

static void getNextFocusableWidget(void** widgets, gsize widgetCount, gsize start, gsize* next) {
    for (*next = start;
            *next < widgetCount && !((TLog_Widget*) widgets[*next])->data->setFocus;
            ++(*next));
}

static void scrollUpToWidget(void** widgets, guint32* heights, guint32 screenHeight,
        gsize* currentWidget, guint32* currentWidgetY, gsize targetWidget) {
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

static void scrollDownToWidget(void** widgets, guint32* heights, guint32 screenHeight,
        gsize* currentWidget, guint32* currentWidgetY, gsize targetWidget) {
    while (targetWidget > *currentWidget) {
        *currentWidgetY += heights[(*currentWidget)++];

        if (*currentWidgetY + heights[*currentWidget] > screenHeight) {
            guint32 todo = *currentWidgetY + heights[*currentWidget] - screenHeight;
            scrl(todo);
            *currentWidgetY = screenHeight - heights[*currentWidget];
            drawLines((TLog_Widget*) widgets[*currentWidget], *currentWidgetY, 0, heights[*currentWidget], screenHeight);
        }
    }
}
