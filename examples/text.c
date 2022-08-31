#include "../include/tobylog.h"
#include "../include/label.h"
#include "../include/text.h"

#include <stdlib.h>
#include <stdio.h>

int main(void) {
    TLog_Init();

    void* widgets[] = {
        TLog_Label_Create("First Name:"),
        TLog_Text_Create(50),
        TLog_Label_Create("Surname:"),
        TLog_Text_Create(50)
    };

    TLog_Run(widgets, 4);

    gchar* firstName = TLog_Text_GetText(widgets[1]);
    gchar* surname = TLog_Text_GetText(widgets[3]);

    TLog_Label_Destroy(widgets[0]);
    TLog_Text_Destroy(widgets[1]);
    TLog_Label_Destroy(widgets[2]);
    TLog_Text_Destroy(widgets[3]);

    TLog_Terminate();

    printf("You typed in: %s %s\n", firstName, surname);

    free(firstName);
    free(surname);

    return 0;
}
