/*
 * Среднеуровневый модуль ресурсов. Предоставляет для resources_high.c
 * функцию инициализации всех систем и загрузки всех ресурсов:
 * - systems_init() - инициализация всех систем;
 * - font_load() - загрузка шрифта;
 * - textures_load() - загрузка текстур;
 * - sounds_load() - загрузка звуков;
 * - map_load() - загрузка карты.
 */

#ifndef RESOURCES_MEDIUM_H
#define RESOURCES_MEDIUM_H

void system_init(void);

void font_load(void);

void textures_load(void);

void sounds_load(void);

void map_load(void);

#endif
