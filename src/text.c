/**
 * @file text.c
 * @author Tobias Heukäufer
 * @brief A text field implementation.
 */

#include "../include/text.h"

#include <stdlib.h>
#include <string.h>

#include <apr_strings.h>

#include <ncurses.h>

#include "string.h"
#include "utf8.h"

/* Thanks! https://stackoverflow.com/a/3599170 */
/** @brief Marks unused function parameters to prevent unused warnings. */
#define UNUSED(x) (void)(x)

struct tlog_text {
    /** @brief Widget data. */
    const TLog_Widget_Data* data;

    /** @brief Pool. */
    apr_pool_t* pool;

    /** @brief Width. */
    uint32_t width;

    /** @brief Text. */
    TLog_String text;
    /** @brief Index of the first visible character. */
    size_t firstVis;
    /** @brief Maximum text length. */
    size_t maxLen;

    /** @brief Wether to consume Return input (true) or not (false). */
    bool consumeReturn;
};

static uint32_t getPreferedWidth(TLog_Widget* widget);
static uint32_t setMaximumWidth(TLog_Widget* widget, uint32_t maxWidth, uint32_t screenHeight);
static void drawLine(TLog_Widget* widget, uint32_t lineY);
static void setFocus(TLog_Widget* widget, bool fromAbove, uint32_t* cursorX, uint32_t* cursorY);
static void putChar(TLog_Widget* widget, char ch,
        uint32_t* cursorX, uint32_t* cursorY, uint32_t* dirtyStart, uint32_t* dirtyEnd);
static bool putAction(TLog_Widget* widget, TLog_Widget_Action action,
        uint32_t* cursorX, uint32_t* cursorY, uint32_t* dirtyStart, uint32_t* dirtyEnd);

static const TLog_Widget_Data TLOG_TEXT_DATA = {
    &getPreferedWidth,
    &setMaximumWidth,
    &drawLine,
    &setFocus,
    &putChar,
    &putAction
};

TLog_Text* TLog_Text_Create(apr_pool_t* pool, size_t maximumWidth) {
    TLog_Text* text = apr_palloc(pool, sizeof(TLog_Text));
    if (!text) {
        goto fail;
    }

    text->data = &TLOG_TEXT_DATA;

    text->pool = pool;

    TLog_String_Init(&text->text, text->pool);
    text->maxLen = maximumWidth;

    text->consumeReturn = 0;

    return text;

    fail:
    return NULL;
}

void TLog_Text_SetConsumeReturn(TLog_Text* text, bool consumeReturn) {
    if (text) {
        text->consumeReturn = consumeReturn;
    }
}

void TLog_Text_SetText(TLog_Text* text, char* value) {
    if (text) {
        TLog_String_Set(&text->text, value);
    }
}

char* TLog_Text_GetText(TLog_Text* text, apr_pool_t* pool) {
    return text ? apr_pstrdup(pool, text->text.buffer) : NULL;
}

static uint32_t getPreferedWidth(TLog_Widget* widget) {
    return ((TLog_Text*) widget)->maxLen + 1;
}

static uint32_t setMaximumWidth(TLog_Widget* widget, uint32_t maxWidth, uint32_t screenHeight) {
    UNUSED(screenHeight);

    TLog_Text* text = (TLog_Text*) widget;

    text->width = maxWidth < text->maxLen + 1 ? maxWidth : text->maxLen + 1;

    char* ch = &text->text.buffer[text->text.len];
    for (size_t i = 0;
        i < text->width - 1 && ch != &text->text.buffer[0];
        ++i, ch = TLog_UTF8_PrevChar(ch));
    text->firstVis = ch - &text->text.buffer[0];

    return 1;
}

static void drawLine(TLog_Widget* widget, uint32_t lineY) {
    UNUSED(lineY);

    TLog_Text* text = (TLog_Text*) widget;

    attrset(A_REVERSE);

    addstr(&text->text.buffer[text->firstVis]);
    addch(' ');

    for (size_t done = text->text.utf8len + 1; done < text->width; ++done) {
        addch(' ');
    }
}

static void setFocus(TLog_Widget* widget, bool fromAbove, uint32_t* cursorX, uint32_t* cursorY) {
    UNUSED(fromAbove);

    TLog_Text* text = (TLog_Text*) widget;
    *cursorX = text->text.utf8len < text->width ? text->text.utf8len : text->width - 1;
    *cursorY = 0;
}

static void putChar(TLog_Widget* widget, char ch,
        uint32_t* cursorX, uint32_t* cursorY, uint32_t* dirtyStart, uint32_t* dirtyEnd) {
    TLog_Text* text = (TLog_Text*) widget;

    *dirtyStart = *dirtyEnd = 0;

    if (text->text.utf8len < text->maxLen) {
        TLog_String_AppendASCII(&text->text, ch);

        if (text->text.utf8len >= text->width) {
            text->firstVis += TLog_UTF8_CharLen(&text->text.buffer[text->firstVis]);
        }

        setFocus(widget, 0, cursorX, cursorY);
        
        *dirtyEnd = 1;
    }
}
        
static bool putAction(TLog_Widget* widget, TLog_Widget_Action action,
        uint32_t* cursorX, uint32_t* cursorY, uint32_t* dirtyStart, uint32_t* dirtyEnd) {
    TLog_Text* text = (TLog_Text*) widget;
    bool consumed = false;

    *dirtyStart = *dirtyEnd = 0;

    if (action == TLOG_WIDGET_ACTION_BACKSPACE) {
        if (text->text.utf8len > 0) {
            TLog_String_Pop(&text->text);

            if (text->firstVis > 0) {
                text->firstVis -= TLog_UTF8_CharLen(TLog_UTF8_PrevChar(&text->text.buffer[text->firstVis]));
            }

            setFocus(widget, 0, cursorX, cursorY);

            *dirtyEnd = 1;
        }
        consumed = true;
    } else if (text->consumeReturn && action == TLOG_WIDGET_ACTION_RETURN) {
        consumed = true;
    }

    return consumed;
}
