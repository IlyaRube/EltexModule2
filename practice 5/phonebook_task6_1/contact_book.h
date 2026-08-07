#ifndef CONTACT_BOOK_H
#define CONTACT_BOOK_H

#include <stddef.h>

#define MAX_NAME_LENGTH 64
#define MAX_WORK_LENGTH 128
#define MAX_ITEM_LENGTH 128
#define MAX_PHONES 5
#define MAX_EMAILS 5
#define MAX_SOCIAL_LINKS 5
#define MAX_MESSENGERS 5

typedef struct Contact {
    char last_name[MAX_NAME_LENGTH];
    char first_name[MAX_NAME_LENGTH];
    char middle_name[MAX_NAME_LENGTH];
    char workplace[MAX_WORK_LENGTH];
    char position[MAX_WORK_LENGTH];

    char phones[MAX_PHONES][MAX_ITEM_LENGTH];
    size_t phone_count;

    char emails[MAX_EMAILS][MAX_ITEM_LENGTH];
    size_t email_count;

    char social_links[MAX_SOCIAL_LINKS][MAX_ITEM_LENGTH];
    size_t social_link_count;

    char messengers[MAX_MESSENGERS][MAX_ITEM_LENGTH];
    size_t messenger_count;
} Contact;

/* Один узел двухсвязного списка. */
typedef struct ContactNode {
    Contact contact;
    struct ContactNode *prev;
    struct ContactNode *next;
} ContactNode;

/*
 * Телефонная книга хранит указатели на начало и конец списка.
 * count нужен для нумерации контактов и проверки индексов.
 */
typedef struct ContactBook {
    ContactNode *head;
    ContactNode *tail;
    size_t count;
} ContactBook;

typedef enum ContactBookResult {
    CONTACT_BOOK_OK = 0,
    CONTACT_BOOK_ERROR_INVALID_ARGUMENT,
    CONTACT_BOOK_ERROR_REQUIRED_FIELD,
    CONTACT_BOOK_ERROR_INVALID_CONTACT,
    CONTACT_BOOK_ERROR_OUT_OF_MEMORY,
    CONTACT_BOOK_ERROR_NOT_FOUND
} ContactBookResult;

void contact_init(Contact *contact);
void contact_book_init(ContactBook *book);
void contact_book_destroy(ContactBook *book);

int contact_is_valid(const Contact *contact);
int contact_compare(const Contact *left, const Contact *right);

ContactBookResult contact_book_add(ContactBook *book, const Contact *contact);
ContactBookResult contact_book_update(ContactBook *book,
                                      size_t index,
                                      const Contact *contact);
ContactBookResult contact_book_delete(ContactBook *book, size_t index);

const Contact *contact_book_get(const ContactBook *book, size_t index);
const ContactNode *contact_book_get_node(const ContactBook *book, size_t index);

int contact_matches_query(const Contact *contact, const char *query);
const char *contact_book_result_message(ContactBookResult result);

#endif
