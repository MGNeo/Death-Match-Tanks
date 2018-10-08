/*
 * Модуль ячейки. Предоставляет перечисление возможных режимов ячейки.
 *
 */

#ifndef CELL_H
#define CELL_H

typedef enum e_cell_state
{
    CS_WALL,
    CS_TREE,
    CS_WATER,
    CS_EMPTY,
    CS_REPAIR
} cell_state;

#endif
