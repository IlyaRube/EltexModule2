Калькулятор, задание 2.2

Сборка на Windows 11 через MinGW/MSYS2:

    mingw32-make

Запуск автотестов:

    mingw32-make test

Запуск калькулятора:

    mingw32-make run

Полная пересборка:

    mingw32-make rebuild

Очистка:

    mingw32-make clean

Русский текст выводится через Unicode API Windows, поэтому команда chcp 65001
не требуется.
