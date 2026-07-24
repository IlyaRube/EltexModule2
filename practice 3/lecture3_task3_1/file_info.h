#ifndef FILE_INFO_H
#define FILE_INFO_H

#include <stddef.h>
#include <sys/types.h>

typedef struct FileInfo {
    mode_t permissions;
    char type_symbol;
} FileInfo;

/* Получает информацию о файле через stat(). */
int file_info_read(const char *path, FileInfo *info,
                   char *error, size_t error_size);

#endif
