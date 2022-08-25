#include "../include/tobylog.h"
#include "../include/label.h"

#include <stdlib.h>
#include <unistd.h>

int main(void) {
    TLog_Init();

    void* widgets[] =  {
        TLog_Label_Create("This is a label! This is such a very beautiful label!\nI agree!")
        // TLog_Label_Create("A")
    };
    TLog_Run(widgets, 1);

    sleep(3);

    TLog_Label_Destroy(widgets[0]);

    TLog_Terminate();

    return 0;
}
