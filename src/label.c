#include "../include/label.h"

#include <stdlib.h>
#include <string.h>

/* Thanks! https://stackoverflow.com/a/3599170 */
#define UNUSED(x) (void)(x)

static uint32_t getPreferedWidth(void* widget);
static uint32_t setMaximumWidth(void* widget, uint32_t maxWidth, uint32_t screenHeight);
static void getLine(void* widget, uint32_t lineY, char* buffer, uint32_t maxLength,
        uint32_t* lengthWritten, int* isReversed);
static int setFocus(void* widget, uint32_t* cursorX, uint32_t* cursorY);

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
};

static const TLog_Widget_Data TLog_Label_Data = {
    &getPreferedWidth,
    &setMaximumWidth,
    &getLine,
    &setFocus,
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

    label->data = &TLog_Label_Data;

    textSize = strlen(text) + 1;
    label->text = malloc(sizeof(char) * textSize);
    if (!label->text) {
        goto fail_text;
    }
    memcpy(label->text, text, textSize);
    /* TODO Remove weird characters! */

    label->lines = malloc(sizeof(TLog_Label_Line) * 1);
    if (!label->lines) {
        goto fail_lines;
    }
    label->lineCapacity = 1;

    return label;

    free(label->lines);
    fail_lines:
    free(label->text);
    fail_text:
    free(label);
    fail_label:
    fail_arg:
    return NULL;
}

void TLog_Label_Destroy(TLog_Label* label) {
    if (!label) {
        return;
    }

    free(label->lines);
    free(label->text);
    free(label);
}

static uint32_t getPreferedWidth(void* widget) {
    TLog_Label* label;
    char* ch;

    if (!widget) {
        return 0;
    }
    
    label = (TLog_Label*) widget;
    
    /* TODO Automatic line breaks! */
    for (ch = label->text; *ch != 0; ++ch) {
        if (*ch == '\n') {
            break;
        }
    }
    label->lines[0].start = label->text;
    label->lines[0].end = ch;
    label->lineCount = 1;

    return ch - label->text;
}

static uint32_t setMaximumWidth(void* widget, uint32_t maxWidth, uint32_t screenHeight) {
    if (!widget) {
        return 0;
    }

    UNUSED(maxWidth);
    UNUSED(screenHeight);
    
    /* TODO Automatic line breaks! */
    return 1;
}

static void getLine(void* widget, uint32_t lineY, char* buffer, uint32_t maxLength,
        uint32_t* lengthWritten, int* isReversed) {
    TLog_Label* label;
    
    if (!widget) {
        *lengthWritten = 0;
        return;
    }
    
    label = (TLog_Label*) widget;

    if (lineY >= label->lineCount) {
        *lengthWritten = 0;
        return;
    }

    *isReversed = 0;
    if (label->lines[lineY].end - label->lines[lineY].start <= maxLength) {
        memcpy(buffer, label->lines[lineY].start, label->lines[lineY].end - label->lines[lineY].start);
        *lengthWritten = label->lines[lineY].end - label->lines[lineY].start;
    } else {
        *lengthWritten = maxLength;

        if (maxLength > 3) {
            memcpy(buffer, label->lines[lineY].start, maxLength - 3);
            buffer += maxLength - 3;
            maxLength = 3;
        }

        for (; maxLength != 0; --maxLength, ++buffer) {
            *buffer = '.';
        }
    }
}

static int setFocus(void* widget, uint32_t* cursorX, uint32_t* cursorY) {
    UNUSED(widget);
    UNUSED(cursorX);
    UNUSED(cursorY);

    return 0;
}
