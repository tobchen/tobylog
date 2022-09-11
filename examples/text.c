#include "../include/tobylog.h"
#include "../include/label.h"
#include "../include/text.h"

#include <stdlib.h>
#include <stdio.h>

#include <apr.h>

int main(int argc, const char *const *argv) {
    apr_app_initialize(&argc, &argv, NULL);

    apr_pool_t* pool;
    apr_pool_create(&pool, NULL);

    TLog_Init(pool);

    TLog_Widget* widgets[] = {
        (TLog_Widget*) TLog_Label_Create(pool, "First Name:"),
        (TLog_Widget*) TLog_Text_Create(pool, 50),
        (TLog_Widget*) TLog_Label_Create(pool, "Surname:"),
        (TLog_Widget*) TLog_Text_Create(pool, 50),
        NULL
    };

    TLog_Run(widgets);

    char* firstName = TLog_Text_GetText((TLog_Text*) widgets[1], pool);
    char* surname = TLog_Text_GetText((TLog_Text*) widgets[3], pool);

    fprintf(stderr, "You typed in: %s %s\n", firstName, surname);

    apr_terminate();

    return 0;
}
