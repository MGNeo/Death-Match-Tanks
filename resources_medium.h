/*
 * Среднеуровневый модуль ресурсов. Предоставляет для resources_high.c
 * функции инициализации всех систем и загрузки всех ресурсов.
 *
 */

#ifndef RESOURCES_MEDIUM_H
#define RESOURCES_MEDIUM_H

void system_init(void);

void font_load(void);

void textures_load(void);

void sounds_load(void);

void map_load(void);

#endif
