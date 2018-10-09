#include <stdlib.h>
#include <stddef.h>
#include <limits.h>

#include "resources_high.h"
#include "resources_medium.h"
#include "crash.h"

// Инициализирует все системы и загружает все ресурсы.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void init_and_load(void)
{
    // Защита от повторного вызова.
    size_t again = 0;
    if (again++ != 0)
    {
        crash("Произошла попытка повторной инициализации всех систем и загрузки всех ресурсов\n");
    }

    // Инициализируем все системы.
    system_init();

    // Загружаем шрифт.
    font_load();

    // Загружаем текстуры.
    textures_load();

    // Загружаем звуки.
    sounds_load();

    // Загружаем карту.
    map_load();
}
