/**
 * @file utf8.h
 * @author Tobias Heukäufer
 * @brief UTF-8 utility.
 */

#ifndef TLOG_SRC_UTF8_H
#define TLOG_SRC_UTF8_H

#include <stdlib.h>

// TODO Document
char* TLog_UTF8_PrevChar(char* ch);

// TODO Document
char* TLog_UTF8_NextChar(char* ch);

// TODO Document
size_t TLog_UTF8_CharLen(char* ch);

#endif
