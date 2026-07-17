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
            printf("[FAIL] %s (%s:%d)\n", #condition, __FILE__, __LINE__);       \
        }                                                                          \
    } while (0)

#define EXPECT_SIZE_EQ(expected, actual)                                           \
    do {                                                                           \
        size_t expected_value = (expected);                                        \
        size_t actual_value = (actual);                                            \
        tests_run++;                                                               \
        if (expected_value == actual_value) {                                      \
            tests_passed++;                                                        \
            printf("[PASS] %s == %s\n", #expected, #actual);                     \
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
        if (strcmp(expected_value, actual_value) == 0) {                            \
            tests_passed++;                                                        \
            printf("[PASS] \"%s\" == \"%s\"\n", expected_value, actual_value); \
        } else {                                                                   \
            printf("[FAIL] строки различаются: ожидалось \"%s\", получено \"%s\" " \
                   "(%s:%d)\n",                                                  \
                   expected_value, actual_value, __FILE__, __LINE__);              \
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

static void test_initialization(void)
{
    ContactBook book;

    contact_book_init(&book);
    EXPECT_SIZE_EQ(0, book.count);
}

static void test_add_valid_contact(void)
{
    ContactBook book;
    Contact contact = make_contact("Иванов", "Иван");

    snprintf(contact.phones[0], sizeof(contact.phones[0]), "%s", "+7-900-000-00-00");
    contact.phone_count = 1;

    contact_book_init(&book);
    EXPECT_TRUE(contact_book_add(&book, &contact) == CONTACT_BOOK_OK);
    EXPECT_SIZE_EQ(1, book.count);
    EXPECT_STRING_EQ("Иванов", book.contacts[0].last_name);
    EXPECT_STRING_EQ("+7-900-000-00-00", book.contacts[0].phones[0]);
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
}

static void test_edit_contact(void)
{
    ContactBook book;
    Contact original = make_contact("Петров", "Пётр");
    Contact edited;

    contact_book_init(&book);
    EXPECT_TRUE(contact_book_add(&book, &original) == CONTACT_BOOK_OK);

    edited = original;
    snprintf(edited.position, sizeof(edited.position), "%s", "Инженер");
    snprintf(edited.emails[0], sizeof(edited.emails[0]), "%s", "petr@example.com");
    edited.email_count = 1;

    EXPECT_TRUE(contact_book_update(&book, 0, &edited) == CONTACT_BOOK_OK);
    EXPECT_STRING_EQ("Инженер", book.contacts[0].position);
    EXPECT_STRING_EQ("petr@example.com", book.contacts[0].emails[0]);
}

static void test_delete_contact_and_shift_array(void)
{
    ContactBook book;
    Contact first = make_contact("Андреев", "Андрей");
    Contact second = make_contact("Борисов", "Борис");
    Contact third = make_contact("Васильев", "Василий");

    contact_book_init(&book);
    contact_book_add(&book, &first);
    contact_book_add(&book, &second);
    contact_book_add(&book, &third);

    EXPECT_TRUE(contact_book_delete(&book, 1) == CONTACT_BOOK_OK);
    EXPECT_SIZE_EQ(2, book.count);
    EXPECT_STRING_EQ("Андреев", book.contacts[0].last_name);
    EXPECT_STRING_EQ("Васильев", book.contacts[1].last_name);
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

static void test_capacity_limit(void)
{
    ContactBook book;
    size_t i;
    int all_added = 1;

    contact_book_init(&book);
    for (i = 0; i < MAX_CONTACTS; i++) {
        Contact contact = make_contact("Тестов", "Пользователь");
        if (contact_book_add(&book, &contact) != CONTACT_BOOK_OK) {
            all_added = 0;
            break;
        }
    }

    EXPECT_TRUE(all_added == 1);

    {
        Contact extra = make_contact("Лишний", "Контакт");
        EXPECT_TRUE(contact_book_add(&book, &extra) == CONTACT_BOOK_ERROR_FULL);
    }
    EXPECT_SIZE_EQ(MAX_CONTACTS, book.count);
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
    Contact first = make_contact("Иванов", "Иван");
    Contact second = make_contact("Петров", "Пётр");

    remove_storage_test_files();
    contact_book_init(&source);
    contact_book_init(&loaded);

    snprintf(first.middle_name, sizeof(first.middle_name), "%s", "Иванович");
    snprintf(first.workplace, sizeof(first.workplace), "%s", "Яндекс");
    snprintf(first.position, sizeof(first.position), "%s", "Курьер");
    snprintf(first.phones[0], sizeof(first.phones[0]), "%s", "+7-900-111-22-33");
    first.phone_count = 1;
    snprintf(first.emails[0], sizeof(first.emails[0]), "%s", "ivan@example.com");
    first.email_count = 1;
    snprintf(first.social_links[0], sizeof(first.social_links[0]), "%s", "vk.com/ivan");
    first.social_link_count = 1;
    snprintf(first.messengers[0], sizeof(first.messengers[0]), "%s", "t.me/ivan");
    first.messenger_count = 1;

    snprintf(second.phones[0], sizeof(second.phones[0]), "%s", "+7-901-444-55-66");
    second.phone_count = 1;

    contact_book_add(&source, &first);
    contact_book_add(&source, &second);

    EXPECT_TRUE(storage_save(&source, TEST_STORAGE_FILE) == STORAGE_OK);
    EXPECT_TRUE(storage_load(&loaded, TEST_STORAGE_FILE) == STORAGE_OK);
    EXPECT_SIZE_EQ(2, loaded.count);
    EXPECT_STRING_EQ("Иванов", loaded.contacts[0].last_name);
    EXPECT_STRING_EQ("Иванович", loaded.contacts[0].middle_name);
    EXPECT_STRING_EQ("Яндекс", loaded.contacts[0].workplace);
    EXPECT_STRING_EQ("+7-900-111-22-33", loaded.contacts[0].phones[0]);
    EXPECT_STRING_EQ("ivan@example.com", loaded.contacts[0].emails[0]);
    EXPECT_STRING_EQ("vk.com/ivan", loaded.contacts[0].social_links[0]);
    EXPECT_STRING_EQ("t.me/ivan", loaded.contacts[0].messengers[0]);
    EXPECT_STRING_EQ("Петров", loaded.contacts[1].last_name);

    remove_storage_test_files();
}

static void test_storage_missing_file(void)
{
    ContactBook book;
    Contact contact = make_contact("Сохранов", "Семён");

    remove_storage_test_files();
    contact_book_init(&book);
    contact_book_add(&book, &contact);

    EXPECT_TRUE(storage_load(&book, TEST_STORAGE_FILE) == STORAGE_ERROR_NOT_FOUND);
    EXPECT_SIZE_EQ(1, book.count);
}

static void test_storage_invalid_format(void)
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

    printf("Запуск автотестов телефонной книги...\n\n");

    test_initialization();
    test_add_valid_contact();
    test_required_fields();
    test_edit_contact();
    test_delete_contact_and_shift_array();
    test_invalid_index();
    test_capacity_limit();
    test_search();
    test_list_count_validation();
    test_storage_save_and_load();
    test_storage_missing_file();
    test_storage_invalid_format();

    printf("\nРезультат: %d из %d проверок пройдено.\n",
           tests_passed, tests_run);

    if (tests_passed == tests_run) {
        printf("Все автотесты пройдены.\n");
        return 0;
    }

    printf("Есть непройденные автотесты.\n");
    return 1;
}
