# Задание 2.1. Телефонная книга на языке Си

Консольное приложение с меню, статическими массивами, автотестами и сохранением контактов в файл.

## Что реализовано

- хранение до 100 контактов в массиве;
- обязательные поля: фамилия и имя;
- хранение места работы и должности;
- до 5 телефонов, адресов электронной почты, социальных сетей и мессенджеров на контакт;
- добавление, просмотр, поиск, редактирование и удаление контактов;
- автоматическая загрузка контактов при запуске;
- автоматическое сохранение после добавления, редактирования и удаления;
- текстовый файл `contacts.txt` в кодировке UTF-8;
- безопасная запись через временный файл `contacts.txt.tmp`;
- сборка и запуск через `Makefile`;
- автотесты операций телефонной книги и файлового хранения.

## Файлы проекта

- `main.c` — меню и пользовательский интерфейс;
- `contact_book.h`, `contact_book.c` — структуры и операции с массивом контактов;
- `storage.h`, `storage.c` — загрузка и сохранение контактов;
- `input.h`, `input.c` — безопасный ввод;
- `tests.c` — автотесты;
- `Makefile` — сборка, запуск и тестирование;
- `build.bat`, `run_tests.bat` — дополнительная сборка без Make.

## Где хранятся контакты

Программа использует файл:

```text
contacts.txt
```

Он создаётся в текущей папке после первого добавления, редактирования или удаления контакта. При следующем запуске программа загружает данные из этого файла.

Команда `make clean` удаляет только исполняемые файлы и не удаляет контакты.

Команда `make clean-data` удаляет `contacts.txt`. Использовать её следует только тогда, когда нужно полностью очистить сохранённую телефонную книгу.

## Сборка через Makefile в Windows 11

Откройте PowerShell в папке проекта:

```powershell
cd D:\eltexC\phonebook_c
```

Проверьте, какая команда Make доступна:

```powershell
where.exe make
where.exe mingw32-make
```

Если доступна команда `mingw32-make`, используйте:

```powershell
mingw32-make clean
mingw32-make
mingw32-make test
mingw32-make run
```

Если доступна команда `make`, используйте:

```powershell
make clean
make
make test
make run
```

## Цели Makefile

```text
make             сборка phonebook.exe
make run         сборка и запуск программы
make test        сборка и запуск автотестов
make rebuild     полная пересборка программы
make clean       удаление phonebook.exe и tests.exe
make clean-data  удаление contacts.txt
make help        список команд
```

Вместо `make` можно использовать `mingw32-make`, если именно эта команда установлена в MSYS2.

## Ручная сборка

Основная программа:

```powershell
gcc -std=c11 -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8 main.c input.c contact_book.c storage.c -o phonebook.exe
```

Автотесты:

```powershell
gcc -std=c11 -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8 tests.c contact_book.c storage.c -o tests.exe
.\tests.exe
```
