#include "storage.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define STORAGE_HEADER "PHONEBOOK_TEXT_V1"
#define TEMP_PATH_SIZE 1024
#define NUMBER_BUFFER_SIZE 64
#define MAX_STORED_CONTACTS 100000

typedef enum LineReadResult {
    LINE_READ_OK = 0,
    LINE_READ_END,
    LINE_READ_ERROR,
    LINE_READ_TOO_LONG
} LineReadResult;

static LineReadResult read_line(FILE *file, char *buffer, size_t buffer_size)
{
    size_t length;
    int ch;

    if (fgets(buffer, (int)buffer_size, file) == NULL) {
        return ferror(file) ? LINE_READ_ERROR : LINE_READ_END;
    }

    length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[--length] = '\0';
        if (length > 0 && buffer[length - 1] == '\r') {
            buffer[length - 1] = '\0';
        }
        return LINE_READ_OK;
    }

    ch = fgetc(file);
    if (ch == '\n' || ch == EOF) {
        return LINE_READ_OK;
    }

    while (ch != '\n' && ch != EOF) {
        ch = fgetc(file);
    }

    return LINE_READ_TOO_LONG;
}

static StorageResult line_result_to_storage(LineReadResult result)
{
    switch (result) {
        case LINE_READ_OK:
            return STORAGE_OK;
        case LINE_READ_ERROR:
            return STORAGE_ERROR_READ;
        case LINE_READ_END:
        case LINE_READ_TOO_LONG:
        default:
            return STORAGE_ERROR_FORMAT;
    }
}

static StorageResult read_required_line(FILE *file,
                                        char *buffer,
                                        size_t buffer_size)
{
    return line_result_to_storage(read_line(file, buffer, buffer_size));
}

static StorageResult read_size_value(FILE *file,
                                     size_t max_value,
                                     size_t *value)
{
    char buffer[NUMBER_BUFFER_SIZE];
    char *end;
    unsigned long long parsed;
    StorageResult result;

    result = read_required_line(file, buffer, sizeof(buffer));
    if (result != STORAGE_OK) {
        return result;
    }

    errno = 0;
    end = NULL;
    parsed = strtoull(buffer, &end, 10);

    if (errno != 0 || end == buffer || *end != '\0' || parsed > max_value) {
        return STORAGE_ERROR_FORMAT;
    }

    *value = (size_t)parsed;
    return STORAGE_OK;
}

static StorageResult read_string_list(FILE *file,
                                      char items[][MAX_ITEM_LENGTH],
                                      size_t max_count,
                                      size_t *count)
{
    size_t i;
    StorageResult result;

    result = read_size_value(file, max_count, count);
    if (result != STORAGE_OK) {
        return result;
    }

    for (i = 0; i < *count; i++) {
        result = read_required_line(file, items[i], MAX_ITEM_LENGTH);
        if (result != STORAGE_OK) {
            return result;
        }

        if (items[i][0] == '\0') {
            return STORAGE_ERROR_FORMAT;
        }
    }

    return STORAGE_OK;
}

static StorageResult read_contact(FILE *file, Contact *contact)
{
    StorageResult result;

    contact_init(contact);

    result = read_required_line(file, contact->last_name,
                                sizeof(contact->last_name));
    if (result != STORAGE_OK) {
        return result;
    }

    result = read_required_line(file, contact->first_name,
                                sizeof(contact->first_name));
    if (result != STORAGE_OK) {
        return result;
    }

    result = read_required_line(file, contact->middle_name,
                                sizeof(contact->middle_name));
    if (result != STORAGE_OK) {
        return result;
    }

    result = read_required_line(file, contact->workplace,
                                sizeof(contact->workplace));
    if (result != STORAGE_OK) {
        return result;
    }

    result = read_required_line(file, contact->position,
                                sizeof(contact->position));
    if (result != STORAGE_OK) {
        return result;
    }

    result = read_string_list(file, contact->phones, MAX_PHONES,
                              &contact->phone_count);
    if (result != STORAGE_OK) {
        return result;
    }

    result = read_string_list(file, contact->emails, MAX_EMAILS,
                              &contact->email_count);
    if (result != STORAGE_OK) {
        return result;
    }

    result = read_string_list(file, contact->social_links, MAX_SOCIAL_LINKS,
                              &contact->social_link_count);
    if (result != STORAGE_OK) {
        return result;
    }

    result = read_string_list(file, contact->messengers, MAX_MESSENGERS,
                              &contact->messenger_count);
    if (result != STORAGE_OK) {
        return result;
    }

    return contact_is_valid(contact) ? STORAGE_OK : STORAGE_ERROR_FORMAT;
}

static int write_line(FILE *file, const char *text)
{
    return fputs(text, file) != EOF && fputc('\n', file) != EOF;
}

static int write_size_value(FILE *file, size_t value)
{
    return fprintf(file, "%zu\n", value) >= 0;
}

static int write_string_list(FILE *file,
                             const char items[][MAX_ITEM_LENGTH],
                             size_t count)
{
    size_t i;

    if (!write_size_value(file, count)) {
        return 0;
    }

    for (i = 0; i < count; i++) {
        if (!write_line(file, items[i])) {
            return 0;
        }
    }

    return 1;
}

