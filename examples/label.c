#include "../include/tobylog.h"
#include "../include/label.h"

#include <stdlib.h>
#include <unistd.h>

int main(void) {
    void* widgets[1];

    TLog_Init();

    widgets[0] = TLog_Label_Create("This is a label! This is such a very beautiful label!");

    TLog_Run(widgets, 1);

    sleep(3);

    TLog_Terminate();

    return 0;
}
