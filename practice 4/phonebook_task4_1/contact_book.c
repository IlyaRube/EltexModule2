#include "contact_book.h"

#include <stdlib.h>
#include <string.h>

static int has_text(const char *text)
{
    return text != NULL && text[0] != '\0';
}

static int string_list_is_valid(const char items[][MAX_ITEM_LENGTH],
                                size_t count,
                                size_t max_count)
{
    size_t index;

    if (count > max_count) {
        return 0;
    }

    for (index = 0; index < count; index++) {
        if (!has_text(items[index])) {
            return 0;
        }
    }

    return 1;
}

static ContactNode *contact_book_get_node_mutable(ContactBook *book,
                                                   size_t index)
{
    ContactNode *node;
    size_t current_index;

    if (book == NULL || index >= book->count) {
        return NULL;
    }

    /* Идём от ближайшего края списка. */
    if (index < book->count / 2U) {
        node = book->head;
        for (current_index = 0U; current_index < index; current_index++) {
            node = node->next;
        }
    } else {
        node = book->tail;
        current_index = book->count - 1U;
        while (current_index > index) {
            node = node->prev;
            current_index--;
        }
    }

    return node;
}

static void unlink_node(ContactBook *book, ContactNode *node)
{
    if (node->prev != NULL) {
        node->prev->next = node->next;
    } else {
        book->head = node->next;
    }

    if (node->next != NULL) {
        node->next->prev = node->prev;
    } else {
        book->tail = node->prev;
    }

    node->prev = NULL;
    node->next = NULL;
}

static void insert_node_before(ContactBook *book,
                               ContactNode *node,
                               ContactNode *before)
{
    if (before == NULL) {
        /* Вставка в конец. */
        node->prev = book->tail;
        node->next = NULL;

        if (book->tail != NULL) {
            book->tail->next = node;
        } else {
            book->head = node;
        }

        book->tail = node;
        return;
    }

    node->next = before;
    node->prev = before->prev;

    if (before->prev != NULL) {
        before->prev->next = node;
    } else {
        book->head = node;
    }

    before->prev = node;
}

static void insert_node_sorted(ContactBook *book, ContactNode *node)
{
    ContactNode *current = book->head;

    /*
     * Ищем первый элемент, который больше нового контакта.
     * Равные контакты остаются в порядке их добавления.
     */
    while (current != NULL &&
           contact_compare(&current->contact, &node->contact) <= 0) {
        current = current->next;
    }

    insert_node_before(book, node, current);
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
        book->head = NULL;
        book->tail = NULL;
        book->count = 0U;
    }
}

void contact_book_destroy(ContactBook *book)
{
    ContactNode *node;

    if (book == NULL) {
        return;
    }

    node = book->head;
    while (node != NULL) {
        ContactNode *next = node->next;
        free(node);
        node = next;
    }

    contact_book_init(book);
}

int contact_is_valid(const Contact *contact)
{
    if (contact == NULL) {
        return 0;
    }

    if (!has_text(contact->last_name) || !has_text(contact->first_name)) {
        return 0;
    }

    return string_list_is_valid(contact->phones,
                                contact->phone_count,
                                MAX_PHONES) &&
           string_list_is_valid(contact->emails,
                                contact->email_count,
                                MAX_EMAILS) &&
           string_list_is_valid(contact->social_links,
                                contact->social_link_count,
                                MAX_SOCIAL_LINKS) &&
           string_list_is_valid(contact->messengers,
                                contact->messenger_count,
                                MAX_MESSENGERS);
}

int contact_compare(const Contact *left, const Contact *right)
{
    int result;

    if (left == NULL && right == NULL) {
        return 0;
    }
    if (left == NULL) {
        return -1;
    }
    if (right == NULL) {
        return 1;
    }

    result = strcmp(left->last_name, right->last_name);
    if (result != 0) {
        return result;
    }

    result = strcmp(left->first_name, right->first_name);
    if (result != 0) {
        return result;
    }

    return strcmp(left->middle_name, right->middle_name);
}

ContactBookResult contact_book_add(ContactBook *book, const Contact *contact)
{
    ContactNode *node;

    if (book == NULL || contact == NULL) {
        return CONTACT_BOOK_ERROR_INVALID_ARGUMENT;
    }

    if (!has_text(contact->last_name) || !has_text(contact->first_name)) {
        return CONTACT_BOOK_ERROR_REQUIRED_FIELD;
    }

    if (!contact_is_valid(contact)) {
        return CONTACT_BOOK_ERROR_INVALID_CONTACT;
    }

    node = malloc(sizeof(*node));
    if (node == NULL) {
        return CONTACT_BOOK_ERROR_OUT_OF_MEMORY;
    }

    node->contact = *contact;
    node->prev = NULL;
    node->next = NULL;

    insert_node_sorted(book, node);
    book->count++;
    return CONTACT_BOOK_OK;
}

ContactBookResult contact_book_update(ContactBook *book,
                                      size_t index,
                                      const Contact *contact)
{
    ContactNode *node;

    if (book == NULL || contact == NULL) {
        return CONTACT_BOOK_ERROR_INVALID_ARGUMENT;
    }

    if (!has_text(contact->last_name) || !has_text(contact->first_name)) {
        return CONTACT_BOOK_ERROR_REQUIRED_FIELD;
    }

    if (!contact_is_valid(contact)) {
        return CONTACT_BOOK_ERROR_INVALID_CONTACT;
    }

    node = contact_book_get_node_mutable(book, index);
    if (node == NULL) {
        return CONTACT_BOOK_ERROR_NOT_FOUND;
    }

    /*
     * Ф.И.О. могло измениться, поэтому узел извлекается и вставляется заново
     * в правильное место. Новая память при редактировании не выделяется.
     */
    unlink_node(book, node);
    node->contact = *contact;
    insert_node_sorted(book, node);

    return CONTACT_BOOK_OK;
}

ContactBookResult contact_book_delete(ContactBook *book, size_t index)
{
    ContactNode *node;

    if (book == NULL) {
        return CONTACT_BOOK_ERROR_INVALID_ARGUMENT;
    }

    node = contact_book_get_node_mutable(book, index);
    if (node == NULL) {
        return CONTACT_BOOK_ERROR_NOT_FOUND;
    }

    unlink_node(book, node);
    free(node);
    book->count--;
    return CONTACT_BOOK_OK;
}

const ContactNode *contact_book_get_node(const ContactBook *book, size_t index)
{
    const ContactNode *node;
    size_t current_index;

    if (book == NULL || index >= book->count) {
        return NULL;
    }

    if (index < book->count / 2U) {
        node = book->head;
        for (current_index = 0U; current_index < index; current_index++) {
            node = node->next;
        }
    } else {
        node = book->tail;
        current_index = book->count - 1U;
        while (current_index > index) {
            node = node->prev;
            current_index--;
        }
    }

    return node;
}

const Contact *contact_book_get(const ContactBook *book, size_t index)
{
    const ContactNode *node = contact_book_get_node(book, index);
    return node == NULL ? NULL : &node->contact;
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
        case CONTACT_BOOK_ERROR_INVALID_CONTACT:
            return "Контакт содержит некорректные данные.";
        case CONTACT_BOOK_ERROR_OUT_OF_MEMORY:
            return "Не удалось выделить память для нового контакта.";
        case CONTACT_BOOK_ERROR_NOT_FOUND:
            return "Контакт не найден.";
        default:
            return "Неизвестная ошибка.";
    }
}
