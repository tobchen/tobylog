#include "utf8.h"

char* TLog_UTF8_PrevChar(char* ch) {
    // The two most significant bits of a non-character-start-byte in UTF-8 are 10
    do {
        --ch;
    } while ((*ch & 0xc0) == 0x80);
    return ch;
}

char* TLog_UTF8_NextChar(char* ch) {
    if (*ch != 0) {
        // The two most significant bits of a non-character-start-byte in UTF-8 are 10
        do {
            ++ch;
        } while (*ch != 0 && (*ch & 0xc0) == 0x80);
    }
    return ch;
}

size_t TLog_UTF8_CharLen(char* ch) {
    return TLog_UTF8_NextChar(ch) - ch;
}
