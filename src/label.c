#include "../include/label.h"

#include <stdlib.h>
#include <string.h>

#define INIT_LINE_CAPACITY 4

/* Thanks! https://stackoverflow.com/a/3599170 */
#define UNUSED(x) (void)(x)

typedef struct tlog_label_line {
    char* start;
    char* end;
} TLog_Label_Line;

struct tlog_label {
    const TLog_Widget_Data* data;

    char* text;

    TLog_Label_Line* lines;
    size_t lineCount;
    size_t lineCapacity;

    uint32_t width;
};

static uint32_t getPreferedWidth(void* widget);
static uint32_t setMaximumWidth(void* widget, uint32_t maxWidth, uint32_t screenHeight);
static void getLine(void* widget, uint32_t lineY, char* buffer,
        uint32_t* lengthWritten, int* isReversed);

static const TLog_Widget_Data TLOG_LABEL_DATA = {
    &getPreferedWidth,
    &setMaximumWidth,
    &getLine,
    NULL,
    NULL,
    NULL
};

TLog_Label* TLog_Label_Create(char* text) {
    size_t textSize;
    TLog_Label* label;

    if (!text) {
        goto fail_arg;
    }

    label = malloc(sizeof(TLog_Label));
    if (!label) {
        goto fail_label;
    }

    label->data = &TLOG_LABEL_DATA;

    textSize = strlen(text) + 1;
    label->text = malloc(sizeof(char) * textSize);
    if (!label->text) {
        goto fail_text;
    }
    memcpy(label->text, text, textSize);
    /* TODO Remove weird characters! */

    label->lines = NULL;

    return label;

    free(label->text);
    fail_text:
    free(label);
    fail_label:
    fail_arg:
    return NULL;
}

void TLog_Label_Destroy(TLog_Label* label) {
    if (label) {
        free(label->lines);
        free(label->text);
        free(label);
    }
}

static uint32_t getPreferedWidth(void* widget) {
    TLog_Label* label;
    uint32_t preferedWidth;
    uint32_t currentWidth;
    char* ch;
    
    if (!widget) {
        return 0;
    }
    
    label = (TLog_Label*) widget;
    
    preferedWidth = currentWidth = 0;
    for (ch = label->text; *ch != 0; ++ch) {
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
    TLog_Label* label;
    TLog_Label_Line* tmpLines;
    TLog_Label_Line* line;

    UNUSED(screenHeight);

    label = (TLog_Label*) widget;

    label->width = maxWidth;

    if (!label->lines) {
        label->lines = malloc(sizeof(TLog_Label_Line) * INIT_LINE_CAPACITY);
        if (!label->lines) {
            return 0;
        }
        label->lineCapacity = INIT_LINE_CAPACITY;
    }

    /* TODO Sexy word wrap */
    for (label->lineCount = 1, line = label->lines, line->start = line->end = label->text;
            *line->end != 0; ++line->end) {
        if (*line->end == '\n'
                || line->end - line->start == label->width) {
            if (label->lineCount == label->lineCapacity) {
                tmpLines = realloc(label->lines, sizeof(TLog_Label_Line) * label->lineCapacity * 2);
                if (!tmpLines) {
                    return 0;
                }
                label->lines = tmpLines;
                label->lineCapacity = label->lineCapacity * 2;
                line = &label->lines[label->lineCount - 1];
            }
            ++line;
            ++label->lineCount;
            line->start = line->end
                    = *(line - 1)->end == '\n' ? (line - 1)->end + 1 : (line - 1)->end;
        }
    }

    return label->lineCount;
}

static void getLine(void* widget, uint32_t lineY, char* buffer,
        uint32_t* lengthWritten, int* isReversed) {
    TLog_Label* label;
    
    label = (TLog_Label*) widget;

    *isReversed = 0;
    *lengthWritten = label->lines[lineY].end - label->lines[lineY].start;
    memcpy(buffer, label->lines[lineY].start, sizeof(char) * *lengthWritten);
}
