#include "../include/tobylog.h"
#include "../include/label.h"

#include <stdlib.h>
#include <unistd.h>

#include <apr.h>

int main(int argc, const char *const *argv) {
    apr_app_initialize(&argc, &argv, NULL);

    apr_pool_t* pool;
    apr_pool_create(&pool, NULL);

    TLog_Init(pool);

    void* widgets[] =  {
        TLog_Label_Create(pool, "This is a label! This is such a very beautiful label!\nI agree!"),
        NULL
    };
    TLog_Run(widgets);

    sleep(3);

    apr_terminate();

    return 0;
}
