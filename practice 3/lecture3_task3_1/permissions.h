#ifndef PERMISSIONS_H
#define PERMISSIONS_H

#include <stddef.h>
#include <sys/types.h>

#define PERMISSIONS_MASK ((mode_t)0777)
#define PERMISSIONS_SYMBOLIC_SIZE 10
#define PERMISSIONS_BITS_SIZE 12
#define PERMISSIONS_OCTAL_SIZE 4

/*
 * Преобразует цифровое или буквенное представление прав в mode_t.
 * Поддерживаются: 755, 0755, rwxr-xr-x и -rwxr-xr-x.
 * Возвращает 0 при успехе, иначе -1.
 */
int permissions_parse(const char *text, mode_t *result,
                      char *error, size_t error_size);

/* Формирует строку вида rwxr-xr-x. */
void permissions_to_symbolic(mode_t mode,
                             char result[PERMISSIONS_SYMBOLIC_SIZE]);

/* Формирует строку вида 111 101 101. */
void permissions_to_bits(mode_t mode,
                         char result[PERMISSIONS_BITS_SIZE]);

/* Формирует строку вида 755. */
void permissions_to_octal(mode_t mode,
                          char result[PERMISSIONS_OCTAL_SIZE]);

#endif
