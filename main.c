#include "resources_high.h"
#include "game.h"

int main(int argc, char **argv)
{
    // Инициализация всех систем и загрузка всех ресурсов.
    init_and_load();

    // Начинаем игру.
    game();
}
