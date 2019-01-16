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
    // Если имеется сообщение для показа.
    if ( (_str != NULL) && (strlen(_str) > 0) )
    {
        char message[MAX_MESSAGE_SIZE + 1];

        // Если длина сформированной строки (без учета нультерминатора) окажется
        // равна заданному пределу, то функция vsnprintf() не ставит нультерминатор,
        // поэтому ставим его заранее, на всякий случай.
        message[MAX_MESSAGE_SIZE] = 0;

        // Попытаемся сформировать сообщение с информацией об отказе.
        va_list args;
        va_start(args, _str);
        const int r_code = _vsnprintf(message, MAX_MESSAGE_SIZE, _str, args);
        va_end(args);

        // Отказ _vsprintf().
        if (r_code < 0)
        {
            MessageBox(NULL,
                       "Невозможно сформировать сообщение с информацией об ошибке, отказ _vsnprintf()",
                       "Ожидаемая критическая ошибка",
                       MB_ICONSTOP);
            goto END;
        }

        // Показываем сообщение.
        MessageBox(NULL,
                   message,
                   "Ожидаемая критическая ошибка",
                   MB_ICONSTOP);
    }

    END:
    abort();
}
