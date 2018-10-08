#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "crash.h"

#define MAX_MESSAGE_SIZE 4096

// Показывает окно с сообщением о причине сбоя, после чего крашит программу.
// Может использоваться до инициализации SDL.
// Допускает _str == NULL и _str == ""
void crash(const char *const _str,
           ...)
{
    if ( (_str != NULL) && (strlen(_str) > 0) )
    {
        char message[MAX_MESSAGE_SIZE];

        // Формируем сообщение.
        va_list args;
        va_start(args, _str);

        const int r_code = vsprintf(message, _str, args);
        if (r_code < 0)
        {
            goto END;
        }

        va_end(args);

        // Показываем сообщение.
        MessageBox(NULL, message, "Ожидаемая критическая ошибка", MB_ICONSTOP);
    }

    END:
    abort();
}
