#include "file_info.h"

#include "permissions.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static char file_type_symbol(mode_t mode)
{
    if (S_ISREG(mode)) {
        return '-';
    }
    if (S_ISDIR(mode)) {
        return 'd';
    }
#ifdef S_ISLNK
    if (S_ISLNK(mode)) {
        return 'l';
    }
#endif
    if (S_ISCHR(mode)) {
        return 'c';
    }
    if (S_ISBLK(mode)) {
        return 'b';
    }
    if (S_ISFIFO(mode)) {
        return 'p';
    }
#ifdef S_ISSOCK
    if (S_ISSOCK(mode)) {
        return 's';
    }
#endif
    return '?';
}

int file_info_read(const char *path, FileInfo *info,
                   char *error, size_t error_size)
{
    struct stat file_stat;

    if (path == NULL || info == NULL) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Передан пустой указатель");
        }
        return -1;
    }

    if (stat(path, &file_stat) != 0) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "stat(\"%s\"): %s",
                     path, strerror(errno));
        }
        return -1;
    }

    info->permissions = file_stat.st_mode & PERMISSIONS_MASK;
    info->type_symbol = file_type_symbol(file_stat.st_mode);
    return 0;
}
