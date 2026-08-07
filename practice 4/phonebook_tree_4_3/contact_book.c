#include "contact_book.h"

#include <stdlib.h>
#include <string.h>

static int has_text(const char *text)
{
    return text != NULL && text[0] != '\0';
}

static int compare_contacts(const Contact *left, const Contact *right)
{
    int result;

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

static int compare_key(const Contact *left_contact, size_t left_id,
                       const Contact *right_contact, size_t right_id)
{
    int result = compare_contacts(left_contact, right_contact);

    if (result != 0) {
        return result;
    }

    if (left_id < right_id) {
        return -1;
    }
    if (left_id > right_id) {
        return 1;
    }
    return 0;
}

static ContactNode *create_node(const Contact *contact, size_t id)
{
    ContactNode *node = malloc(sizeof(*node));

    if (node == NULL) {
        return NULL;
    }

    node->contact = *contact;
    node->id = id;
    node->left = NULL;
    node->right = NULL;
    return node;
}

static ContactNode *insert_existing_node(ContactNode *root, ContactNode *node)
{
    ContactNode *current;

    if (root == NULL) {
        return node;
    }

    current = root;
    for (;;) {
        if (compare_key(&node->contact, node->id,
                        &current->contact, current->id) < 0) {
            if (current->left == NULL) {
                current->left = node;
                break;
            }
            current = current->left;
        } else {
            if (current->right == NULL) {
                current->right = node;
                break;
            }
            current = current->right;
        }
    }

    return root;
}

static void free_subtree(ContactNode *node)
{
    if (node == NULL) {
        return;
    }

    free_subtree(node->left);
    free_subtree(node->right);
    free(node);
}

static const ContactNode *node_at_index(const ContactNode *node,
                                        size_t target_index,
                                        size_t *current_index)
{
    const ContactNode *found;

    if (node == NULL) {
        return NULL;
    }

    found = node_at_index(node->left, target_index, current_index);
    if (found != NULL) {
        return found;
    }

    if (*current_index == target_index) {
        return node;
    }
    (*current_index)++;

    return node_at_index(node->right, target_index, current_index);
}

static ContactNode *mutable_node_at_index(ContactNode *node,
                                          size_t target_index,
                                          size_t *current_index)
{
    ContactNode *found;

    if (node == NULL) {
        return NULL;
    }

    found = mutable_node_at_index(node->left, target_index, current_index);
    if (found != NULL) {
        return found;
    }

    if (*current_index == target_index) {
        return node;
    }
    (*current_index)++;

    return mutable_node_at_index(node->right, target_index, current_index);
}

static ContactNode *delete_exact_node(ContactNode *root,
                                      const Contact *contact,
                                      size_t id,
                                      int *deleted)
{
    int cmp;

    if (root == NULL) {
        return NULL;
    }

    cmp = compare_key(contact, id, &root->contact, root->id);
    if (cmp < 0) {
        root->left = delete_exact_node(root->left, contact, id, deleted);
        return root;
    }
    if (cmp > 0) {
        root->right = delete_exact_node(root->right, contact, id, deleted);
        return root;
    }

    *deleted = 1;

    if (root->left == NULL) {
        ContactNode *right = root->right;
        free(root);
        return right;
    }

    if (root->right == NULL) {
        ContactNode *left = root->left;
        free(root);
        return left;
    }

    {
        ContactNode *successor = root->right;
        Contact successor_contact;
        size_t successor_id;
        int successor_deleted = 0;

        while (successor->left != NULL) {
            successor = successor->left;
        }

        successor_contact = successor->contact;
        successor_id = successor->id;
        root->contact = successor_contact;
        root->id = successor_id;
        root->right = delete_exact_node(root->right,
                                        &successor_contact,
                                        successor_id,
                                        &successor_deleted);
    }

    return root;
}

static size_t subtree_height(const ContactNode *node)
{
    size_t left_height;
    size_t right_height;

    if (node == NULL) {
        return 0;
    }

    left_height = subtree_height(node->left);
    right_height = subtree_height(node->right);
    return 1 + (left_height > right_height ? left_height : right_height);
}

/*
 * Преобразование дерева в "лозу" (правую цепочку) для алгоритма DSW.
 * Возвращает количество узлов.
 */
static size_t tree_to_vine(ContactNode *pseudo_root)
{
    ContactNode *tail = pseudo_root;
    ContactNode *rest = tail->right;
    size_t count = 0;

    while (rest != NULL) {
        if (rest->left == NULL) {
            tail = rest;
            rest = rest->right;
            count++;
        } else {
            ContactNode *left = rest->left;
            rest->left = left->right;
            left->right = rest;
            rest = left;
            tail->right = left;
        }
    }

    return count;
}

static void compress_vine(ContactNode *pseudo_root, size_t count)
{
    ContactNode *scanner = pseudo_root;
    size_t i;

    for (i = 0; i < count; i++) {
        ContactNode *child = scanner->right;
        ContactNode *grandchild;

        if (child == NULL || child->right == NULL) {
            break;
        }

        grandchild = child->right;
        scanner->right = grandchild;
        child->right = grandchild->left;
        grandchild->left = child;
        scanner = grandchild;
    }
}

static int is_bst_recursive(const ContactNode *node,
                            const ContactNode *min_node,
                            const ContactNode *max_node,
                            size_t *visited)
{
    if (node == NULL) {
        return 1;
    }

    if (min_node != NULL &&
        compare_key(&node->contact, node->id,
                    &min_node->contact, min_node->id) <= 0) {
        return 0;
    }

    if (max_node != NULL &&
        compare_key(&node->contact, node->id,
                    &max_node->contact, max_node->id) >= 0) {
        return 0;
    }

    (*visited)++;

    return is_bst_recursive(node->left, min_node, node, visited) &&
           is_bst_recursive(node->right, node, max_node, visited);
}

static void note_successful_change(ContactBook *book)
{
    book->changes_since_balance++;

    if (book->changes_since_balance >= CONTACT_BOOK_BALANCE_INTERVAL) {
        contact_book_balance(book);
    }
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
        book->root = NULL;
        book->count = 0;
        book->next_id = 1;
        book->changes_since_balance = 0;
        book->balance_count = 0;
    }
}

