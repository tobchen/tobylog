/**
 * @file text.c
 * @author Tobias Heukäufer
 * @brief A text field implementation.
 */

#include "../include/text.h"

#include <stdlib.h>
#include <string.h>

/* Thanks! https://stackoverflow.com/a/3599170 */
/** @brief Marks unused function parameters to prevent unused warnings. */
#define UNUSED(x) (void)(x)

struct tlog_text {
    /** @brief Widget data. */
    const TLog_Widget_Data* data;

    /** @brief Width. */
    uint32_t width;

    /** @brief Text. */
    char* text;
    /** @brief Text length. */
    uint32_t textLength;
    /** @brief Text buffer's capacity. */
    uint32_t textCapacity;

    /** @brief Wether to consume Return input (true) or not (false). */
    int consumeReturn;
};

static uint32_t getPreferedWidth(void* widget);
static uint32_t setMaximumWidth(void* widget, uint32_t maxWidth, uint32_t screenHeight);
static void getLine(void* widget, uint32_t lineY, char* buffer,
        uint32_t* lengthWritten, int* isReversed);
static void setFocus(void* widget, int fromAbove, uint32_t* cursorX, uint32_t* cursorY);
static void putChar(void* widget, char ch,
        uint32_t* cursorX, uint32_t* cursorY, uint32_t* dirtyStart, uint32_t* dirtyEnd);
static int putAction(void* widget, TLog_Widget_Action action,
        uint32_t* cursorX, uint32_t* cursorY, uint32_t* dirtyStart, uint32_t* dirtyEnd);

static const TLog_Widget_Data TLOG_TEXT_DATA = {
    &getPreferedWidth,
    &setMaximumWidth,
    &getLine,
    &setFocus,
    &putChar,
    &putAction
};

TLog_Text* TLog_Text_Create(uint32_t maximumWidth) {
    TLog_Text* text = malloc(sizeof(TLog_Text));
    if (!text) {
        goto fail_text;
    }

    text->data = &TLOG_TEXT_DATA;

    text->text = malloc(sizeof(char) * maximumWidth);
    if (!text) {
        goto fail_text_text;
    }
    text->textCapacity = maximumWidth;
    text->textLength = 0;

    text->consumeReturn = 0;

    return text;

    free(text->text);
    fail_text_text:
    free(text);
    fail_text:
    return NULL;
}

void TLog_Text_Destroy(TLog_Text* text) {
    if (text) {
        free(text->text);
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
        text->textLength = 0;
        for (size_t i = text->textCapacity; i > 0 && *value != 0; ++i, ++value) {
            text->text[text->textLength++] = *value;
        }
    }
}

char* TLog_Text_GetText(TLog_Text* text) {
    char* result = NULL;
    if (text) {
        result = malloc(sizeof(char) * (text->textLength + 1));
        if (result) {
            memcpy(result, text->text, sizeof(char) * text->textLength);
            result[text->textLength] = 0;
        }
    }
    return result;
}

static uint32_t getPreferedWidth(void* widget) {
    return ((TLog_Text*) widget)->textCapacity + 1;
}

static uint32_t setMaximumWidth(void* widget, uint32_t maxWidth, uint32_t screenHeight) {
    UNUSED(screenHeight);

    TLog_Text* text = (TLog_Text*) widget;
    text->width = maxWidth < text->textCapacity + 1 ? maxWidth : text->textCapacity + 1;
    return 1;
}

static void getLine(void* widget, uint32_t lineY, char* buffer,
        uint32_t* lengthWritten, int* isReversed) {
    UNUSED(lineY);

    TLog_Text* text = (TLog_Text*) widget;

    *lengthWritten = text->width;
    *isReversed = 1;
    if (text->width > text->textLength) {
        memcpy(buffer, text->text, sizeof(char) * text->textLength);
        memset(&buffer[text->textLength], ' ', sizeof(char) * (text->width - text->textLength));
    } else {
        memcpy(buffer, &text->text[text->textLength - text->width + 1], sizeof(char) * (text->width - 1));
        buffer[text->width - 1] = ' ';
    }
}

static void setFocus(void* widget, int fromAbove, uint32_t* cursorX, uint32_t* cursorY) {
    UNUSED(fromAbove);

    TLog_Text* text = (TLog_Text*) widget;
    *cursorX = text->textLength < text->width ? text->textLength : text->width - 1;
    *cursorY = 0;
}

static void putChar(void* widget, char ch,
        uint32_t* cursorX, uint32_t* cursorY, uint32_t* dirtyStart, uint32_t* dirtyEnd) {
    TLog_Text* text = (TLog_Text*) widget;

    *dirtyStart = *dirtyEnd = 0;

    if (text->textLength < text->textCapacity) {
        text->text[text->textLength] = ch;
        ++text->textLength;

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
        if (text->textLength > 0) {
            --text->textLength;

            setFocus(text, 0, cursorX, cursorY);

            *dirtyEnd = 1;
        }
        consumed = 1;
    } else if (text->consumeReturn && action == TLOG_WIDGET_ACTION_RETURN) {
        consumed = 1;
    }

    return consumed;
}
