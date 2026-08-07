#include "storage.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

/*
 * Бинарный формат файла contacts.txt, версия 1.
 *
 * Структура файла:
 *   16 байт  - сигнатура "PHONEBOOK_BIN_V1"
 *   4 байта  - количество контактов (uint32_t, little-endian)
 *   далее    - контакты по порядку симметричного обхода BST
 *
 * Каждая строка хранится как:
 *   4 байта  - длина строки без '\0'
 *   N байт   - содержимое строки
 *
 * Каждый список (телефоны, email и т. п.) хранится как:
 *   4 байта  - количество элементов
 *   далее    - строки в описанном выше формате
 *
 * Мы намеренно не пишем структуру Contact через fwrite целиком:
 * в ней есть size_t и возможные байты выравнивания, поэтому такой файл
 * зависел бы от архитектуры и компилятора.
 */
#define STORAGE_MAGIC "PHONEBOOK_BIN_V1"
#define STORAGE_MAGIC_SIZE 16u
#define TEMP_PATH_SIZE 1024
#define MAX_STORED_CONTACTS 100000u

static int write_bytes(FILE *file, const void *data, size_t size)
{
    return size == 0 || fwrite(data, 1, size, file) == size;
}

static StorageResult read_bytes(FILE *file, void *data, size_t size)
{
    size_t read_count;

    if (size == 0) {
        return STORAGE_OK;
    }

    read_count = fread(data, 1, size, file);
    if (read_count == size) {
        return STORAGE_OK;
    }

    return ferror(file) ? STORAGE_ERROR_READ : STORAGE_ERROR_FORMAT;
}

static int write_u32_le(FILE *file, uint32_t value)
{
    unsigned char bytes[4];

    bytes[0] = (unsigned char)(value & 0xFFu);
    bytes[1] = (unsigned char)((value >> 8) & 0xFFu);
    bytes[2] = (unsigned char)((value >> 16) & 0xFFu);
    bytes[3] = (unsigned char)((value >> 24) & 0xFFu);

    return write_bytes(file, bytes, sizeof(bytes));
}

static StorageResult read_u32_le(FILE *file, uint32_t *value)
{
    unsigned char bytes[4];
    StorageResult result;

    if (value == NULL) {
        return STORAGE_ERROR_INVALID_ARGUMENT;
    }

    result = read_bytes(file, bytes, sizeof(bytes));
    if (result != STORAGE_OK) {
        return result;
    }

    *value = (uint32_t)bytes[0] |
             ((uint32_t)bytes[1] << 8) |
             ((uint32_t)bytes[2] << 16) |
             ((uint32_t)bytes[3] << 24);

    return STORAGE_OK;
}

static int write_binary_string(FILE *file, const char *text)
{
    size_t length;

    if (text == NULL) {
        return 0;
    }

    length = strlen(text);
    if (length > UINT32_MAX) {
        return 0;
    }

    return write_u32_le(file, (uint32_t)length) &&
           write_bytes(file, text, length);
}

static StorageResult read_binary_string(FILE *file,
                                        char *buffer,
                                        size_t buffer_size)
{
    uint32_t stored_length;
    StorageResult result;

    if (buffer == NULL || buffer_size == 0) {
        return STORAGE_ERROR_INVALID_ARGUMENT;
    }

    result = read_u32_le(file, &stored_length);
    if (result != STORAGE_OK) {
        return result;
    }

    if ((size_t)stored_length >= buffer_size) {
        return STORAGE_ERROR_FORMAT;
    }

    result = read_bytes(file, buffer, (size_t)stored_length);
    if (result != STORAGE_OK) {
        return result;
    }

    /* В корректном файле внутри строки не должно быть встроенного '\0'. */
    if (stored_length > 0 &&
        memchr(buffer, '\0', (size_t)stored_length) != NULL) {
        return STORAGE_ERROR_FORMAT;
    }

    buffer[stored_length] = '\0';
    return STORAGE_OK;
}

static int write_string_list(FILE *file,
                             const char items[][MAX_ITEM_LENGTH],
                             size_t count)
{
    size_t i;

    if (count > UINT32_MAX || !write_u32_le(file, (uint32_t)count)) {
        return 0;
    }

    for (i = 0; i < count; i++) {
        if (!write_binary_string(file, items[i])) {
            return 0;
        }
    }

    return 1;
}

