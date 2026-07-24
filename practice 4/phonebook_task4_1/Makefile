CC = gcc
BASE_CFLAGS = -std=c11 -Wall -Wextra -Wpedantic

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

APP = phonebook_list$(EXEEXT)
TESTS = tests$(EXEEXT)

DATA_FILE = contacts.txt
TEMP_DATA_FILE = contacts.txt.tmp

COMMON_SOURCES = contact_book.c storage.c
APP_SOURCES = main.c input.c $(COMMON_SOURCES)
TEST_SOURCES = tests.c $(COMMON_SOURCES)
HEADERS = contact_book.h input.h storage.h

.PHONY: all run test rebuild clean clean-data sanitize help

all: $(APP)

$(APP): $(APP_SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(APP_SOURCES) -o $(APP)

$(TESTS): $(TEST_SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(TEST_SOURCES) -o $(TESTS)

run: $(APP)
	$(RUN_APP)

test: $(TESTS)
	$(RUN_TESTS)

rebuild: clean all

sanitize:
	$(CC) $(BASE_CFLAGS) -g -O1 -fsanitize=address,undefined \
		-fno-omit-frame-pointer $(TEST_SOURCES) -o tests_sanitize$(EXEEXT)
	./tests_sanitize$(EXEEXT)

clean:
ifeq ($(OS),Windows_NT)
	-$(REMOVE_FILES) "$(APP)" "$(TESTS)" "tests_sanitize$(EXEEXT)" 2>NUL
else
	$(REMOVE_FILES) "$(APP)" "$(TESTS)" "tests_sanitize$(EXEEXT)"
endif

clean-data:
ifeq ($(OS),Windows_NT)
	-$(REMOVE_FILES) "$(DATA_FILE)" "$(TEMP_DATA_FILE)" 2>NUL
else
	$(REMOVE_FILES) "$(DATA_FILE)" "$(TEMP_DATA_FILE)"
endif

help:
	@echo Available commands:
	@echo mingw32-make             - build phonebook_list.exe
	@echo mingw32-make run         - build and run application
	@echo mingw32-make test        - build and run tests
	@echo mingw32-make rebuild     - clean and rebuild
	@echo mingw32-make clean       - remove executable files
	@echo mingw32-make clean-data  - delete contacts.txt
