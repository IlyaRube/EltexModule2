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
            printf("[PASS] %s\n", #condition);                                   \
        } else {                                                                   \
            printf("[FAIL] %s (%s:%d)\n", #condition, __FILE__, __LINE__);        \
        }                                                                          \
    } while (0)

#define EXPECT_SIZE_EQ(expected, actual)                                           \
    do {                                                                           \
        size_t expected_value = (size_t)(expected);                                \
        size_t actual_value = (size_t)(actual);                                    \
        tests_run++;                                                               \
        if (expected_value == actual_value) {                                      \
            tests_passed++;                                                        \
            printf("[PASS] %s == %s == %zu\n",                                  \
                   #expected, #actual, actual_value);                              \
        } else {                                                                   \
            printf("[FAIL] %s == %s: ожидалось %zu, получено %zu (%s:%d)\n",    \
                   #expected, #actual, expected_value, actual_value,               \
                   __FILE__, __LINE__);                                            \
        }                                                                          \
    } while (0)

#define EXPECT_STRING_EQ(expected, actual)                                         \
    do {                                                                           \
        const char *expected_value = (expected);                                   \
        const char *actual_value = (actual);                                       \
        tests_run++;                                                               \
        if (actual_value != NULL && strcmp(expected_value, actual_value) == 0) {   \
            tests_passed++;                                                        \
            printf("[PASS] \"%s\" == \"%s\"\n", expected_value, actual_value); \
        } else {                                                                   \
            printf("[FAIL] строки различаются: ожидалось \"%s\", получено \"%s\" " \
                   "(%s:%d)\n",                                                  \
                   expected_value,                                                 \
                   actual_value != NULL ? actual_value : "(null)",               \
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

static const Contact *get_contact(const ContactBook *book, size_t index)
{
    return contact_book_get(book, index);
}

static void test_initialization(void)
{
    ContactBook book;

    contact_book_init(&book);
    EXPECT_TRUE(book.root == NULL);
    EXPECT_SIZE_EQ(0, book.count);
    EXPECT_SIZE_EQ(0, contact_book_height(&book));
    EXPECT_TRUE(contact_book_is_bst(&book) == 1);
    contact_book_clear(&book);
}

static void test_add_and_sorted_inorder(void)
{
    ContactBook book;
    Contact third = make_contact("Васильев", "Василий");
    Contact first = make_contact("Андреев", "Андрей");
    Contact second = make_contact("Борисов", "Борис");

    contact_book_init(&book);
    EXPECT_TRUE(contact_book_add(&book, &third) == CONTACT_BOOK_OK);
    EXPECT_TRUE(contact_book_add(&book, &first) == CONTACT_BOOK_OK);
    EXPECT_TRUE(contact_book_add(&book, &second) == CONTACT_BOOK_OK);

    EXPECT_SIZE_EQ(3, book.count);
    EXPECT_STRING_EQ("Андреев", get_contact(&book, 0)->last_name);
    EXPECT_STRING_EQ("Борисов", get_contact(&book, 1)->last_name);
    EXPECT_STRING_EQ("Васильев", get_contact(&book, 2)->last_name);
    EXPECT_TRUE(contact_book_is_bst(&book) == 1);

    contact_book_clear(&book);
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
    contact_book_clear(&book);
}

static void test_edit_without_key_change(void)
{
    ContactBook book;
    Contact original = make_contact("Петров", "Пётр");
    Contact edited;

    contact_book_init(&book);
    EXPECT_TRUE(contact_book_add(&book, &original) == CONTACT_BOOK_OK);

    edited = original;
    snprintf(edited.position, sizeof(edited.position), "%s", "Инженер");
    snprintf(edited.emails[0], sizeof(edited.emails[0]), "%s",
             "petr@example.com");
    edited.email_count = 1;

    EXPECT_TRUE(contact_book_update(&book, 0, &edited) == CONTACT_BOOK_OK);
    EXPECT_SIZE_EQ(1, book.changes_since_balance);
    EXPECT_STRING_EQ("Инженер", get_contact(&book, 0)->position);
    EXPECT_STRING_EQ("petr@example.com", get_contact(&book, 0)->emails[0]);
    EXPECT_TRUE(contact_book_is_bst(&book) == 1);

    contact_book_clear(&book);
}

static void test_edit_key_repositions_node(void)
{
    ContactBook book;
    Contact a = make_contact("Андреев", "Андрей");
    Contact b = make_contact("Борисов", "Борис");
    Contact c = make_contact("Васильев", "Василий");
    Contact edited;

    contact_book_init(&book);
    contact_book_add(&book, &a);
    contact_book_add(&book, &b);
    contact_book_add(&book, &c);

    edited = *get_contact(&book, 1);
    snprintf(edited.last_name, sizeof(edited.last_name), "%s", "Яковлев");

    EXPECT_TRUE(contact_book_update(&book, 1, &edited) == CONTACT_BOOK_OK);
    EXPECT_STRING_EQ("Андреев", get_contact(&book, 0)->last_name);
    EXPECT_STRING_EQ("Васильев", get_contact(&book, 1)->last_name);
    EXPECT_STRING_EQ("Яковлев", get_contact(&book, 2)->last_name);
    EXPECT_TRUE(contact_book_is_bst(&book) == 1);

    contact_book_clear(&book);
}

static void test_duplicate_names_are_supported(void)
{
    ContactBook book;
    Contact first = make_contact("Иванов", "Иван");
    Contact second = make_contact("Иванов", "Иван");

    snprintf(first.position, sizeof(first.position), "%s", "Инженер");
    snprintf(second.position, sizeof(second.position), "%s", "Тестировщик");

    contact_book_init(&book);
    EXPECT_TRUE(contact_book_add(&book, &first) == CONTACT_BOOK_OK);
    EXPECT_TRUE(contact_book_add(&book, &second) == CONTACT_BOOK_OK);
    EXPECT_SIZE_EQ(2, book.count);
    EXPECT_STRING_EQ("Инженер", get_contact(&book, 0)->position);
    EXPECT_STRING_EQ("Тестировщик", get_contact(&book, 1)->position);

    EXPECT_TRUE(contact_book_delete(&book, 0) == CONTACT_BOOK_OK);
    EXPECT_SIZE_EQ(1, book.count);
    EXPECT_STRING_EQ("Тестировщик", get_contact(&book, 0)->position);
    EXPECT_TRUE(contact_book_is_bst(&book) == 1);

    contact_book_clear(&book);
}

static void test_delete_node_with_two_children(void)
{
    ContactBook book;
    const char *names[] = {"D", "B", "F", "A", "C", "E", "G"};
    size_t i;

    contact_book_init(&book);
    for (i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
        Contact contact = make_contact(names[i], "Test");
        EXPECT_TRUE(contact_book_add(&book, &contact) == CONTACT_BOOK_OK);
    }

    contact_book_balance(&book);
    EXPECT_TRUE(book.root != NULL);
    EXPECT_STRING_EQ("D", book.root->contact.last_name);
    EXPECT_TRUE(book.root->left != NULL && book.root->right != NULL);

    EXPECT_TRUE(contact_book_delete(&book, 3) == CONTACT_BOOK_OK);
    EXPECT_SIZE_EQ(6, book.count);
    EXPECT_STRING_EQ("A", get_contact(&book, 0)->last_name);
    EXPECT_STRING_EQ("B", get_contact(&book, 1)->last_name);
    EXPECT_STRING_EQ("C", get_contact(&book, 2)->last_name);
    EXPECT_STRING_EQ("E", get_contact(&book, 3)->last_name);
    EXPECT_STRING_EQ("F", get_contact(&book, 4)->last_name);
    EXPECT_STRING_EQ("G", get_contact(&book, 5)->last_name);
    EXPECT_TRUE(contact_book_is_bst(&book) == 1);

    contact_book_clear(&book);
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
    contact_book_clear(&book);
}

static void test_periodic_balancing(void)
{
    ContactBook book;
    const char *names[] = {"A", "B", "C", "D", "E"};
    size_t i;

    contact_book_init(&book);

    for (i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
        Contact contact = make_contact(names[i], "Test");
        EXPECT_TRUE(contact_book_add(&book, &contact) == CONTACT_BOOK_OK);
    }

    EXPECT_SIZE_EQ(CONTACT_BOOK_BALANCE_INTERVAL, book.count);
    EXPECT_SIZE_EQ(1, book.balance_count);
    EXPECT_SIZE_EQ(0, book.changes_since_balance);
    EXPECT_SIZE_EQ(3, contact_book_height(&book));
    EXPECT_TRUE(contact_book_is_bst(&book) == 1);

    contact_book_clear(&book);
}

static void test_manual_balancing_reduces_height(void)
{
    ContactBook book;
    const char *names[] = {"A", "B", "C", "D", "E", "F", "G", "H", "I"};
    size_t i;
    size_t height_before;
    size_t height_after;

    contact_book_init(&book);

    for (i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
        Contact contact = make_contact(names[i], "Test");
        contact_book_add(&book, &contact);
    }

    height_before = contact_book_height(&book);
    contact_book_balance(&book);
    height_after = contact_book_height(&book);

    EXPECT_TRUE(height_after <= height_before);
    EXPECT_SIZE_EQ(4, height_after);
    EXPECT_TRUE(contact_book_is_bst(&book) == 1);

    contact_book_clear(&book);
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

static void test_list_count_validation(void)
{
    Contact contact = make_contact("Кузнецов", "Кузьма");

    contact.phone_count = MAX_PHONES + 1;
    EXPECT_TRUE(contact_is_valid(&contact) == 0);
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
    Contact first = make_contact("Петров", "Пётр");
    Contact second = make_contact("Иванов", "Иван");

    remove_storage_test_files();
    contact_book_init(&source);
    contact_book_init(&loaded);

    snprintf(second.middle_name, sizeof(second.middle_name), "%s", "Иванович");
    snprintf(second.workplace, sizeof(second.workplace), "%s", "Яндекс");
    snprintf(second.position, sizeof(second.position), "%s", "Курьер");
    snprintf(second.phones[0], sizeof(second.phones[0]), "%s", "+7-900-111-22-33");
    second.phone_count = 1;
    snprintf(second.emails[0], sizeof(second.emails[0]), "%s", "ivan@example.com");
    second.email_count = 1;
    snprintf(second.social_links[0], sizeof(second.social_links[0]), "%s", "vk.com/ivan");
    second.social_link_count = 1;
    snprintf(second.messengers[0], sizeof(second.messengers[0]), "%s", "t.me/ivan");
    second.messenger_count = 1;

    snprintf(first.phones[0], sizeof(first.phones[0]), "%s", "+7-901-444-55-66");
    first.phone_count = 1;

    contact_book_add(&source, &first);
    contact_book_add(&source, &second);

    EXPECT_TRUE(storage_save(&source, TEST_STORAGE_FILE) == STORAGE_OK);
    EXPECT_TRUE(storage_load(&loaded, TEST_STORAGE_FILE) == STORAGE_OK);
    EXPECT_SIZE_EQ(2, loaded.count);
    EXPECT_STRING_EQ("Иванов", get_contact(&loaded, 0)->last_name);
    EXPECT_STRING_EQ("Иванович", get_contact(&loaded, 0)->middle_name);
    EXPECT_STRING_EQ("Яндекс", get_contact(&loaded, 0)->workplace);
    EXPECT_STRING_EQ("+7-900-111-22-33", get_contact(&loaded, 0)->phones[0]);
    EXPECT_STRING_EQ("ivan@example.com", get_contact(&loaded, 0)->emails[0]);
    EXPECT_STRING_EQ("vk.com/ivan", get_contact(&loaded, 0)->social_links[0]);
    EXPECT_STRING_EQ("t.me/ivan", get_contact(&loaded, 0)->messengers[0]);
    EXPECT_STRING_EQ("Петров", get_contact(&loaded, 1)->last_name);
    EXPECT_TRUE(contact_book_is_bst(&loaded) == 1);

    contact_book_clear(&source);
    contact_book_clear(&loaded);
    remove_storage_test_files();
}

static void test_storage_missing_file_preserves_book(void)
{
    ContactBook book;
    Contact contact = make_contact("Сохранов", "Семён");

    remove_storage_test_files();
    contact_book_init(&book);
    contact_book_add(&book, &contact);

    EXPECT_TRUE(storage_load(&book, TEST_STORAGE_FILE) == STORAGE_ERROR_NOT_FOUND);
    EXPECT_SIZE_EQ(1, book.count);
    EXPECT_STRING_EQ("Сохранов", get_contact(&book, 0)->last_name);

    contact_book_clear(&book);
}

static void test_storage_invalid_format_preserves_book(void)
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

    EXPECT_TRUE(storage_load(&book, TEST_INVALID_FILE) == STORAGE_ERROR_FORMAT);
    EXPECT_SIZE_EQ(1, book.count);
    EXPECT_STRING_EQ("Старов", get_contact(&book, 0)->last_name);

    contact_book_clear(&book);
    remove_storage_test_files();
}

static void test_clear_frees_tree_state(void)
{
    ContactBook book;
    Contact a = make_contact("A", "Test");
    Contact b = make_contact("B", "Test");

    contact_book_init(&book);
    contact_book_add(&book, &a);
    contact_book_add(&book, &b);
    contact_book_clear(&book);

    EXPECT_TRUE(book.root == NULL);
    EXPECT_SIZE_EQ(0, book.count);
    EXPECT_SIZE_EQ(0, book.changes_since_balance);
    EXPECT_SIZE_EQ(0, book.balance_count);
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

    printf("Запуск автотестов телефонной книги на бинарном дереве...\n\n");

    test_initialization();
    test_add_and_sorted_inorder();
    test_required_fields();
    test_edit_without_key_change();
    test_edit_key_repositions_node();
    test_duplicate_names_are_supported();
    test_delete_node_with_two_children();
    test_invalid_index();
    test_periodic_balancing();
    test_manual_balancing_reduces_height();
    test_search();
    test_list_count_validation();
    test_storage_save_and_load();
    test_storage_missing_file_preserves_book();
    test_storage_invalid_format_preserves_book();
    test_clear_frees_tree_state();

    printf("\nРезультат: %d из %d проверок пройдено.\n",
           tests_passed, tests_run);

    if (tests_passed == tests_run) {
        printf("Все автотесты пройдены.\n");
        return 0;
    }

    printf("Есть непройденные автотесты.\n");
    return 1;
}
