#ifndef STORAGE_H
#define STORAGE_H

#include "contact_book.h"

typedef enum StorageResult {
    STORAGE_OK = 0,
    STORAGE_ERROR_INVALID_ARGUMENT,
    STORAGE_ERROR_NOT_FOUND,
    STORAGE_ERROR_OPEN,
    STORAGE_ERROR_READ,
    STORAGE_ERROR_WRITE,
    STORAGE_ERROR_FORMAT,
    STORAGE_ERROR_MEMORY,
    STORAGE_ERROR_REPLACE
} StorageResult;

StorageResult storage_save(const ContactBook *book, const char *file_path);
StorageResult storage_load(ContactBook *book, const char *file_path);
const char *storage_result_message(StorageResult result);

#endif
