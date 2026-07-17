#include "contact_book.h"
#include "input.h"
#include "storage.h"

#include <locale.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define CONTACTS_FILE "contacts.txt"

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

static void print_separator(void)
{
    printf("------------------------------------------------------------\n");
}

static void save_contacts(const ContactBook *book)
{
    StorageResult result = storage_save(book, CONTACTS_FILE);

    if (result == STORAGE_OK) {
        printf("Данные сохранены в файле «%s».\n", CONTACTS_FILE);
    } else {
        printf("Ошибка сохранения: %s\n", storage_result_message(result));
        printf("Текущие изменения останутся только в памяти до закрытия программы.\n");
    }
}

static void print_contact_name(const Contact *contact)
{
    printf("%s %s", contact->last_name, contact->first_name);
    if (contact->middle_name[0] != '\0') {
        printf(" %s", contact->middle_name);
    }
}

static void print_string_list(const char *title,
                              const char items[][MAX_ITEM_LENGTH],
                              size_t count)
{
    size_t i;

    printf("%s:\n", title);
    if (count == 0) {
        printf("  не указано\n");
        return;
    }

    for (i = 0; i < count; i++) {
        printf("  %zu. %s\n", i + 1, items[i]);
    }
}

static void print_contact_details(const Contact *contact, size_t index)
{
    print_separator();
    printf("Контакт №%zu\n", index + 1);
    printf("Ф.И.О.: ");
    print_contact_name(contact);
    printf("\n");
    printf("Место работы: %s\n",
           contact->workplace[0] != '\0' ? contact->workplace : "не указано");
    printf("Должность: %s\n",
           contact->position[0] != '\0' ? contact->position : "не указано");
    print_string_list("Телефоны", contact->phones, contact->phone_count);
    print_string_list("Электронная почта", contact->emails, contact->email_count);
    print_string_list("Социальные сети", contact->social_links,
                      contact->social_link_count);
    print_string_list("Мессенджеры", contact->messengers,
                      contact->messenger_count);
    print_separator();
}

static void list_contacts(const ContactBook *book)
{
    size_t i;

    print_separator();
    printf("СПИСОК КОНТАКТОВ (%zu из %d)\n", book->count, MAX_CONTACTS);
    print_separator();

    if (book->count == 0) {
        printf("Телефонная книга пуста.\n");
        return;
    }

    for (i = 0; i < book->count; i++) {
        const Contact *contact = &book->contacts[i];

        printf("%zu. ", i + 1);
        print_contact_name(contact);
        if (contact->phone_count > 0) {
            printf(" | %s", contact->phones[0]);
        }
        if (contact->workplace[0] != '\0') {
            printf(" | %s", contact->workplace);
        }
        printf("\n");
    }
}

static int read_required_field(const char *prompt, char *field, size_t field_size)
{
    for (;;) {
        if (!input_read_line(prompt, field, field_size)) {
            return 0;
        }

        if (field[0] != '\0') {
            return 1;
        }

        printf("Это поле обязательно для заполнения.\n");
    }
}

static int read_string_list(const char *title,
                            char items[][MAX_ITEM_LENGTH],
                            size_t max_count,
                            size_t *count)
{
    int requested_count;
    size_t i;
    char prompt[160];

    printf("\n%s\n", title);
    if (!input_read_int_range("Количество записей: ", 0, (int)max_count,
                              &requested_count)) {
        return 0;
    }

    memset(items, 0, max_count * MAX_ITEM_LENGTH);
    *count = (size_t)requested_count;

    for (i = 0; i < *count; i++) {
        snprintf(prompt, sizeof(prompt), "  Запись %zu: ", i + 1);
        while (1) {
            if (!input_read_line(prompt, items[i], MAX_ITEM_LENGTH)) {
                return 0;
            }
            if (items[i][0] != '\0') {
                break;
            }
            printf("  Пустую запись сохранять нельзя.\n");
        }
    }

    return 1;
}

