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
    guint32 width;

    /** @brief Text. */
    GString* text;
    /** @brief Index of the first visible character. */
    gsize firstVis;
    /** @brief Text length. */
    gsize utf8len;
    /** @brief Maximum text length. */
    gsize maxLen;

    /** @brief Wether to consume Return input (true) or not (false). */
    gboolean consumeReturn;
};

static guint32 getPreferedWidth(void* widget);
static guint32 setMaximumWidth(void* widget, guint32 maxWidth, guint32 screenHeight);
static void drawLine(void* widget, guint32 lineY);
static void setFocus(void* widget, gboolean fromAbove, guint32* cursorX, guint32* cursorY);
static void putChar(void* widget, gchar ch,
        guint32* cursorX, guint32* cursorY, guint32* dirtyStart, guint32* dirtyEnd);
static gboolean putAction(void* widget, TLog_Widget_Action action,
        guint32* cursorX, guint32* cursorY, guint32* dirtyStart, guint32* dirtyEnd);

static const TLog_Widget_Data TLOG_TEXT_DATA = {
    &getPreferedWidth,
    &setMaximumWidth,
    &drawLine,
    &setFocus,
    &putChar,
    &putAction
};

TLog_Text* TLog_Text_Create(gsize maximumWidth) {
    TLog_Text* text = g_malloc(sizeof(TLog_Text));
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
    g_free(text);
    fail_text:
    return NULL;
}

void TLog_Text_Destroy(TLog_Text* text) {
    if (text) {
        g_string_free(text->text, TRUE);
        g_free(text);
    }
}

void TLog_Text_SetConsumeReturn(TLog_Text* text, gboolean consumeReturn) {
    if (text) {
        text->consumeReturn = consumeReturn;
    }
}

void TLog_Text_SetText(TLog_Text* text, gchar* value) {
    if (text) {
        g_string_truncate(text->text, 0);
        text->utf8len = 0;

        if (value) {
            for (gchar* next = g_utf8_find_next_char(value, NULL);
                    *value != 0 && text->utf8len < text->maxLen;
                    value = next, next = g_utf8_find_next_char(value, NULL), ++text->utf8len) {
                g_string_append_len(text->text, value, next - value);
            }
        }
    }
}

gchar* TLog_Text_GetText(TLog_Text* text) {
    return text ? g_strdup(text->text->str) : NULL;
}

static guint32 getPreferedWidth(void* widget) {
    return ((TLog_Text*) widget)->maxLen + 1;
}

static guint32 setMaximumWidth(void* widget, guint32 maxWidth, guint32 screenHeight) {
    UNUSED(screenHeight);

    TLog_Text* text = (TLog_Text*) widget;

    text->width = maxWidth < text->maxLen + 1 ? maxWidth : text->maxLen + 1;

    gchar* ch = text->text->str;
    for (gsize i = 0; text->utf8len - i > text->width; ++i) {
        ch = g_utf8_find_next_char(ch, NULL);
    }
    text->firstVis = ch - text->text->str;

    return 1;
}

static void drawLine(void* widget, guint32 lineY) {
    UNUSED(lineY);

    TLog_Text* text = (TLog_Text*) widget;

    attrset(A_REVERSE);

    addstr(&text->text->str[text->firstVis]);
    addch(' ');

    for (gsize done = text->utf8len + 1; done < text->width; ++done) {
        addch(' ');
    }
}

static void setFocus(void* widget, gboolean fromAbove, guint32* cursorX, guint32* cursorY) {
    UNUSED(fromAbove);

    TLog_Text* text = (TLog_Text*) widget;
    *cursorX = text->utf8len < text->width ? text->utf8len : text->width - 1;
    *cursorY = 0;
}

static void putChar(void* widget, gchar ch,
        guint32* cursorX, guint32* cursorY, guint32* dirtyStart, guint32* dirtyEnd) {
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
        
static gboolean putAction(void* widget, TLog_Widget_Action action,
        guint32* cursorX, guint32* cursorY, guint32* dirtyStart, guint32* dirtyEnd) {
    TLog_Text* text = (TLog_Text*) widget;
    gboolean consumed = FALSE;

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
        consumed = TRUE;
    } else if (text->consumeReturn && action == TLOG_WIDGET_ACTION_RETURN) {
        consumed = TRUE;
    }

    return consumed;
}
