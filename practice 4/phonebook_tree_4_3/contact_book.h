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

/* После каждых 5 структурных изменений дерево автоматически балансируется. */
#define CONTACT_BOOK_BALANCE_INTERVAL 5

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

typedef struct ContactNode {
    Contact contact;
    size_t id;
    struct ContactNode *left;
    struct ContactNode *right;
} ContactNode;

typedef struct ContactBook {
    ContactNode *root;
    size_t count;
    size_t next_id;
    size_t changes_since_balance;
    size_t balance_count;
} ContactBook;

typedef enum ContactBookResult {
    CONTACT_BOOK_OK = 0,
    CONTACT_BOOK_ERROR_INVALID_ARGUMENT,
    CONTACT_BOOK_ERROR_REQUIRED_FIELD,
    CONTACT_BOOK_ERROR_NOT_FOUND,
    CONTACT_BOOK_ERROR_MEMORY
} ContactBookResult;

void contact_init(Contact *contact);
void contact_book_init(ContactBook *book);
void contact_book_clear(ContactBook *book);
int contact_is_valid(const Contact *contact);

ContactBookResult contact_book_add(ContactBook *book, const Contact *contact);
ContactBookResult contact_book_update(ContactBook *book, size_t index,
                                      const Contact *contact);
ContactBookResult contact_book_delete(ContactBook *book, size_t index);

const Contact *contact_book_get(const ContactBook *book, size_t index);
int contact_matches_query(const Contact *contact, const char *query);

/* Функции для демонстрации и проверки бинарного дерева. */
size_t contact_book_height(const ContactBook *book);
void contact_book_balance(ContactBook *book);
int contact_book_is_bst(const ContactBook *book);

const char *contact_book_result_message(ContactBookResult result);

#endif
