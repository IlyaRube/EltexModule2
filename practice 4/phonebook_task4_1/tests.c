#include "contact_book.h"
#include "storage.h"

#include <locale.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

static int tests_run = 0;
static int tests_passed = 0;

#define EXPECT_TRUE(condition)                                                     \
    do {                                                                           \
        tests_run++;                                                               \
        if (condition) {                                                           \
            tests_passed++;                                                        \
            printf("[PASS] %s\n", #condition);                                  \
        } else {                                                                   \
            printf("[FAIL] %s (%s:%d)\n", #condition, __FILE__, __LINE__);      \
        }                                                                          \
    } while (0)

#define EXPECT_SIZE_EQ(expected, actual)                                           \
    do {                                                                           \
        size_t expected_value = (size_t)(expected);                                \
        size_t actual_value = (size_t)(actual);                                    \
        tests_run++;                                                               \
        if (expected_value == actual_value) {                                      \
            tests_passed++;                                                        \
            printf("[PASS] %s == %s\n", #expected, #actual);                    \
        } else {                                                                   \
            printf("[FAIL] %s == %s: ожидалось %zu, получено %zu (%s:%d)\n",   \
                   #expected, #actual, expected_value, actual_value,               \
                   __FILE__, __LINE__);                                            \
        }                                                                          \
    } while (0)

#define EXPECT_STRING_EQ(expected, actual)                                         \
    do {                                                                           \
        const char *expected_value = (expected);                                   \
        const char *actual_value = (actual);                                       \
        tests_run++;                                                               \
        if (actual_value != NULL && strcmp(expected_value, actual_value) == 0) {    \
            tests_passed++;                                                        \
            printf("[PASS] \"%s\" == \"%s\"\n",                           \
                   expected_value, actual_value);                                  \
        } else {                                                                   \
            printf("[FAIL] строки различаются: ожидалось \"%s\", "            \
                   "получено \"%s\" (%s:%d)\n",                              \
                   expected_value,                                                 \
                   actual_value != NULL ? actual_value : "NULL",                 \
                   __FILE__, __LINE__);                                            \
        }                                                                          \
    } while (0)

static Contact make_contact(const char *last_name, const char *first_name)
{
    Contact contact;

    contact_init(&contact);
    snprintf(contact.last_name, sizeof(contact.last_name), "%s", last_name);
    snprintf(contact.first_name, sizeof(contact.first_name), "%s", first_name);
    return contact;
}

static int links_are_consistent(const ContactBook *book)
{
    const ContactNode *node;
    const ContactNode *previous = NULL;
    size_t forward_count = 0U;
    size_t backward_count = 0U;

    if (book == NULL) {
        return 0;
    }

    if (book->count == 0U) {
        return book->head == NULL && book->tail == NULL;
    }

    if (book->head == NULL || book->tail == NULL ||
        book->head->prev != NULL || book->tail->next != NULL) {
        return 0;
    }

    for (node = book->head; node != NULL; node = node->next) {
        if (node->prev != previous) {
            return 0;
        }
        previous = node;
        forward_count++;
    }

    if (previous != book->tail) {
        return 0;
    }

    previous = NULL;
    for (node = book->tail; node != NULL; node = node->prev) {
        if (node->next != previous) {
            return 0;
        }
        previous = node;
        backward_count++;
    }

    return previous == book->head &&
           forward_count == book->count &&
           backward_count == book->count;
}

static int list_is_sorted(const ContactBook *book)
{
    const ContactNode *node;

    if (book == NULL) {
        return 0;
    }

    for (node = book->head; node != NULL && node->next != NULL;
         node = node->next) {
        if (contact_compare(&node->contact, &node->next->contact) > 0) {
            return 0;
        }
    }

    return 1;
}

static void test_initialization(void)
{
    ContactBook book;

    contact_book_init(&book);
    EXPECT_SIZE_EQ(0, book.count);
    EXPECT_TRUE(book.head == NULL);
    EXPECT_TRUE(book.tail == NULL);
}

static void test_sorted_insertion_and_links(void)
{
    ContactBook book;
    Contact petrov = make_contact("Петров", "Пётр");
    Contact andreev = make_contact("Андреев", "Андрей");
    Contact vasilev = make_contact("Васильев", "Василий");

    contact_book_init(&book);

    EXPECT_TRUE(contact_book_add(&book, &petrov) == CONTACT_BOOK_OK);
    EXPECT_TRUE(contact_book_add(&book, &andreev) == CONTACT_BOOK_OK);
    EXPECT_TRUE(contact_book_add(&book, &vasilev) == CONTACT_BOOK_OK);
    EXPECT_SIZE_EQ(3, book.count);

    EXPECT_STRING_EQ("Андреев", book.head->contact.last_name);
    EXPECT_STRING_EQ("Васильев", book.head->next->contact.last_name);
    EXPECT_STRING_EQ("Петров", book.tail->contact.last_name);

    EXPECT_TRUE(book.head->prev == NULL);
    EXPECT_TRUE(book.tail->next == NULL);
    EXPECT_TRUE(book.head->next->prev == book.head);
    EXPECT_TRUE(book.tail->prev == book.head->next);
    EXPECT_TRUE(links_are_consistent(&book));
    EXPECT_TRUE(list_is_sorted(&book));

    contact_book_destroy(&book);
}

static void test_sorting_by_full_name(void)
{
    ContactBook book;
    Contact third = make_contact("Иванов", "Пётр");
    Contact first = make_contact("Иванов", "Алексей");
    Contact second = make_contact("Иванов", "Алексей");

    snprintf(first.middle_name, sizeof(first.middle_name), "%s", "Андреевич");
    snprintf(second.middle_name, sizeof(second.middle_name), "%s", "Борисович");

    contact_book_init(&book);
    contact_book_add(&book, &third);
    contact_book_add(&book, &second);
    contact_book_add(&book, &first);

    EXPECT_STRING_EQ("Алексей", book.head->contact.first_name);
    EXPECT_STRING_EQ("Андреевич", book.head->contact.middle_name);
    EXPECT_STRING_EQ("Борисович", book.head->next->contact.middle_name);
    EXPECT_STRING_EQ("Пётр", book.tail->contact.first_name);
    EXPECT_TRUE(list_is_sorted(&book));

    contact_book_destroy(&book);
}

static void test_required_fields(void)
{
    ContactBook book;
    Contact no_last_name = make_contact("", "Иван");
    Contact no_first_name = make_contact("Иванов", "");

    contact_book_init(&book);
    EXPECT_TRUE(contact_book_add(&book, &no_last_name) ==
                CONTACT_BOOK_ERROR_REQUIRED_FIELD);
    EXPECT_TRUE(contact_book_add(&book, &no_first_name) ==
                CONTACT_BOOK_ERROR_REQUIRED_FIELD);
    EXPECT_SIZE_EQ(0, book.count);
    EXPECT_TRUE(links_are_consistent(&book));
}

static void test_invalid_contact_lists(void)
{
    ContactBook book;
    Contact invalid_count = make_contact("Кузнецов", "Кузьма");
    Contact empty_phone = make_contact("Смирнов", "Семён");

    invalid_count.phone_count = MAX_PHONES + 1U;
    empty_phone.phone_count = 1U;
    empty_phone.phones[0][0] = '\0';

    contact_book_init(&book);
    EXPECT_TRUE(contact_is_valid(&invalid_count) == 0);
    EXPECT_TRUE(contact_is_valid(&empty_phone) == 0);
    EXPECT_TRUE(contact_book_add(&book, &invalid_count) ==
                CONTACT_BOOK_ERROR_INVALID_CONTACT);
    EXPECT_TRUE(contact_book_add(&book, &empty_phone) ==
                CONTACT_BOOK_ERROR_INVALID_CONTACT);
    EXPECT_SIZE_EQ(0, book.count);
}

static void test_get_by_index(void)
{
    ContactBook book;
    Contact a = make_contact("Андреев", "Андрей");
    Contact b = make_contact("Борисов", "Борис");
    Contact c = make_contact("Васильев", "Василий");
    Contact d = make_contact("Громов", "Григорий");

    contact_book_init(&book);
    contact_book_add(&book, &d);
    contact_book_add(&book, &b);
    contact_book_add(&book, &a);
    contact_book_add(&book, &c);

    EXPECT_STRING_EQ("Андреев", contact_book_get(&book, 0)->last_name);
    EXPECT_STRING_EQ("Борисов", contact_book_get(&book, 1)->last_name);
    EXPECT_STRING_EQ("Васильев", contact_book_get(&book, 2)->last_name);
    EXPECT_STRING_EQ("Громов", contact_book_get(&book, 3)->last_name);
    EXPECT_TRUE(contact_book_get(&book, 4) == NULL);

    contact_book_destroy(&book);
}

static void test_update_and_reorder(void)
{
    ContactBook book;
    Contact andreev = make_contact("Андреев", "Андрей");
    Contact petrov = make_contact("Петров", "Пётр");
    Contact sidorov = make_contact("Сидоров", "Сидор");
    Contact edited;

    contact_book_init(&book);
    contact_book_add(&book, &petrov);
    contact_book_add(&book, &sidorov);
    contact_book_add(&book, &andreev);

    edited = *contact_book_get(&book, 1);
    snprintf(edited.last_name, sizeof(edited.last_name), "%s", "Яковлев");
    snprintf(edited.position, sizeof(edited.position), "%s", "Инженер");

    EXPECT_TRUE(contact_book_update(&book, 1, &edited) == CONTACT_BOOK_OK);
    EXPECT_SIZE_EQ(3, book.count);
    EXPECT_STRING_EQ("Андреев", book.head->contact.last_name);
    EXPECT_STRING_EQ("Сидоров", book.head->next->contact.last_name);
    EXPECT_STRING_EQ("Яковлев", book.tail->contact.last_name);
    EXPECT_STRING_EQ("Инженер", book.tail->contact.position);
    EXPECT_TRUE(links_are_consistent(&book));
    EXPECT_TRUE(list_is_sorted(&book));

    contact_book_destroy(&book);
}

static void test_delete_head_middle_tail(void)
{
    ContactBook book;
    Contact a = make_contact("Андреев", "Андрей");
    Contact b = make_contact("Борисов", "Борис");
    Contact c = make_contact("Васильев", "Василий");
    Contact d = make_contact("Громов", "Григорий");

    contact_book_init(&book);
    contact_book_add(&book, &d);
    contact_book_add(&book, &b);
    contact_book_add(&book, &a);
    contact_book_add(&book, &c);

    EXPECT_TRUE(contact_book_delete(&book, 0) == CONTACT_BOOK_OK);
    EXPECT_SIZE_EQ(3, book.count);
    EXPECT_STRING_EQ("Борисов", book.head->contact.last_name);
    EXPECT_TRUE(book.head->prev == NULL);
    EXPECT_TRUE(links_are_consistent(&book));

    EXPECT_TRUE(contact_book_delete(&book, 1) == CONTACT_BOOK_OK);
    EXPECT_SIZE_EQ(2, book.count);
    EXPECT_STRING_EQ("Борисов", book.head->contact.last_name);
    EXPECT_STRING_EQ("Громов", book.tail->contact.last_name);
    EXPECT_TRUE(links_are_consistent(&book));

    EXPECT_TRUE(contact_book_delete(&book, 1) == CONTACT_BOOK_OK);
    EXPECT_SIZE_EQ(1, book.count);
    EXPECT_TRUE(book.head == book.tail);
    EXPECT_STRING_EQ("Борисов", book.head->contact.last_name);
    EXPECT_TRUE(links_are_consistent(&book));

    EXPECT_TRUE(contact_book_delete(&book, 0) == CONTACT_BOOK_OK);
    EXPECT_SIZE_EQ(0, book.count);
    EXPECT_TRUE(book.head == NULL && book.tail == NULL);

    contact_book_destroy(&book);
}

static void test_invalid_index(void)
{
    ContactBook book;
    Contact contact = make_contact("Сидоров", "Сидор");

    contact_book_init(&book);
    EXPECT_TRUE(contact_book_update(&book, 0, &contact) ==
                CONTACT_BOOK_ERROR_NOT_FOUND);
    EXPECT_TRUE(contact_book_delete(&book, 0) ==
                CONTACT_BOOK_ERROR_NOT_FOUND);
    EXPECT_TRUE(contact_book_get(&book, 0) == NULL);
}

static void test_search(void)
{
    Contact contact = make_contact("Смирнов", "Алексей");

    snprintf(contact.middle_name, sizeof(contact.middle_name), "%s", "Олегович");

    EXPECT_TRUE(contact_matches_query(&contact, "Смир") == 1);
    EXPECT_TRUE(contact_matches_query(&contact, "Алекс") == 1);
    EXPECT_TRUE(contact_matches_query(&contact, "Олег") == 1);
    EXPECT_TRUE(contact_matches_query(&contact, "Петров") == 0);
    EXPECT_TRUE(contact_matches_query(&contact, "") == 0);
}

static void test_reverse_traversal(void)
{
    ContactBook book;
    Contact a = make_contact("Андреев", "Андрей");
    Contact b = make_contact("Борисов", "Борис");
    Contact c = make_contact("Васильев", "Василий");
    const ContactNode *node;

    contact_book_init(&book);
    contact_book_add(&book, &b);
    contact_book_add(&book, &c);
    contact_book_add(&book, &a);

    node = book.tail;
    EXPECT_STRING_EQ("Васильев", node->contact.last_name);
    node = node->prev;
    EXPECT_STRING_EQ("Борисов", node->contact.last_name);
    node = node->prev;
    EXPECT_STRING_EQ("Андреев", node->contact.last_name);
    EXPECT_TRUE(node->prev == NULL);
    EXPECT_TRUE(links_are_consistent(&book));

    contact_book_destroy(&book);
}

static void test_destroy(void)
{
    ContactBook book;
    Contact a = make_contact("Андреев", "Андрей");
    Contact b = make_contact("Борисов", "Борис");

    contact_book_init(&book);
    contact_book_add(&book, &a);
    contact_book_add(&book, &b);
    contact_book_destroy(&book);

    EXPECT_SIZE_EQ(0, book.count);
    EXPECT_TRUE(book.head == NULL);
    EXPECT_TRUE(book.tail == NULL);
}

#define TEST_STORAGE_FILE "test_contacts_storage.txt"
#define TEST_INVALID_FILE "test_contacts_invalid.txt"

static void remove_storage_test_files(void)
{
    remove(TEST_STORAGE_FILE);
    remove("test_contacts_storage.txt.tmp");
    remove(TEST_INVALID_FILE);
    remove("test_contacts_invalid.txt.tmp");
}

static void test_storage_save_and_load(void)
{
    ContactBook source;
    ContactBook loaded;
    Contact petrov = make_contact("Петров", "Пётр");
    Contact ivanov = make_contact("Иванов", "Иван");

    remove_storage_test_files();
    contact_book_init(&source);
    contact_book_init(&loaded);

    snprintf(ivanov.middle_name, sizeof(ivanov.middle_name), "%s", "Иванович");
    snprintf(ivanov.workplace, sizeof(ivanov.workplace), "%s", "Яндекс");
    snprintf(ivanov.position, sizeof(ivanov.position), "%s", "Курьер");
    snprintf(ivanov.phones[0], sizeof(ivanov.phones[0]), "%s",
             "+7-900-111-22-33");
    ivanov.phone_count = 1U;
    snprintf(ivanov.emails[0], sizeof(ivanov.emails[0]), "%s",
             "ivan@example.com");
    ivanov.email_count = 1U;
    snprintf(ivanov.social_links[0], sizeof(ivanov.social_links[0]), "%s",
             "vk.com/ivan");
    ivanov.social_link_count = 1U;
    snprintf(ivanov.messengers[0], sizeof(ivanov.messengers[0]), "%s",
             "t.me/ivan");
    ivanov.messenger_count = 1U;

    contact_book_add(&source, &petrov);
    contact_book_add(&source, &ivanov);

    EXPECT_TRUE(storage_save(&source, TEST_STORAGE_FILE) == STORAGE_OK);
    EXPECT_TRUE(storage_load(&loaded, TEST_STORAGE_FILE) == STORAGE_OK);
    EXPECT_SIZE_EQ(2, loaded.count);
    EXPECT_STRING_EQ("Иванов", loaded.head->contact.last_name);
    EXPECT_STRING_EQ("Петров", loaded.tail->contact.last_name);
    EXPECT_STRING_EQ("Иванович", loaded.head->contact.middle_name);
    EXPECT_STRING_EQ("Яндекс", loaded.head->contact.workplace);
    EXPECT_STRING_EQ("Курьер", loaded.head->contact.position);
    EXPECT_STRING_EQ("+7-900-111-22-33", loaded.head->contact.phones[0]);
    EXPECT_STRING_EQ("ivan@example.com", loaded.head->contact.emails[0]);
    EXPECT_STRING_EQ("vk.com/ivan", loaded.head->contact.social_links[0]);
    EXPECT_STRING_EQ("t.me/ivan", loaded.head->contact.messengers[0]);
    EXPECT_TRUE(links_are_consistent(&loaded));
    EXPECT_TRUE(list_is_sorted(&loaded));

    contact_book_destroy(&source);
    contact_book_destroy(&loaded);
    remove_storage_test_files();
}

static void test_storage_missing_file_keeps_book(void)
{
    ContactBook book;
    Contact contact = make_contact("Сохранов", "Семён");

    remove_storage_test_files();
    contact_book_init(&book);
    contact_book_add(&book, &contact);

    EXPECT_TRUE(storage_load(&book, TEST_STORAGE_FILE) ==
                STORAGE_ERROR_NOT_FOUND);
    EXPECT_SIZE_EQ(1, book.count);
    EXPECT_STRING_EQ("Сохранов", book.head->contact.last_name);
    EXPECT_TRUE(links_are_consistent(&book));

    contact_book_destroy(&book);
}

static void test_storage_invalid_format_keeps_book(void)
{
    ContactBook book;
    Contact contact = make_contact("Старов", "Степан");
    FILE *file;

    remove_storage_test_files();
    file = fopen(TEST_INVALID_FILE, "wb");
    EXPECT_TRUE(file != NULL);
    if (file == NULL) {
        return;
    }

    fputs("BROKEN_FORMAT\n1\n", file);
    fclose(file);

    contact_book_init(&book);
    contact_book_add(&book, &contact);

    EXPECT_TRUE(storage_load(&book, TEST_INVALID_FILE) ==
                STORAGE_ERROR_FORMAT);
    EXPECT_SIZE_EQ(1, book.count);
    EXPECT_STRING_EQ("Старов", book.head->contact.last_name);
    EXPECT_TRUE(links_are_consistent(&book));

    contact_book_destroy(&book);
    remove_storage_test_files();
}

static void configure_console(void)
{
#ifdef _WIN32
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    setlocale(LC_ALL, "Russian_Russia.1251");
#else
    setlocale(LC_ALL, "");
#endif
}

int main(void)
{
    configure_console();

    printf("Запуск автотестов двухсвязной телефонной книги...\n\n");

    test_initialization();
    test_sorted_insertion_and_links();
    test_sorting_by_full_name();
    test_required_fields();
    test_invalid_contact_lists();
    test_get_by_index();
    test_update_and_reorder();
    test_delete_head_middle_tail();
    test_invalid_index();
    test_search();
    test_reverse_traversal();
    test_destroy();
    test_storage_save_and_load();
    test_storage_missing_file_keeps_book();
    test_storage_invalid_format_keeps_book();

    printf("\nРезультат: %d из %d проверок пройдено.\n",
           tests_passed, tests_run);

    if (tests_passed == tests_run) {
        printf("Все автотесты пройдены.\n");
        return 0;
    }

    printf("Есть непройденные автотесты.\n");
    return 1;
}
