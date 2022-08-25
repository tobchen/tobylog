/**
 * @file label.h
 * @author Tobias Heukäufer
 * @brief A label.
 */

#ifndef TLOG_INCLUDE_LABEL_H
#define TLOG_INCLUDE_LABEL_H

#include "widget.h"

/** @brief A label.  */
typedef struct tlog_label TLog_Label;

/**
 * @brief Create a label.
 * 
 * @param text Label's text
 * @return A new label, or NULL on error
 */
TLog_Label* TLog_Label_Create(char* text);

/**
 * @brief Destroy a label.
 * 
 * @param label The label to destroy
 */
void TLog_Label_Destroy(TLog_Label* label);

#endif