static StorageResult read_string_list(FILE *file,
                                      char items[][MAX_ITEM_LENGTH],
                                      size_t max_count,
                                      size_t *count)
{
    uint32_t stored_count;
    size_t i;
    StorageResult result;

    if (count == NULL) {
        return STORAGE_ERROR_INVALID_ARGUMENT;
    }

    result = read_u32_le(file, &stored_count);
    if (result != STORAGE_OK) {
        return result;
    }

    if ((size_t)stored_count > max_count) {
        return STORAGE_ERROR_FORMAT;
    }

    *count = (size_t)stored_count;

    for (i = 0; i < *count; i++) {
        result = read_binary_string(file, items[i], MAX_ITEM_LENGTH);
        if (result != STORAGE_OK) {
            return result;
        }

        if (items[i][0] == '\0') {
            return STORAGE_ERROR_FORMAT;
        }
    }

    return STORAGE_OK;
}

static int write_contact(FILE *file, const Contact *contact)
{
    return write_binary_string(file, contact->last_name) &&
           write_binary_string(file, contact->first_name) &&
           write_binary_string(file, contact->middle_name) &&
           write_binary_string(file, contact->workplace) &&
           write_binary_string(file, contact->position) &&
           write_string_list(file, contact->phones, contact->phone_count) &&
           write_string_list(file, contact->emails, contact->email_count) &&
           write_string_list(file, contact->social_links,
                             contact->social_link_count) &&
           write_string_list(file, contact->messengers,
                             contact->messenger_count);
}

static StorageResult read_contact(FILE *file, Contact *contact)
{
    StorageResult result;

    if (contact == NULL) {
        return STORAGE_ERROR_INVALID_ARGUMENT;
    }

    contact_init(contact);

    result = read_binary_string(file, contact->last_name,
                                sizeof(contact->last_name));
    if (result != STORAGE_OK) {
        return result;
    }

    result = read_binary_string(file, contact->first_name,
                                sizeof(contact->first_name));
    if (result != STORAGE_OK) {
        return result;
    }

    result = read_binary_string(file, contact->middle_name,
                                sizeof(contact->middle_name));
    if (result != STORAGE_OK) {
        return result;
    }

    result = read_binary_string(file, contact->workplace,
                                sizeof(contact->workplace));
    if (result != STORAGE_OK) {
        return result;
    }

    result = read_binary_string(file, contact->position,
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

    if (book->count > MAX_STORED_CONTACTS || book->count > UINT32_MAX) {
        return STORAGE_ERROR_FORMAT;
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

    if (!write_bytes(file, STORAGE_MAGIC, STORAGE_MAGIC_SIZE) ||
        !write_u32_le(file, (uint32_t)book->count)) {
        fclose(file);
        remove(temporary_path);
        return STORAGE_ERROR_WRITE;
    }

    /* Симметричный обход через contact_book_get сохраняет контакты по Ф.И.О. */
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
    unsigned char magic[STORAGE_MAGIC_SIZE];
    uint32_t stored_count;
    size_t i;
    StorageResult result;
    int trailing_byte;

    if (book == NULL || file_path == NULL || file_path[0] == '\0') {
        return STORAGE_ERROR_INVALID_ARGUMENT;
    }

    file = fopen(file_path, "rb");
    if (file == NULL) {
        return errno == ENOENT ? STORAGE_ERROR_NOT_FOUND : STORAGE_ERROR_OPEN;
    }

    result = read_bytes(file, magic, sizeof(magic));
    if (result != STORAGE_OK) {
        fclose(file);
        return result;
    }

    if (memcmp(magic, STORAGE_MAGIC, STORAGE_MAGIC_SIZE) != 0) {
        fclose(file);
        return STORAGE_ERROR_FORMAT;
    }

    result = read_u32_le(file, &stored_count);
    if (result != STORAGE_OK) {
        fclose(file);
        return result;
    }

    if (stored_count > MAX_STORED_CONTACTS) {
        fclose(file);
        return STORAGE_ERROR_FORMAT;
    }

    contact_book_init(&loaded_book);

    for (i = 0; i < (size_t)stored_count; i++) {
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

    /* После последней записи в корректном бинарном файле больше ничего нет. */
    trailing_byte = fgetc(file);
    if (trailing_byte != EOF) {
        fclose(file);
        contact_book_clear(&loaded_book);
        return STORAGE_ERROR_FORMAT;
    }
    if (ferror(file)) {
        fclose(file);
        contact_book_clear(&loaded_book);
        return STORAGE_ERROR_READ;
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
            return "Ошибка чтения бинарного файла с контактами.";
        case STORAGE_ERROR_WRITE:
            return "Ошибка записи бинарного файла с контактами.";
        case STORAGE_ERROR_FORMAT:
            return "Бинарный файл с контактами повреждён или имеет неверный формат.";
        case STORAGE_ERROR_REPLACE:
            return "Не удалось заменить старый файл новым.";
        default:
            return "Неизвестная ошибка работы с файлом.";
    }
}