static int write_contact(FILE *file, const Contact *contact)
{
    return write_line(file, contact->last_name) &&
           write_line(file, contact->first_name) &&
           write_line(file, contact->middle_name) &&
           write_line(file, contact->workplace) &&
           write_line(file, contact->position) &&
           write_string_list(file, contact->phones, contact->phone_count) &&
           write_string_list(file, contact->emails, contact->email_count) &&
           write_string_list(file, contact->social_links,
                             contact->social_link_count) &&
           write_string_list(file, contact->messengers,
                             contact->messenger_count);
}

static int replace_file(const char *temporary_path, const char *file_path)
{
#ifdef _WIN32
    return MoveFileExA(temporary_path, file_path,
                       MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    return rename(temporary_path, file_path) == 0;
#endif
}

StorageResult storage_save(const ContactBook *book, const char *file_path)
{
    char temporary_path[TEMP_PATH_SIZE];
    FILE *file;
    size_t i;
    int path_length;

    if (book == NULL || file_path == NULL || file_path[0] == '\0') {
        return STORAGE_ERROR_INVALID_ARGUMENT;
    }

    for (i = 0; i < book->count; i++) {
        const Contact *contact = contact_book_get(book, i);
        if (contact == NULL || !contact_is_valid(contact)) {
            return STORAGE_ERROR_FORMAT;
        }
    }

    path_length = snprintf(temporary_path, sizeof(temporary_path),
                           "%s.tmp", file_path);
    if (path_length < 0 || (size_t)path_length >= sizeof(temporary_path)) {
        return STORAGE_ERROR_INVALID_ARGUMENT;
    }

    file = fopen(temporary_path, "wb");
    if (file == NULL) {
        return STORAGE_ERROR_OPEN;
    }

    if (!write_line(file, STORAGE_HEADER) ||
        !write_size_value(file, book->count)) {
        fclose(file);
        remove(temporary_path);
        return STORAGE_ERROR_WRITE;
    }

    /* Симметричный обход дерева через contact_book_get сохраняет Ф.И.О. по порядку. */
    for (i = 0; i < book->count; i++) {
        const Contact *contact = contact_book_get(book, i);
        if (contact == NULL || !write_contact(file, contact)) {
            fclose(file);
            remove(temporary_path);
            return STORAGE_ERROR_WRITE;
        }
    }

    if (fclose(file) != 0) {
        remove(temporary_path);
        return STORAGE_ERROR_WRITE;
    }

    if (!replace_file(temporary_path, file_path)) {
        remove(temporary_path);
        return STORAGE_ERROR_REPLACE;
    }

    return STORAGE_OK;
}

StorageResult storage_load(ContactBook *book, const char *file_path)
{
    FILE *file;
    ContactBook loaded_book;
    char header[64];
    size_t stored_count;
    size_t i;
    StorageResult result;

    if (book == NULL || file_path == NULL || file_path[0] == '\0') {
        return STORAGE_ERROR_INVALID_ARGUMENT;
    }

    file = fopen(file_path, "rb");
    if (file == NULL) {
        return errno == ENOENT ? STORAGE_ERROR_NOT_FOUND : STORAGE_ERROR_OPEN;
    }

    result = read_required_line(file, header, sizeof(header));
    if (result != STORAGE_OK) {
        fclose(file);
        return result;
    }

    if (strcmp(header, STORAGE_HEADER) != 0) {
        fclose(file);
        return STORAGE_ERROR_FORMAT;
    }

    result = read_size_value(file, MAX_STORED_CONTACTS, &stored_count);
    if (result != STORAGE_OK) {
        fclose(file);
        return result;
    }

    contact_book_init(&loaded_book);

    for (i = 0; i < stored_count; i++) {
        Contact contact;
        ContactBookResult add_result;

        result = read_contact(file, &contact);
        if (result != STORAGE_OK) {
            fclose(file);
            contact_book_clear(&loaded_book);
            return result;
        }

        add_result = contact_book_add(&loaded_book, &contact);
        if (add_result != CONTACT_BOOK_OK) {
            fclose(file);
            contact_book_clear(&loaded_book);
            return add_result == CONTACT_BOOK_ERROR_MEMORY
                       ? STORAGE_ERROR_READ
                       : STORAGE_ERROR_FORMAT;
        }
    }

    if (fclose(file) != 0) {
        contact_book_clear(&loaded_book);
        return STORAGE_ERROR_READ;
    }

    /* Старое дерево освобождается только после полностью успешной загрузки. */
    contact_book_clear(book);
    *book = loaded_book;
    return STORAGE_OK;
}

const char *storage_result_message(StorageResult result)
{
    switch (result) {
        case STORAGE_OK:
            return "Операция с файлом выполнена успешно.";
        case STORAGE_ERROR_INVALID_ARGUMENT:
            return "Передан недопустимый путь или указатель.";
        case STORAGE_ERROR_NOT_FOUND:
            return "Файл с контактами не найден.";
        case STORAGE_ERROR_OPEN:
            return "Не удалось открыть файл с контактами.";
        case STORAGE_ERROR_READ:
            return "Ошибка чтения файла с контактами.";
        case STORAGE_ERROR_WRITE:
            return "Ошибка записи файла с контактами.";
        case STORAGE_ERROR_FORMAT:
            return "Файл с контактами повреждён или имеет неверный формат.";
        case STORAGE_ERROR_REPLACE:
            return "Не удалось заменить старый файл новым.";
        default:
            return "Неизвестная ошибка работы с файлом.";
    }
}
