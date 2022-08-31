/**
 * @file widget.h
 * @author Tobias Heukäufer
 * @brief General widget data.
 */

#ifndef TLOG_INCLUDE_WIDGET_H
#define TLOG_INCLUDE_WIDGET_H

#include <glib.h>

/** @brief Action values. */
typedef enum tlog_widget_action {
    /** @brief Return key */
    TLOG_WIDGET_ACTION_RETURN,
    /** @brief Escape key */
    TLOG_WIDGET_ACTION_ESC,
    /** @brief Backspace key */
    TLOG_WIDGET_ACTION_BACKSPACE,
    /** @brief Arrow up key */
    TLOG_WIDGET_ACTION_UP,
    /** @brief Arrow down key */
    TLOG_WIDGET_ACTION_DOWN,
    /** @brief Arrow left key */
    TLOG_WIDGET_ACTION_LEFT,
    /** @brief Arrow right key */
    TLOG_WIDGET_ACTION_RIGHT,
    /** @brief Tab key */
    TLOG_WIDGET_ACTION_TAB
} TLog_Widget_Action;

/**
 * @brief Returns a widget's prefered width.
 * 
 * @param widget The widget to query
 * @return The widget's prefered width, or 0 on error
 */
typedef guint32 (*TLog_Widget_GetPreferedWidth) (void* widget);

/**
 * @brief Sets a widget's maximum width.
 * 
 * In a Tobylog's run this function is the last chance to do widget preparation (e.g. memory allocations)
 * that might fail, as later widget calls won't check for errors, meaning from here on, widgets are expected
 * to run smoothly.
 * 
 * If the returned height is greater than the given screen height, this run will end as cancelled,
 * as Tobylog can currently not handle widgets larger than the screen.
 * 
 * @param widget The widget to configure
 * @param maxWidth Maximum width
 * @param screenHeight Screen height
 * @return The widget's height, or 0 on error
 */
typedef guint32 (*TLog_Widget_SetMaximumWidth) (void* widget, guint32 maxWidth, guint32 screenHeight);

/**
 * @brief Draws a widget's line to the screen.
 * 
 * When called, the cursor is at the position the layout manager expects the widget to draw its line at,
 * and the terminal's attributes are set to normal.
 * Drawing longer lines than was previously set by @ref TLog_Widget_SetMaximumWidth may lead to
 * display errors.
 * 
 * @param widget The widget to query
 * @param lineY Widget's line to store 
 */
typedef void (*TLog_Widget_DrawLine) (void* widget, guint32 lineY);

/**
 * @brief Notifies a widget of having focus.
 * 
 * @param widget The widget to notify
 * @param fromAbove Wether focus was received from above (true) or below (false)
 * @param cursorX Where to store the cursor X position in widget space
 * @param cursorY Where to store the cursor Y position in widget space
 */
typedef void (*TLog_Widget_SetFocus) (void* widget, gboolean fromAbove, guint32* cursorX, guint32* cursorY);

/**
 * @brief Sends a character to a widget.
 * 
 * @param widget The widget to send to
 * @param ch Character to be sent
 * @param cursorX Where to store the cursor X position in widget space
 * @param cursorY Where to store the cursor Y position in widget space
 * @param dirtyStart Index of first dirty line
 * @param dirtyEnd Index after last dirty line
 */
typedef void (*TLog_Widget_PutChar) (void* widget, gchar ch,
        guint32* cursorX, guint32* cursorY, guint32* dirtyStart, guint32* dirtyEnd);

/**
 * @brief Sends an action to a widget.
 * 
 * @param widget The widget to send to
 * @param action Action to be sent
 * @param cursorX Where to store the cursor X position in widget space
 * @param cursorY Where to store the cursor Y position in widget space
 * @param dirtyStart Index of first dirty line
 * @param dirtyEnd Index after last dirty line
 * @return TRUE if action was consumed, or FALSE else
 */
typedef gboolean (*TLog_Widget_PutAction) (void* widget, TLog_Widget_Action action,
        guint32* cursorX, guint32* cursorY, guint32* dirtyStart, guint32* dirtyEnd);

/** @brief Common widget data. */
typedef struct tlog_widget_data {
    /** @brief @copybrief TLog_Widget_GetPreferedWidth */
    TLog_Widget_GetPreferedWidth getPreferedWidth;
    /** @brief @copybrief TLog_Widget_SetMaximumWidth */
    TLog_Widget_SetMaximumWidth setMaximumWidth;
    /** @brief @copybrief TLog_Widget_DrawLine */
    TLog_Widget_DrawLine drawLine;
    /**
     * @brief @copybrief TLog_Widget_SetFocus
     * 
     * Set NULL if unfocusable.
     */
    TLog_Widget_SetFocus setFocus;
    /**
     * @brief @copybrief TLog_Widget_PutChar
     * 
     * Set NULL if not taking characters.
     */
    TLog_Widget_PutChar putChar;
    /**
     * @brief @copybrief TLog_Widget_PutAction
     *
     * Set NULL if not taking action values. 
     */
    TLog_Widget_PutAction putAction;
} TLog_Widget_Data;


/** @brief General widget. */
typedef struct tlog_widget {
    /** @brief @copybrief TLog_Widget_Data */
    TLog_Widget_Data* data;
} TLog_Widget;

#endif