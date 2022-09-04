/**
 * @file label.h
 * @author Tobias Heukäufer
 * @brief A label.
 */

#ifndef TLOG_INCLUDE_LABEL_H
#define TLOG_INCLUDE_LABEL_H

#include <apr_pools.h>

#include "widget.h"

/** @brief A label. */
typedef struct tlog_label TLog_Label;

/**
 * @brief Creates a label.
 * 
 * @param pool Pool to handle the label
 * @param text Label's text
 * @return A new label, or NULL on error
 */
TLog_Label* TLog_Label_Create(apr_pool_t* pool, char* text);

#endif
