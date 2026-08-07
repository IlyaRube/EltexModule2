# Задание 6.1. Телефонная книга со статической библиотекой

Исходная программа из задания 4.1 сохранена: контакты по-прежнему хранятся в **двухсвязном упорядоченном списке**. Изменена организация проекта: структуры и функции работы со списком вынесены в модуль `contact_book.c/.h`, а реализация `contact_book.c` компилируется в статическую библиотеку `libcontactbook.a`.

## Что требуется в задании 6.1

> Доработать решение задачи 4.1 так, чтобы структуры и функции по работе с двухсвязным упорядоченным списком находились в статической библиотеке.

В проекте это реализовано так:

1. `contact_book.c` компилируется отдельно в объектный файл `contact_book.o`;
2. архиватор `ar` создаёт из него статическую библиотеку `libcontactbook.a`;
3. `main.c`, `input.c`, `storage.c` компилируются отдельно;
4. приложение линкуется с `libcontactbook.a` через `-L. -lcontactbook`;
5. автотесты также линкуются именно со статической библиотекой, а не с `contact_book.c` напрямую.

Команды соответствуют схеме из лекции:

```text
gcc contact_book.c -c -o contact_book.o
ar rcs libcontactbook.a contact_book.o
gcc main.o input.o storage.o -L. -lcontactbook -o phonebook_static.exe
```

`contact_book.h` остаётся заголовочным интерфейсом библиотеки. В нём находятся определения `Contact`, `ContactNode`, `ContactBook`, коды результата и объявления функций. Это нормально для C: компилятору модулей-клиентов необходимо видеть типы и прототипы, а исполняемый код функций находится в `libcontactbook.a`.

## Структура проекта

- `contact_book.h` — публичный интерфейс статической библиотеки: структуры контакта, узла, списка и прототипы функций;
- `contact_book.c` — реализация двухсвязного упорядоченного списка;
- `libcontactbook.a` — статическая библиотека, создаётся при сборке;
- `main.c` — меню телефонной книги;
- `input.c/.h` — консольный ввод;
- `storage.c/.h` — сохранение/загрузка `contacts.txt`;
- `tests.c` — автотесты списка, связей, сортировки и файлов;
- `Makefile` — сборка GCC + `ar`;
- `build.bat`, `run_tests.bat` — сборка и тесты на Windows.

## Двухсвязный упорядоченный список

Узел списка:

```c
typedef struct ContactNode {
    Contact contact;
    struct ContactNode *prev;
    struct ContactNode *next;
} ContactNode;
```

Телефонная книга:

```c
typedef struct ContactBook {
    ContactNode *head;
    ContactNode *tail;
    size_t count;
} ContactBook;
```

Список упорядочивается по фамилии, затем по имени и отчеству. При добавлении новый узел сразу вставляется на нужное место. При изменении Ф.И.О. существующий узел отсоединяется и вставляется заново в правильную позицию.

## Сборка на Windows 11 / MinGW GCC

В PowerShell из каталога проекта:

```powershell
mingw32-make clean
mingw32-make test
mingw32-make run
```

Проверить, что библиотека действительно создана и содержит объектный модуль:

```powershell
mingw32-make library-info
```

Ожидаемый вывод:

```text
contact_book.o
```

После `mingw32-make` в каталоге появятся:

```text
contact_book.o
libcontactbook.a
main.o
input.o
storage.o
phonebook_static.exe
```

## Сборка вручную

```powershell
gcc -std=c11 -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=CP1251 -c contact_book.c -o contact_book.o
ar rcs libcontactbook.a contact_book.o

gcc -std=c11 -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=CP1251 -c main.c -o main.o
gcc -std=c11 -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=CP1251 -c input.c -o input.o
gcc -std=c11 -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=CP1251 -c storage.c -o storage.o

gcc main.o input.o storage.o -L. -lcontactbook -o phonebook_static.exe
```

Для тестов:

```powershell
gcc -std=c11 -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=CP1251 -c tests.c -o tests.o
gcc tests.o storage.o -L. -lcontactbook -o tests.exe
tests.exe
```

## Почему это именно статическая библиотека

`libcontactbook.a` создаётся командой `ar`. Во время линковки нужный объектный код из неё включается в `phonebook_static.exe`. Для запуска программы отдельный файл библиотеки рядом с `.exe` не требуется. Это отличие от динамической библиотеки (`.dll`/`.so`).
