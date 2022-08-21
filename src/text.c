#include "../include/text.h"

#include <stdlib.h>
#include <string.h>

struct tlog_text {
    const TLog_Widget_Data* data;

    uint32_t widgetWidth;

    char* text;
    uint32_t currentWidth;
    uint32_t maximumWidth;

    int consumeEnter;
};

static uint32_t getPreferedWidth(void* widget);
static uint32_t setMaximumWidth(void* widget, uint32_t maxWidth, uint32_t screenHeight);
static void getLine(void* widget, uint32_t lineY, char* buffer, uint32_t maxLength,
        uint32_t* writtenLength, int* isReversed);
static void setFocus(void* widget, uint32_t* cursorX, uint32_t* cursorY);
static void putChar(void* widget, char ch,
        uint32_t* cursorX, uint32_t* cursorY, uint32_t dirtyStart, uint32_t dirtyEnd);
static int putAction(void* widget, TLog_Widget_Action action,
        uint32_t* cursorX, uint32_t* cursorY, uint32_t dirtyStart, uint32_t dirtyEnd);

static const TLog_Widget_Data TLOG_TEXT_DATA = {
    &getPreferedWidth,
    &setMaximumWidth,
    &getLine,
    &setFocus,
    &putChar,
    &putAction
};

TLog_Text* TLog_Text_Create(uint32_t maximumWidth) {
    TLog_Text* text;

    text = malloc(sizeof(TLog_Text));
    if (!text) {
        goto fail_text;
    }

    text->data = &TLOG_TEXT_DATA;

    text->text = malloc(sizeof(char) * maximumWidth);
    if (!text) {
        goto fail_text_text;
    }
    text->maximumWidth = maximumWidth;
    text->currentWidth = 0;

    text->consumeEnter = 0;

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

void TLog_Text_SetConsumeEnter(TLog_Text* text, int consumeEnter) {
    if (text) {
        text->consumeEnter = consumeEnter;
    }
}

char* TLog_Text_GetText(TLog_Text* text) {
    char* result = NULL;
    if (text) {
        char* result = malloc(sizeof(char) * (text->currentWidth + 1));
        if (result) {
            memcpy(result, text->text, sizeof(char) * text->currentWidth);
            result[text->currentWidth] = 0;
        }
    }
    return result;
}

static uint32_t getPreferedWidth(void* widget) {
    return ((TLog_Text*) widget)->maximumWidth + 1;
}

static uint32_t setMaximumWidth(void* widget, uint32_t maxWidth, uint32_t screenHeight) {
    TLog_Text* text = (TLog_Text*) widget;
    text->widgetWidth = maxWidth < text->maximumWidth + 1 ? maxWidth : text->maximumWidth + 1;
    return 1;
}

static void getLine(void* widget, uint32_t lineY, char* buffer, uint32_t maxLength,
        uint32_t* writtenLength, int* isReversed) {
    
}

static void setFocus(void* widget, uint32_t* cursorX, uint32_t* cursorY);
static void putChar(void* widget, char ch,
        uint32_t* cursorX, uint32_t* cursorY, uint32_t dirtyStart, uint32_t dirtyEnd);
        
static int putAction(void* widget, TLog_Widget_Action action,
        uint32_t* cursorX, uint32_t* cursorY, uint32_t dirtyStart, uint32_t dirtyEnd);
