#include "string.h"

#include <string.h>

#include <apr_strings.h>

#include "utf8.h"

#define INIT_CAP 64

int TLog_String_Init(TLog_String* str, apr_pool_t* pool) {
    if (!str) {
        return 0;
    }

    str->pool = pool;

    str->buf = apr_array_make(pool, INIT_CAP, sizeof(char));
    if (!str->buf) {
        return -1;
    }
    APR_ARRAY_PUSH(str->buf, char) = 0;
    str->len = str->utf8len = 0;

    return 0;
}

void TLog_String_Clear(TLog_String* str) {
    if (str) {
        apr_array_clear(str->buf);
        APR_ARRAY_PUSH(str->buf, char) = 0;
        str->len = str->utf8len = 0;
    }
}

void TLog_String_Set(TLog_String* str, char* value) {
    if (str) {
        if (!value) {
            TLog_String_Clear(str);
        } else {
            apr_array_clear(str->buf);

            for (char* ch = value; *ch != 0; ++ch) {
                APR_ARRAY_PUSH(str->buf, char) = *ch;
            }
            str->len = str->buf->nelts;
            APR_ARRAY_PUSH(str->buf, char) = 0;
            
            char* ch;
            for (ch = str->buf->elts, str->utf8len = 0; *ch != 0; ++str->utf8len, ch = TLog_UTF8_NextChar(ch));
        }
    }
}

void TLog_String_AppendASCII(TLog_String* str, char ch) {
    if (str && ch >= ' ') {
        APR_ARRAY_IDX(str->buf, str->len, char) = ch;
        APR_ARRAY_PUSH(str->buf, char) = 0;
        ++str->len;
        ++str->utf8len;
    }
}

void TLog_String_Pop(TLog_String* str) {
    if (str && str->utf8len > 0) {
        char* last = TLog_UTF8_PrevChar(TLog_String_CharAt(str, str->len));
        int targetLen = last - TLog_String_CharAt(str, 0);
        while (str->buf->nelts > targetLen) {
            apr_array_pop(str->buf);
        }

        str->len = str->buf->nelts;
        --str->utf8len;
        APR_ARRAY_PUSH(str->buf, char) = 0;
    }
}

char* TLog_String_CharAt(TLog_String* str, size_t i) {
    return str && (int) i < str->buf->nelts ? &APR_ARRAY_IDX(str->buf, i, char) : NULL;
}
