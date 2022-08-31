/**
 * @file label.c
 * @author Tobias Heukäufer
 * @brief A label implementation.
 */

#include "../include/label.h"

#include <stdlib.h>

#include <ncurses.h>

/** @brief Initial capacity of a label's line meta buffer. */
#define INIT_LINE_CAPACITY 4

/* Thanks! https://stackoverflow.com/a/3599170 */
/** @brief Marks unused function parameters to prevent unused warnings. */
#define UNUSED(x) (void)(x)

/** @brief A label's line's start and ending. */
typedef struct tlog_label_line {
    /** @brief First character in line. */
    gchar* start;
    /** @brief The character after the last character in line. */
    gchar* end;
} TLog_Label_Line;

struct tlog_label {
    /** @brief Widget data. */
    const TLog_Widget_Data* data;

    /** @brief Text. */
    gchar* text;

    /** @brief Line starts and endings. */
    GArray* lines;
};

static guint32 getPreferedWidth(void* widget);
static guint32 setMaximumWidth(void* widget, guint32 maxWidth, guint32 screenHeight);
static void drawLine(void* widget, guint32 lineY);

/** @brief Label widget functions. */
static const TLog_Widget_Data TLOG_LABEL_DATA = {
    &getPreferedWidth,
    &setMaximumWidth,
    &drawLine,
    NULL,
    NULL,
    NULL
};

TLog_Label* TLog_Label_Create(gchar* text) {
    if (!text) {
        goto fail_arg;
    }

    TLog_Label* label = g_malloc(sizeof(TLog_Label));
    if (!label) {
        goto fail_label;
    }

    label->data = &TLOG_LABEL_DATA;

    label->text = g_strdup(text);
    if (!label->text) {
        goto fail_text;
    }

    label->lines = g_array_new(FALSE, FALSE, sizeof(TLog_Label_Line));
    if (!label->lines) {
        goto fail_lines;
    }

    return label;

    fail_lines:
    g_free(label->text);
    fail_text:
    g_free(label);
    fail_label:
    fail_arg:
    return NULL;
}

void TLog_Label_Destroy(TLog_Label* label) {
    if (label) {
        g_array_free(label->lines, TRUE);
        g_free(label->text);
        g_free(label);
    }
}

static guint32 getPreferedWidth(void* widget) {
    TLog_Label* label = (TLog_Label*) widget;
    
    guint32 preferedWidth = 0;
    guint32 currentWidth = 0;
    for (gchar* ch = label->text; *ch != 0; ch = g_utf8_find_next_char(ch, NULL)) {
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

static guint32 setMaximumWidth(void* widget, guint32 maxWidth, guint32 screenHeight) {
    UNUSED(screenHeight);

    TLog_Label* label = (TLog_Label*) widget;

    g_array_remove_range(label->lines, 0, label->lines->len);

    /* TODO Sexy word wrap */
    gsize utf8width;
    TLog_Label_Line line;
    for (line.start = line.end = label->text, utf8width = 0;
            *line.end != 0; line.end = g_utf8_find_next_char(line.end, NULL), ++utf8width) {
        if (*line.end == '\n' || utf8width == maxWidth) {
            g_array_append_val(label->lines, line);
            utf8width = 0;
            line.start = line.end = *line.end == '\n' ? g_utf8_find_next_char(line.end, NULL) : line.end;
        }
    }
    g_array_append_val(label->lines, line);

    if (label->lines->len > screenHeight) {
        g_array_remove_range(label->lines, screenHeight, label->lines->len - screenHeight);
    }

    return label->lines->len;
}

static void drawLine(void* widget, guint32 lineY) {
    TLog_Label* label = (TLog_Label*) widget;
    TLog_Label_Line* line = &g_array_index(label->lines, TLog_Label_Line, lineY);
    addnstr(line->start, line->end - line->start);
}
