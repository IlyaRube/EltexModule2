#ifndef CHMOD_PARSER_H
#define CHMOD_PARSER_H

#include <stddef.h>
#include <sys/types.h>

/*
 * Применяет к current команду, похожую на chmod.
 * Поддерживаются:
 *   u+x
 *   g-w
 *   o=r
 *   a+rx
 *   ug+wx
 *   g=u
 *   u+r-w
 *   u+x,g-w,o=r
 *   755
 *   rwxr-xr-x
 *
 * Изменяется только маска в памяти. Файл не изменяется.
 */
int chmod_apply(mode_t current, const char *command, mode_t *result,
                char *error, size_t error_size);

#endif
