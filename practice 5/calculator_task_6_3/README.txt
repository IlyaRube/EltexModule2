Калькулятор, задание 6.3
=======================

Задание:
Доработать решение 2.3 так, чтобы функции загружались из динамических
библиотек. В одной библиотеке находится одна функция. При запуске программа
считывает каталог с библиотеками и загружает найденные функции.

Что реализовано
---------------
1. Меню по-прежнему динамическое и хранит указатели на функции, как в 2.3.
2. В main.c нет арифметических функций и нет switch по операциям.
3. При запуске сканируется каталог plugins.
4. Windows: библиотеки загружаются через LoadLibraryA/GetProcAddress.
5. Linux: библиотеки загружаются через dlopen/dlsym.
6. Каждая операция находится в отдельной DLL/SO:
       plugins/add.dll       или plugins/add.so
       plugins/subtract.dll  или plugins/subtract.so
       plugins/multiply.dll  или plugins/multiply.so
       plugins/divide.dll    или plugins/divide.so
7. Имя файла библиотеки задаёт имя экспортируемой функции:
       add.dll -> функция add
       divide.so -> функция divide
8. Поэтому новую команду можно добавить без изменения main.c: достаточно
   положить новую совместимую библиотеку в каталог plugins и перезапустить
   калькулятор.

Структура
---------
main.c               - меню, ввод пользователя, запуск выбранной операции
calculator.c/.h      - динамический массив команд и вызов указателя на функцию
plugin_loader.c/.h   - сканирование каталога и загрузка DLL/SO
plugin_api.h         - общий интерфейс функций-плагинов
operations/*.c       - исходники четырёх отдельных библиотек
plugins/             - каталог собранных DLL/SO
tests.c              - автотесты, включая реальную загрузку библиотек
Makefile             - сборка Windows/Linux

Сборка Windows 11 + MinGW
-------------------------
В PowerShell из каталога проекта:

    mingw32-make clean
    mingw32-make

Автотесты:

    mingw32-make test

Запуск:

    mingw32-make run

Или напрямую:

    .\calculator.exe plugins

Можно передать другой каталог библиотек:

    .\calculator.exe D:\my_plugins

Сборка Linux / WSL
------------------

    make clean
    make
    make test
    make run

Или:

    ./calculator plugins

Как добавить новую операцию
---------------------------
Например, файл operations/maximum.c:

#include "plugin_api.h"

CALCULATOR_PLUGIN_EXPORT CalculatorStatus maximum(
    double first,
    double second,
    double *result
)
{
    if (result == 0) {
        return CALCULATOR_NULL_RESULT;
    }

    *result = first > second ? first : second;
    return CALCULATOR_OK;
}

Главное правило: имя библиотеки и имя функции должны совпадать.
Для maximum.c библиотека должна называться maximum.dll (Windows) или
maximum.so (Linux), а экспортируемая функция должна называться maximum.

Ручная сборка новой библиотеки в Windows:

    gcc -std=c11 -Wall -Wextra -Wpedantic -I. -shared operations\maximum.c -o plugins\maximum.dll

Ручная сборка в Linux:

    gcc -std=c11 -Wall -Wextra -Wpedantic -I. -fPIC -shared operations/maximum.c -o plugins/maximum.so

После этого достаточно снова запустить calculator. Перекомпилировать main.c
для обнаружения новой операции не требуется.
