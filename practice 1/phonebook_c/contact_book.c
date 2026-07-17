#include "contact_book.h"

#include <string.h>

static int has_text(const char *text)
{
    return text != NULL && text[0] != '\0';
}

void contact_init(Contact *contact)
{
    if (contact != NULL) {
        memset(contact, 0, sizeof(*contact));
    }
}

void contact_book_init(ContactBook *book)
{
    if (book != NULL) {
        memset(book, 0, sizeof(*book));
    }
}

int contact_is_valid(const Contact *contact)
{
    if (contact == NULL) {
        return 0;
    }

    if (!has_text(contact->last_name) || !has_text(contact->first_name)) {
        return 0;
    }

    if (contact->phone_count > MAX_PHONES ||
        contact->email_count > MAX_EMAILS ||
        contact->social_link_count > MAX_SOCIAL_LINKS ||
        contact->messenger_count > MAX_MESSENGERS) {
        return 0;
    }

    return 1;
}

ContactBookResult contact_book_add(ContactBook *book, const Contact *contact)
{
    if (book == NULL || contact == NULL) {
        return CONTACT_BOOK_ERROR_INVALID_ARGUMENT;
    }

    if (!contact_is_valid(contact)) {
        return CONTACT_BOOK_ERROR_REQUIRED_FIELD;
    }

    if (book->count >= MAX_CONTACTS) {
        return CONTACT_BOOK_ERROR_FULL;
    }

    book->contacts[book->count] = *contact;
    book->count++;

    return CONTACT_BOOK_OK;
}

ContactBookResult contact_book_update(ContactBook *book, size_t index, const Contact *contact)
{
    if (book == NULL || contact == NULL) {
        return CONTACT_BOOK_ERROR_INVALID_ARGUMENT;
    }

    if (index >= book->count) {
        return CONTACT_BOOK_ERROR_NOT_FOUND;
    }

    if (!contact_is_valid(contact)) {
        return CONTACT_BOOK_ERROR_REQUIRED_FIELD;
    }

    book->contacts[index] = *contact;
    return CONTACT_BOOK_OK;
}

ContactBookResult contact_book_delete(ContactBook *book, size_t index)
{
    size_t i;

    if (book == NULL) {
        return CONTACT_BOOK_ERROR_INVALID_ARGUMENT;
    }

    if (index >= book->count) {
        return CONTACT_BOOK_ERROR_NOT_FOUND;
    }

    for (i = index; i + 1 < book->count; i++) {
        book->contacts[i] = book->contacts[i + 1];
    }

    book->count--;
    contact_init(&book->contacts[book->count]);

    return CONTACT_BOOK_OK;
}

const Contact *contact_book_get(const ContactBook *book, size_t index)
{
    if (book == NULL || index >= book->count) {
        return NULL;
    }

    return &book->contacts[index];
}

int contact_matches_query(const Contact *contact, const char *query)
{
    if (contact == NULL || query == NULL || query[0] == '\0') {
        return 0;
    }

    return strstr(contact->last_name, query) != NULL ||
           strstr(contact->first_name, query) != NULL ||
           strstr(contact->middle_name, query) != NULL;
}

const char *contact_book_result_message(ContactBookResult result)
{
    switch (result) {
        case CONTACT_BOOK_OK:
            return "Операция выполнена успешно.";
        case CONTACT_BOOK_ERROR_INVALID_ARGUMENT:
            return "Передан недопустимый аргумент.";
        case CONTACT_BOOK_ERROR_REQUIRED_FIELD:
            return "Фамилия и имя обязательны для заполнения.";
        case CONTACT_BOOK_ERROR_FULL:
            return "Телефонная книга заполнена.";
        case CONTACT_BOOK_ERROR_NOT_FOUND:
            return "Контакт не найден.";
        default:
            return "Неизвестная ошибка.";
    }
}