static int read_new_contact(Contact *contact)
{
    contact_init(contact);

    printf("\nДОБАВЛЕНИЕ КОНТАКТА\n");
    printf("Обязательные поля отмечены знаком *.\n\n");

    if (!read_required_field("Фамилия*: ", contact->last_name,
                             sizeof(contact->last_name)) ||
        !read_required_field("Имя*: ", contact->first_name,
                             sizeof(contact->first_name)) ||
        !input_read_line("Отчество: ", contact->middle_name,
                         sizeof(contact->middle_name)) ||
        !input_read_line("Место работы: ", contact->workplace,
                         sizeof(contact->workplace)) ||
        !input_read_line("Должность: ", contact->position,
                         sizeof(contact->position)) ||
        !read_string_list("Номера телефонов", contact->phones, MAX_PHONES,
                          &contact->phone_count) ||
        !read_string_list("Адреса электронной почты", contact->emails,
                          MAX_EMAILS, &contact->email_count) ||
        !read_string_list("Ссылки на страницы в социальных сетях",
                          contact->social_links, MAX_SOCIAL_LINKS,
                          &contact->social_link_count) ||
        !read_string_list("Профили в мессенджерах", contact->messengers,
                          MAX_MESSENGERS, &contact->messenger_count)) {
        return 0;
    }

    return 1;
}

static int read_contact_index(const ContactBook *book,
                              const char *prompt,
                              size_t *index)
{
    int number;

    if (book->count == 0) {
        printf("Телефонная книга пуста.\n");
        return 0;
    }

    list_contacts(book);
    if (!input_read_int_range(prompt, 1, (int)book->count, &number)) {
        return 0;
    }

    *index = (size_t)(number - 1);
    return 1;
}

static int edit_text_field(const char *label,
                           char *field,
                           size_t field_size,
                           int required)
{
    char prompt[256];
    char value[MAX_WORK_LENGTH];

    snprintf(prompt, sizeof(prompt),
             "%s [%s] (Enter - оставить%s): ",
             label,
             field[0] != '\0' ? field : "не указано",
             required ? "" : ", - очистить");

    if (!input_read_line(prompt, value, sizeof(value))) {
        return 0;
    }

    if (value[0] == '\0') {
        return 1;
    }

    if (!required && strcmp(value, "-") == 0) {
        field[0] = '\0';
        return 1;
    }

    if (required && strcmp(value, "-") == 0) {
        printf("Обязательное поле очистить нельзя. Значение оставлено прежним.\n");
        return 1;
    }

    snprintf(field, field_size, "%s", value);
    return 1;
}

static int ask_replace_list(const char *title,
                            char items[][MAX_ITEM_LENGTH],
                            size_t max_count,
                            size_t *count)
{
    int answer;
    char prompt[192];

    snprintf(prompt, sizeof(prompt), "Изменить раздел «%s»? (1 - да, 0 - нет): ",
             title);
    if (!input_read_int_range(prompt, 0, 1, &answer)) {
        return 0;
    }

    if (answer == 0) {
        return 1;
    }

    return read_string_list(title, items, max_count, count);
}

static void add_contact(ContactBook *book)
{
    Contact contact;
    ContactBookResult result;

    if (!read_new_contact(&contact)) {
        printf("Ввод прерван.\n");
        return;
    }

    result = contact_book_add(book, &contact);
    printf("%s\n", contact_book_result_message(result));
    if (result == CONTACT_BOOK_OK) {
        save_contacts(book);
    }
}

static void show_contact(const ContactBook *book)
{
    size_t index;

    if (read_contact_index(book, "Введите номер контакта: ", &index)) {
        print_contact_details(&book->contacts[index], index);
    }
}

static void edit_contact(ContactBook *book)
{
    size_t index;
    Contact edited;
    ContactBookResult result;

    if (!read_contact_index(book, "Введите номер контакта для редактирования: ",
                            &index)) {
        return;
    }

    edited = book->contacts[index];
    print_contact_details(&edited, index);
    printf("Enter сохраняет старое значение. Для очистки необязательного поля введите -.\n\n");

    if (!edit_text_field("Фамилия", edited.last_name,
                         sizeof(edited.last_name), 1) ||
        !edit_text_field("Имя", edited.first_name,
                         sizeof(edited.first_name), 1) ||
        !edit_text_field("Отчество", edited.middle_name,
                         sizeof(edited.middle_name), 0) ||
        !edit_text_field("Место работы", edited.workplace,
                         sizeof(edited.workplace), 0) ||
        !edit_text_field("Должность", edited.position,
                         sizeof(edited.position), 0) ||
        !ask_replace_list("Номера телефонов", edited.phones, MAX_PHONES,
                          &edited.phone_count) ||
        !ask_replace_list("Адреса электронной почты", edited.emails,
                          MAX_EMAILS, &edited.email_count) ||
        !ask_replace_list("Социальные сети", edited.social_links,
                          MAX_SOCIAL_LINKS, &edited.social_link_count) ||
        !ask_replace_list("Мессенджеры", edited.messengers,
                          MAX_MESSENGERS, &edited.messenger_count)) {
        printf("Ввод прерван. Изменения не сохранены.\n");
        return;
    }

    result = contact_book_update(book, index, &edited);
    printf("%s\n", contact_book_result_message(result));
    if (result == CONTACT_BOOK_OK) {
        save_contacts(book);
    }
}

