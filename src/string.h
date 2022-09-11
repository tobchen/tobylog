#ifndef TLOG_SRC_STRING_H
#define TLOG_SRC_STRING_H

#include <stdbool.h>

#include <apr_pools.h>
#include <apr_tables.h>

typedef struct tlog_string {
    apr_pool_t* pool;

    char* buffer;
    size_t capacity;
    size_t len;
    size_t utf8len;
} TLog_String;

// TODO Document
int TLog_String_Init(TLog_String* str, apr_pool_t* pool);

// TODO Document
void TLog_String_Clear(TLog_String* str);

// TODO Document
int TLog_String_Set(TLog_String* str, char* value);

// TODO Document
int TLog_String_AppendASCII(TLog_String* str, char ch);

// TODO Document
void TLog_String_Pop(TLog_String* str);

#endif
