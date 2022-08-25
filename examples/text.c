#include "../include/tobylog.h"
#include "../include/label.h"
#include "../include/text.h"

#include <stdlib.h>

int main(void) {
    TLog_Init();

    void* widgets[] = {
        TLog_Label_Create("First Name:"),
        TLog_Text_Create(50),
        TLog_Label_Create("Surname:"),
        TLog_Text_Create(50)
    };
    TLog_Run(widgets, 4);

    TLog_Label_Destroy(widgets[0]);
    TLog_Text_Destroy(widgets[1]);
    TLog_Label_Destroy(widgets[2]);
    TLog_Text_Destroy(widgets[3]);

    TLog_Terminate();

    return 0;
}
