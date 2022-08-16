#ifndef TLOG_INCLUDE_LABEL_H
#define TLOG_INCLUDE_LABEL_H

#include "widget.h"

typedef struct tlog_label TLog_Label;

TLog_Label* TLog_Label_Create(char* text);
void TLog_Label_Destroy(TLog_Label* label);

#endif
