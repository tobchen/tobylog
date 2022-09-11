#include "string.h"

#include <string.h>

#include <apr_strings.h>

#include "utf8.h"

#define INIT_CAP 64

static int ensureCapacity(TLog_String* str, size_t capacity);

int TLog_String_Init(TLog_String* str, apr_pool_t* pool) {
    if (!str || !pool) {
        return 0;
    }

    str->pool = pool;

    str->buffer = apr_palloc(pool, INIT_CAP * sizeof(char));
    if (!str->buffer) {
        return -1;
    }

    str->capacity = INIT_CAP;
    str->buffer[0] = 0;
    str->len = str->utf8len = 0;

    return 0;
}

void TLog_String_Clear(TLog_String* str) {
    if (str) {
        str->buffer[0] = 0;
        str->len = str->utf8len = 0;
    }
}

int TLog_String_Set(TLog_String* str, char* value) {
    if (str) {
        if (!value) {
            TLog_String_Clear(str);
        } else {
            size_t newLen = strlen(value);
            if (ensureCapacity(str, newLen + 1)) {
                return -1;
            }

            memcpy(str->buffer, value, sizeof(char) * (newLen + 1));
            str->len = newLen;
            
            char* ch;
            for (ch = str->buffer, str->utf8len = 0; *ch != 0; ++str->utf8len, ch = TLog_UTF8_NextChar(ch));
        }
    }
    return 0;
}

int TLog_String_AppendASCII(TLog_String* str, char ch) {
    if (str && ch >= ' ') {
        if (ensureCapacity(str, str->len + 2)) {
            return -1;
        }
        
        str->buffer[str->len] = ch;
        ++str->len;
        ++str->utf8len;
        str->buffer[str->len] = 0;
    }
    return 0;
}

void TLog_String_Pop(TLog_String* str) {
    if (str && str->utf8len > 0) {
        char* last = TLog_UTF8_PrevChar(&str->buffer[str->len]);
        str->len = last - str->buffer;
        str->buffer[str->len] = 0;
        --str->utf8len;
    }
}

static int ensureCapacity(TLog_String* str, size_t capacity) {
    if (capacity > str->capacity) {
        size_t newCapacity;
        for (newCapacity = str->capacity; newCapacity < capacity; newCapacity *= 2);
        
        char* newBuffer = apr_palloc(str->pool, newCapacity * sizeof(char));
        if (!newBuffer) {
            return -1;
        }

        memcpy(newBuffer, str->buffer, sizeof(char) * (str->len + 1));
    }
    return 0;
}
