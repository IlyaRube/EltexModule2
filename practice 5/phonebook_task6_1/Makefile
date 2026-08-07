CC = gcc
AR = ar
BASE_CFLAGS = -std=c11 -Wall -Wextra -Wpedantic
ARFLAGS = rcs

ifeq ($(OS),Windows_NT)
EXEEXT = .exe
CHARSET_FLAGS = -finput-charset=UTF-8 -fexec-charset=CP1251
RUN_APP = $(APP)
RUN_TESTS = $(TESTS)
REMOVE_FILES = cmd /C del /Q
else
EXEEXT =
CHARSET_FLAGS = -finput-charset=UTF-8 -fexec-charset=UTF-8
RUN_APP = ./$(APP)
RUN_TESTS = ./$(TESTS)
REMOVE_FILES = rm -f
endif

CFLAGS = $(BASE_CFLAGS) $(CHARSET_FLAGS)

APP = phonebook_static$(EXEEXT)
TESTS = tests$(EXEEXT)
LIBRARY = libcontactbook.a

DATA_FILE = contacts.txt
TEMP_DATA_FILE = contacts.txt.tmp

LIB_OBJECTS = contact_book.o
APP_OBJECTS = main.o input.o storage.o
TEST_OBJECTS = tests.o storage.o
HEADERS = contact_book.h input.h storage.h

.PHONY: all run test rebuild clean clean-data library-info help

all: $(APP)

# Задание 6.1: реализация двухсвязного упорядоченного списка
# собирается отдельно в статическую библиотеку libcontactbook.a.
$(LIBRARY): $(LIB_OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

contact_book.o: contact_book.c contact_book.h
	$(CC) $(CFLAGS) -c contact_book.c -o $@

main.o: main.c contact_book.h input.h storage.h
	$(CC) $(CFLAGS) -c main.c -o $@

input.o: input.c input.h contact_book.h
	$(CC) $(CFLAGS) -c input.c -o $@

storage.o: storage.c storage.h contact_book.h
	$(CC) $(CFLAGS) -c storage.c -o $@

tests.o: tests.c contact_book.h storage.h
	$(CC) $(CFLAGS) -c tests.c -o $@

# Важно: -lcontactbook стоит после объектных файлов, которым нужны
# функции библиотеки. Это важно для статической линковки GCC/ld.
$(APP): $(APP_OBJECTS) $(LIBRARY)
	$(CC) $(CFLAGS) $(APP_OBJECTS) -L. -lcontactbook -o $@

$(TESTS): $(TEST_OBJECTS) $(LIBRARY)
	$(CC) $(CFLAGS) $(TEST_OBJECTS) -L. -lcontactbook -o $@

run: $(APP)
	$(RUN_APP)

test: $(TESTS)
	$(RUN_TESTS)

library-info: $(LIBRARY)
	$(AR) t $(LIBRARY)

rebuild: clean all

clean:
ifeq ($(OS),Windows_NT)
	-$(REMOVE_FILES) "$(APP)" "$(TESTS)" "$(LIBRARY)" "contact_book.o" "main.o" "input.o" "storage.o" "tests.o" 2>NUL
else
	$(REMOVE_FILES) "$(APP)" "$(TESTS)" "$(LIBRARY)" contact_book.o main.o input.o storage.o tests.o
endif

clean-data:
ifeq ($(OS),Windows_NT)
	-$(REMOVE_FILES) "$(DATA_FILE)" "$(TEMP_DATA_FILE)" 2>NUL
else
	$(REMOVE_FILES) "$(DATA_FILE)" "$(TEMP_DATA_FILE)"
endif

help:
	@echo Available commands:
	@echo mingw32-make             - build phonebook_static.exe and libcontactbook.a
	@echo mingw32-make run         - build and run application
	@echo mingw32-make test        - build library, tests and run tests
	@echo mingw32-make library-info - show files stored in static library
	@echo mingw32-make rebuild     - clean and rebuild
	@echo mingw32-make clean       - remove build artifacts
	@echo mingw32-make clean-data  - delete contacts.txt
