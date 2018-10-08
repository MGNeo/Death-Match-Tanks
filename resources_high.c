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
    init_system();

    // Загружаем шрифт.
    load_font();

    // Загружаем текстуры.
    load_textures();

    // Загружаем звуки.
    load_sounds();

    // Загружаем карту.
    load_map();
}