static void delete_contact(ContactBook *book)
{
    size_t index;
    int confirmation;
    ContactBookResult result;

    if (!read_contact_index(book, "Введите номер контакта для удаления: ",
                            &index)) {
        return;
    }

    print_contact_details(&book->contacts[index], index);
    if (!input_read_int_range("Удалить этот контакт? (1 - да, 0 - нет): ",
                              0, 1, &confirmation)) {
        return;
    }

    if (confirmation == 0) {
        printf("Удаление отменено.\n");
        return;
    }

    result = contact_book_delete(book, index);
    printf("%s\n", contact_book_result_message(result));
    if (result == CONTACT_BOOK_OK) {
        save_contacts(book);
    }
}

static void search_contacts(const ContactBook *book)
{
    char query[MAX_NAME_LENGTH];
    size_t i;
    size_t found = 0;

    if (!input_read_line("Введите фамилию, имя или отчество для поиска: ",
                         query, sizeof(query))) {
        return;
    }

    if (query[0] == '\0') {
        printf("Пустой поисковый запрос.\n");
        return;
    }

    print_separator();
    for (i = 0; i < book->count; i++) {
        if (contact_matches_query(&book->contacts[i], query)) {
            printf("%zu. ", i + 1);
            print_contact_name(&book->contacts[i]);
            printf("\n");
            found++;
        }
    }

    if (found == 0) {
        printf("Совпадений не найдено.\n");
    } else {
        printf("Найдено контактов: %zu\n", found);
    }
    print_separator();
}

static void print_menu(void)
{
    printf("\n");
    print_separator();
    printf("ТЕЛЕФОННАЯ КНИГА\n");
    print_separator();
    printf("1. Показать список контактов\n");
    printf("2. Показать контакт подробно\n");
    printf("3. Добавить контакт\n");
    printf("4. Редактировать контакт\n");
    printf("5. Удалить контакт\n");
    printf("6. Найти контакт по Ф.И.О.\n");
    printf("0. Выход\n");
    print_separator();
}

int main(void)
{
    ContactBook book;
    StorageResult load_result;
    int choice;

    configure_console();
    contact_book_init(&book);

    load_result = storage_load(&book, CONTACTS_FILE);
    if (load_result == STORAGE_OK) {
        printf("Загружено контактов из файла «%s»: %zu.\n",
               CONTACTS_FILE, book.count);
    } else if (load_result == STORAGE_ERROR_NOT_FOUND) {
        printf("Файл «%s» пока не создан. Он появится после первого изменения.\n",
               CONTACTS_FILE);
    } else {
        printf("Предупреждение: %s\n", storage_result_message(load_result));
        printf("Запущена пустая телефонная книга. Исходный файл не изменён.\n");
        contact_book_init(&book);
    }

    for (;;) {
        print_menu();
        if (!input_read_int_range("Выберите пункт меню: ", 0, 6, &choice)) {
            printf("\nВвод завершён.\n");
            break;
        }

        switch (choice) {
            case 1:
                list_contacts(&book);
                break;
            case 2:
                show_contact(&book);
                break;
            case 3:
                add_contact(&book);
                break;
            case 4:
                edit_contact(&book);
                break;
            case 5:
                delete_contact(&book);
                break;
            case 6:
                search_contacts(&book);
                break;
            case 0:
                printf("Работа программы завершена.\n");
                return 0;
            default:
                break;
        }
    }

    return 0;
}
