/**
 * @file text.h
 * @author Tobias Heukäufer
 * @brief A text field.
 */

#ifndef TLOG_INCLUDE_TEXT_H
#define TLOG_INCLUDE_TEXT_H

#include "widget.h"

#include <apr_pools.h>

/** @brief A text field. */
typedef struct tlog_text TLog_Text;

/**
 * @brief Creates a text field.
 * 
 * @param pool Memory pool
 * @param maximumWidth Maximum width of text the field can hold
 * @return A new text field, or NULL on error
 */
TLog_Text* TLog_Text_Create(apr_pool_t* pool, size_t maximumWidth);

/**
 * @brief Sets wether a text field is to consume Return inputs.
 * 
 * Default behaviour is to not consume.
 * 
 * @param text The text field
 * @param consumeReturn Wether to consume (true) or not (false)
 */
void TLog_Text_SetConsumeReturn(TLog_Text* text, bool consumeReturn);

/**
 * @brief Sets a text field's text.
 * 
 * If the given text is longer than the text field's capacity it will be cut off.
 * 
 * @param text The text field
 * @param value The text
 */
void TLog_Text_SetText(TLog_Text* text, char* value);

/**
 * @brief Returns a text field's text.
 * 
 * The returned text will be newly allocated and is owned by the caller (free with g_free).
 * 
 * @param text The text field
 * @param pool Memory pool for the text
 * @return The text field's text, or NULL on error
 */
char* TLog_Text_GetText(TLog_Text* text, apr_pool_t* pool);

#endif