void contact_book_clear(ContactBook *book)
{
    if (book == NULL) {
        return;
    }

    free_subtree(book->root);
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
    ContactNode *node;

    if (book == NULL || contact == NULL) {
        return CONTACT_BOOK_ERROR_INVALID_ARGUMENT;
    }

    if (!contact_is_valid(contact)) {
        return CONTACT_BOOK_ERROR_REQUIRED_FIELD;
    }

    node = create_node(contact, book->next_id);
    if (node == NULL) {
        return CONTACT_BOOK_ERROR_MEMORY;
    }

    book->next_id++;
    book->root = insert_existing_node(book->root, node);
    book->count++;
    note_successful_change(book);

    return CONTACT_BOOK_OK;
}

ContactBookResult contact_book_update(ContactBook *book, size_t index,
                                      const Contact *contact)
{
    ContactNode *old_node;
    Contact old_contact;
    size_t old_id;
    size_t current_index = 0;

    if (book == NULL || contact == NULL) {
        return CONTACT_BOOK_ERROR_INVALID_ARGUMENT;
    }

    if (index >= book->count) {
        return CONTACT_BOOK_ERROR_NOT_FOUND;
    }

    if (!contact_is_valid(contact)) {
        return CONTACT_BOOK_ERROR_REQUIRED_FIELD;
    }

    old_node = mutable_node_at_index(book->root, index, &current_index);
    if (old_node == NULL) {
        return CONTACT_BOOK_ERROR_NOT_FOUND;
    }

    old_contact = old_node->contact;
    old_id = old_node->id;

    /* Если ключ Ф.И.О. не изменился, перестраивать дерево не требуется. */
    if (compare_contacts(&old_contact, contact) == 0) {
        old_node->contact = *contact;
        return CONTACT_BOOK_OK;
    }

    {
        ContactNode *replacement = create_node(contact, old_id);
        int deleted = 0;

        if (replacement == NULL) {
            return CONTACT_BOOK_ERROR_MEMORY;
        }

        book->root = delete_exact_node(book->root, &old_contact, old_id,
                                       &deleted);
        if (!deleted) {
            free(replacement);
            return CONTACT_BOOK_ERROR_NOT_FOUND;
        }

        book->root = insert_existing_node(book->root, replacement);
    }

    note_successful_change(book);
    return CONTACT_BOOK_OK;
}

ContactBookResult contact_book_delete(ContactBook *book, size_t index)
{
    ContactNode *node;
    Contact contact;
    size_t id;
    size_t current_index = 0;
    int deleted = 0;

    if (book == NULL) {
        return CONTACT_BOOK_ERROR_INVALID_ARGUMENT;
    }

    if (index >= book->count) {
        return CONTACT_BOOK_ERROR_NOT_FOUND;
    }

    node = mutable_node_at_index(book->root, index, &current_index);
    if (node == NULL) {
        return CONTACT_BOOK_ERROR_NOT_FOUND;
    }

    contact = node->contact;
    id = node->id;
    book->root = delete_exact_node(book->root, &contact, id, &deleted);

    if (!deleted) {
        return CONTACT_BOOK_ERROR_NOT_FOUND;
    }

    book->count--;
    note_successful_change(book);
    return CONTACT_BOOK_OK;
}

const Contact *contact_book_get(const ContactBook *book, size_t index)
{
    size_t current_index = 0;
    const ContactNode *node;

    if (book == NULL || index >= book->count) {
        return NULL;
    }

    node = node_at_index(book->root, index, &current_index);
    return node != NULL ? &node->contact : NULL;
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

size_t contact_book_height(const ContactBook *book)
{
    if (book == NULL) {
        return 0;
    }

    return subtree_height(book->root);
}

void contact_book_balance(ContactBook *book)
{
    ContactNode pseudo_root;
    size_t node_count;
    size_t m;

    if (book == NULL) {
        return;
    }

    if (book->root == NULL) {
        book->changes_since_balance = 0;
        return;
    }

    memset(&pseudo_root, 0, sizeof(pseudo_root));
    pseudo_root.right = book->root;

    node_count = tree_to_vine(&pseudo_root);

    /* m = 2^floor(log2(n + 1)) - 1 */
    m = 1;
    while (m <= node_count + 1) {
        m <<= 1;
    }
    m = (m >> 1) - 1;

    compress_vine(&pseudo_root, node_count - m);
    while (m > 1) {
        m >>= 1;
        compress_vine(&pseudo_root, m);
    }

    book->root = pseudo_root.right;
    book->changes_since_balance = 0;
    book->balance_count++;
}

int contact_book_is_bst(const ContactBook *book)
{
    size_t visited = 0;

    if (book == NULL) {
        return 0;
    }

    if (!is_bst_recursive(book->root, NULL, NULL, &visited)) {
        return 0;
    }

    return visited == book->count;
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
        case CONTACT_BOOK_ERROR_NOT_FOUND:
            return "Контакт не найден.";
        case CONTACT_BOOK_ERROR_MEMORY:
            return "Не удалось выделить память для контакта.";
        default:
            return "Неизвестная ошибка.";
    }
}
