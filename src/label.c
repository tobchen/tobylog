/**
 * @file label.c
 * @author Tobias Heukäufer
 * @brief A label implementation.
 */

#include "../include/label.h"

#include <stdlib.h>

#include <apr_tables.h>
#include <apr_strings.h>

#include <ncurses.h>

#include "utf8.h"

/** @brief Initial capacity of a label's line meta buffer. */
#define INIT_LINE_CAPACITY 4

/* Thanks! https://stackoverflow.com/a/3599170 */
/** @brief Marks unused function parameters to prevent unused warnings. */
#define UNUSED(x) (void)(x)

/** @brief A label's line's start and ending. */
typedef struct tlog_label_line {
    /** @brief First character in line. */
    char* start;
    /** @brief The character after the last character in line. */
    char* end;
} TLog_Label_Line;

struct tlog_label {
    /** @brief Widget data. */
    const TLog_Widget_Data* data;

    /** @brief Memory pool */
    apr_pool_t* pool;

    /** @brief Text. */
    char* text;

    /** @brief Line starts and endings. */
    apr_array_header_t* lines;
};

static uint32_t getPreferedWidth(void* widget);
static uint32_t setMaximumWidth(void* widget, uint32_t maxWidth, uint32_t screenHeight);
static void drawLine(void* widget, uint32_t lineY);

/** @brief Label widget functions. */
static const TLog_Widget_Data TLOG_LABEL_DATA = {
    &getPreferedWidth,
    &setMaximumWidth,
    &drawLine,
    NULL,
    NULL,
    NULL
};

TLog_Label* TLog_Label_Create(apr_pool_t* pool, char* text) {
    if (!text) {
        goto fail;
    }

    TLog_Label* label = apr_palloc(pool, sizeof(TLog_Label));
    if (!label) {
        goto fail;
    }

    label->data = &TLOG_LABEL_DATA;

    label->pool = pool;

    label->text = apr_pstrdup(pool, text);
    if (!label->text) {
        goto fail;
    }

    label->lines = apr_array_make(pool, 0, sizeof(TLog_Label_Line));
    if (!label->lines) {
        goto fail;
    }

    return label;

    fail:
    return NULL;
}

static uint32_t getPreferedWidth(void* widget) {
    TLog_Label* label = (TLog_Label*) widget;
    
    uint32_t preferedWidth = 0;
    uint32_t currentWidth = 0;
    for (char* ch = label->text; *ch != 0; ch = TLog_UTF8_NextChar(ch)) {
        if (*ch == '\n') {
            currentWidth = 0;
        } else {
            /* Shall I one-line it? */
            ++currentWidth;
            if (currentWidth > preferedWidth) {
                preferedWidth = currentWidth;
            }
        }
    }

    return preferedWidth;
}

static uint32_t setMaximumWidth(void* widget, uint32_t maxWidth, uint32_t screenHeight) {
    TLog_Label* label = (TLog_Label*) widget;

    apr_array_clear(label->lines);

    /* TODO Sexy word wrap */
    size_t utf8width;
    TLog_Label_Line line;
    for (line.start = line.end = label->text, utf8width = 0;
            *line.end != 0;
            line.end = TLog_UTF8_NextChar(line.end), ++utf8width) {
        if (*line.end == '\n' || utf8width == maxWidth) {
            APR_ARRAY_PUSH(label->lines, TLog_Label_Line) = line;
            utf8width = 0;
            line.start = line.end = *line.end == '\n' ? TLog_UTF8_NextChar(line.end) : line.end;
        }
    }
    APR_ARRAY_PUSH(label->lines, TLog_Label_Line) = line;

    while (label->lines->nelts > (int) screenHeight) {
        apr_array_pop(label->lines);
    }

    return label->lines->nelts;
}

static void drawLine(void* widget, uint32_t lineY) {
    TLog_Label* label = (TLog_Label*) widget;
    TLog_Label_Line* line = &APR_ARRAY_IDX(label->lines, lineY, TLog_Label_Line);
    addnstr(line->start, line->end - line->start);
}
