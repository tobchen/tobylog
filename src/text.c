/**
 * @file text.c
 * @author Tobias Heukäufer
 * @brief A text field implementation.
 */

#include "../include/text.h"

#include <stdlib.h>
#include <string.h>

#include <ncurses.h>

/* Thanks! https://stackoverflow.com/a/3599170 */
/** @brief Marks unused function parameters to prevent unused warnings. */
#define UNUSED(x) (void)(x)

struct tlog_text {
    /** @brief Widget data. */
    const TLog_Widget_Data* data;

    /** @brief Width. */
    uint32_t width;

    /** @brief Text. */
    GString* text;
    /** @brief Index of the first visible character. */
    size_t firstVis;
    /** @brief Text length. */
    size_t utf8len;
    /** @brief Maximum text length. */
    size_t maxLen;

    /** @brief Wether to consume Return input (true) or not (false). */
    int consumeReturn;
};

static uint32_t getPreferedWidth(void* widget);
static uint32_t setMaximumWidth(void* widget, uint32_t maxWidth, uint32_t screenHeight);
static void drawLine(void* widget, uint32_t lineY);
static void setFocus(void* widget, int fromAbove, uint32_t* cursorX, uint32_t* cursorY);
static void putChar(void* widget, char ch,
        uint32_t* cursorX, uint32_t* cursorY, uint32_t* dirtyStart, uint32_t* dirtyEnd);
static int putAction(void* widget, TLog_Widget_Action action,
        uint32_t* cursorX, uint32_t* cursorY, uint32_t* dirtyStart, uint32_t* dirtyEnd);

static const TLog_Widget_Data TLOG_TEXT_DATA = {
    &getPreferedWidth,
    &setMaximumWidth,
    &drawLine,
    &setFocus,
    &putChar,
    &putAction
};

TLog_Text* TLog_Text_Create(size_t maximumWidth) {
    TLog_Text* text = malloc(sizeof(TLog_Text));
    if (!text) {
        goto fail_text;
    }

    text->data = &TLOG_TEXT_DATA;

    text->text = g_string_new(NULL);
    if (!text) {
        goto fail_text_text;
    }
    text->maxLen = maximumWidth;
    text->utf8len = 0;

    text->consumeReturn = 0;

    return text;

    fail_text_text:
    free(text);
    fail_text:
    return NULL;
}

void TLog_Text_Destroy(TLog_Text* text) {
    if (text) {
        g_string_free(text->text, TRUE);
        free(text);
    }
}

void TLog_Text_SetConsumeReturn(TLog_Text* text, int consumeReturn) {
    if (text) {
        text->consumeReturn = consumeReturn;
    }
}

void TLog_Text_SetText(TLog_Text* text, char* value) {
    if (text) {
        g_string_truncate(text->text, 0);
        text->utf8len = 0;

        for (char* next = g_utf8_find_next_char(value, NULL);
                *value != 0 && text->utf8len < text->maxLen;
                value = next, next = g_utf8_find_next_char(value, NULL), ++text->utf8len) {
            g_string_append_len(text->text, value, next - value);
        }
    }
}

char* TLog_Text_GetText(TLog_Text* text) {
    return text ? g_strdup(text->text->str) : NULL;
}

static uint32_t getPreferedWidth(void* widget) {
    return ((TLog_Text*) widget)->maxLen + 1;
}

static uint32_t setMaximumWidth(void* widget, uint32_t maxWidth, uint32_t screenHeight) {
    UNUSED(screenHeight);

    TLog_Text* text = (TLog_Text*) widget;

    text->width = maxWidth < text->maxLen + 1 ? maxWidth : text->maxLen + 1;

    char* ch = text->text->str;
    for (size_t i = 0; text->utf8len - i > text->width; ++i) {
        ch = g_utf8_find_next_char(ch, NULL);
    }
    text->firstVis = ch - text->text->str;

    return 1;
}

static void drawLine(void* widget, uint32_t lineY) {
    UNUSED(lineY);

    TLog_Text* text = (TLog_Text*) widget;

    attrset(A_REVERSE);

    addstr(&text->text->str[text->firstVis]);
    addch(' ');

    for (size_t done = text->utf8len + 1; done < text->width; ++done) {
        addch(' ');
    }
}

static void setFocus(void* widget, int fromAbove, uint32_t* cursorX, uint32_t* cursorY) {
    UNUSED(fromAbove);

    TLog_Text* text = (TLog_Text*) widget;
    *cursorX = text->utf8len < text->width ? text->utf8len : text->width - 1;
    *cursorY = 0;
}

static void putChar(void* widget, char ch,
        uint32_t* cursorX, uint32_t* cursorY, uint32_t* dirtyStart, uint32_t* dirtyEnd) {
    TLog_Text* text = (TLog_Text*) widget;

    *dirtyStart = *dirtyEnd = 0;

    if (text->utf8len < text->maxLen) {
        g_string_append_c(text->text, ch);
        ++text->utf8len;

        if (text->utf8len >= text->width) {
            text->firstVis += 
                    g_utf8_find_next_char(&text->text->str[text->firstVis], NULL) - &text->text->str[text->firstVis];
        }

        setFocus(text, 0, cursorX, cursorY);
        
        *dirtyEnd = 1;
    }
}
        
static int putAction(void* widget, TLog_Widget_Action action,
        uint32_t* cursorX, uint32_t* cursorY, uint32_t* dirtyStart, uint32_t* dirtyEnd) {
    TLog_Text* text = (TLog_Text*) widget;
    int consumed = 0;

    *dirtyStart = *dirtyEnd = 0;

    if (action == TLOG_WIDGET_ACTION_BACKSPACE) {
        if (text->utf8len > 0) {
            g_string_truncate(text->text, g_utf8_find_prev_char(text->text->str, &text->text->str[text->text->len])
                    - text->text->str);
            --text->utf8len;

            if (text->firstVis > 0) {
                text->firstVis = g_utf8_find_prev_char(text->text->str, &text->text->str[text->firstVis])
                        - text->text->str;
            }

            setFocus(text, 0, cursorX, cursorY);

            *dirtyEnd = 1;
        }
        consumed = 1;
    } else if (text->consumeReturn && action == TLOG_WIDGET_ACTION_RETURN) {
        consumed = 1;
    }

    return consumed;
}
