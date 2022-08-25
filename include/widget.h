/**
 * @file widget.h
 * @author Tobias Heukäufer
 * @brief General widget data.
 */

#ifndef TLOG_INCLUDE_WIDGET_H
#define TLOG_INCLUDE_WIDGET_H

#include <stdint.h>

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
typedef uint32_t (*TLog_Widget_GetPreferedWidth) (void* widget);

/**
 * @brief Sets a widget's maximum width.
 * 
 * In a Tobylog's run this function is the last chance to do widget preparation (e.g. memory allocations)
 * that might fail, as later widget calls won't check for errors, meaning from here on, widgets are expected
 * to run smoothly.
 * 
 * @param widget The widget to configure
 * @param maxWidth Maximum width
 * @param screenHeight Screen height
 * @return The widget's height, or 0 on error
 */
typedef uint32_t (*TLog_Widget_SetMaximumWidth) (void* widget, uint32_t maxWidth, uint32_t screenHeight);

/**
 * @brief Stores a widget's line in a buffer.
 * 
 * The buffer is guaranteed to be at least as wide as the maximum width set by @ref TLog_Widget_SetMaximumWidth.
 * 
 * @param widget The widget to query
 * @param lineY Widget's line to store
 * @param buffer Buffer to store the line
 * @param writtenLength Where to store the length written to the buffer
 * @param isReversed Where to store wether this line is to be displayed reversed (true) or not (false) 
 */
typedef void (*TLog_Widget_GetLine) (void* widget, uint32_t lineY, char* buffer,
        uint32_t* writtenLength, int* isReversed);

/**
 * @brief Notifies a widget of having focus.
 * 
 * @param widget The widget to notify
 * @param fromAbove Wether focus was received from above (true) or below (false)
 * @param cursorX Where to store the cursor X position in widget space
 * @param cursorY Where to store the cursor Y position in widget space
 */
typedef void (*TLog_Widget_SetFocus) (void* widget, int fromAbove, uint32_t* cursorX, uint32_t* cursorY);

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
typedef void (*TLog_Widget_PutChar) (void* widget, char ch,
        uint32_t* cursorX, uint32_t* cursorY, uint32_t* dirtyStart, uint32_t* dirtyEnd);

/**
 * @brief Sends an action to a widget.
 * 
 * @param widget The widget to send to
 * @param action Action to be sent
 * @param cursorX Where to store the cursor X position in widget space
 * @param cursorY Where to store the cursor Y position in widget space
 * @param dirtyStart Index of first dirty line
 * @param dirtyEnd Index after last dirty line
 * @return True if action was consumed, or false else
 */
typedef int (*TLog_Widget_PutAction) (void* widget, TLog_Widget_Action action,
        uint32_t* cursorX, uint32_t* cursorY, uint32_t* dirtyStart, uint32_t* dirtyEnd);

/** @brief Common widget data. */
typedef struct tlog_widget_data {
    /** @brief @copybrief TLog_Widget_GetPreferedWidth */
    TLog_Widget_GetPreferedWidth getPreferedWidth;
    /** @brief @copybrief TLog_Widget_SetMaximumWidth */
    TLog_Widget_SetMaximumWidth setMaximumWidth;
    /** @brief @copybrief TLog_Widget_GetLine */
    TLog_Widget_GetLine getLine;
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