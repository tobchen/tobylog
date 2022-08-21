#ifndef TLOG_INCLUDE_TEXT_H
#define TLOG_INCLUDE_TEXT_H

#include "widget.h"

typedef struct tlog_text TLog_Text;

TLog_Text* TLog_Text_Create(uint32_t maximumWidth);
void TLog_Text_Destroy(TLog_Text* text);

void TLog_Text_SetConsumeEnter(TLog_Text* text, int consumeEnter);

char* TLog_Text_GetText(TLog_Text* text);

#endif
