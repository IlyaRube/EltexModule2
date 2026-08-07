# Задание 6.2. Телефонная книга с динамической библиотекой

Проект является доработкой решения 6.1. Логика телефонной книги не менялась: данные хранятся в **двухсвязном упорядоченном списке**, есть полноценное консольное меню, сохранение/загрузка контактов и автотесты.

Главное изменение задания 6.2: реализация `contact_book.c` теперь находится не в статической `libcontactbook.a`, а в **динамической библиотеке**.

На Windows 11 + MinGW GCC создаются:

```text
contactbook.dll          <- динамическая библиотека с реализацией списка
libcontactbook.dll.a     <- импортная библиотека для этапа линковки
phonebook_dynamic.exe    <- приложение
tests.exe                <- автотесты
```

Важно: `libcontactbook.dll.a` не содержит реализацию так, как статическая `libcontactbook.a`. Это импортная библиотека, которая сообщает линковщику, какие функции приложение будет получать из `contactbook.dll` во время запуска.

## Что находится в динамической библиотеке

В `contact_book.c` реализованы функции работы с контактом и двухсвязным упорядоченным списком:

- `contact_init`;
- `contact_book_init`;
- `contact_book_destroy`;
- `contact_book_add`;
- `contact_book_update`;
- `contact_book_delete`;
- `contact_book_get`;
- `contact_book_get_node`;
- `contact_compare`;
- `contact_is_valid`;
- `contact_matches_query`;
- `contact_book_result_message`.

Публичные структуры `Contact`, `ContactNode`, `ContactBook`, перечисление результата и прототипы функций находятся в `contact_book.h`. Заголовок нужен клиентским `.c`-файлам на этапе компиляции. Исполняемый код перечисленных функций находится в DLL/`.so`.

Для Windows в `contact_book.h` используется стандартная схема экспорта/импорта:

```c
#if defined(_WIN32)
    #if defined(CONTACT_BOOK_BUILD_DLL)
        #define CONTACT_BOOK_API __declspec(dllexport)
    #else
        #define CONTACT_BOOK_API __declspec(dllimport)
    #endif
#else
    #define CONTACT_BOOK_API
#endif
```

При сборке DLL задаётся `CONTACT_BOOK_BUILD_DLL`, поэтому функции экспортируются. При сборке `main.c`, `storage.c`, `input.c` и тестов функции объявляются импортируемыми.

## Полноценное меню

После запуска доступны:

```text
1. Показать список контактов
2. Показать контакт подробно
3. Добавить контакт
4. Редактировать контакт
5. Удалить контакт
6. Найти контакт по Ф.И.О.
7. Показать список в обратном порядке
0. Выход
```

Обратный проход демонстрирует использование связи `prev` двухсвязного списка.

## Сборка на Windows 11 / MinGW GCC

Из PowerShell в каталоге проекта:

```powershell
mingw32-make clean
mingw32-make test
mingw32-make run
```

Полная пересборка:

```powershell
mingw32-make rebuild
```

Проверка того, что приложение действительно зависит от DLL:

```powershell
mingw32-make library-info
```

В выводе `objdump` должна присутствовать строка:

```text
DLL Name: contactbook.dll
```

Это удобная проверка, что `contact_book.c` не был статически включён в `phonebook_dynamic.exe`.

## Сборка вручную на Windows

### 1. Компиляция реализации библиотеки

```powershell
gcc -std=c11 -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=CP1251 -DCONTACT_BOOK_BUILD_DLL -c contact_book.c -o contact_book_dll.o
```

### 2. Создание DLL

```powershell
gcc -shared contact_book_dll.o -Wl,--out-implib,libcontactbook.dll.a -o contactbook.dll
```

### 3. Компиляция приложения

```powershell
gcc -std=c11 -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=CP1251 -c main.c -o main.o
gcc -std=c11 -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=CP1251 -c input.c -o input.o
gcc -std=c11 -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=CP1251 -c storage.c -o storage.o
```

### 4. Линковка приложения с динамической библиотекой

```powershell
gcc main.o input.o storage.o -L. -lcontactbook -o phonebook_dynamic.exe
```

`contactbook.dll` должна оставаться рядом с `phonebook_dynamic.exe` при запуске.

## Автотесты

```powershell
mingw32-make test
```

Тестовый исполняемый файл также линкуется через `-lcontactbook` и использует ту же `contactbook.dll`.

Набор тестов сохранён из задания 6.1. Он проверяет:

- инициализацию пустого списка;
- сортированную вставку;
- корректность `prev`/`next`;
- обязательные фамилию и имя;
- валидацию массивов телефонов/e-mail/соцсетей/мессенджеров;
- получение контактов;
- редактирование с повторной сортировкой;
- удаление из начала, середины и конца;
- поиск по Ф.И.О.;
- обратный проход через `prev`;
- уничтожение списка;
- сохранение и загрузку файла;
- обработку отсутствующего и повреждённого файла.

## Почему это именно динамическая библиотека

В 6.1 код `contact_book.o` помещался в `libcontactbook.a` и во время линковки копировался в EXE.

В 6.2 `contact_book.o` используется для создания `contactbook.dll`. EXE содержит ссылки на импортируемые функции, а их код загружается из DLL при запуске. Если убрать `contactbook.dll`, программа не сможет стартовать.

## Linux

`Makefile` также поддерживает обычный GCC под Linux. Там создаётся `libcontactbook.so` через `-fPIC` и `-shared`, а запуск выполняется с `LD_LIBRARY_PATH=.`.

```bash
make clean
make test
make run
make library-info
```
