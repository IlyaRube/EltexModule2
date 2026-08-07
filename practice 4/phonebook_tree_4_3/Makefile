CC = gcc
BASE_CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2 -finput-charset=UTF-8

ifeq ($(OS),Windows_NT)
EXEEXT = .exe
CFLAGS = $(BASE_CFLAGS) -fexec-charset=CP1251
RUN_APP = $(APP)
RUN_TESTS = $(TESTS)
REMOVE_FILES = cmd /C del /Q
else
EXEEXT =
CFLAGS = $(BASE_CFLAGS) -fexec-charset=UTF-8
RUN_APP = ./$(APP)
RUN_TESTS = ./$(TESTS)
REMOVE_FILES = rm -f
endif

APP = phonebook$(EXEEXT)
TESTS = tests$(EXEEXT)

DATA_FILE = contacts.txt
TEMP_DATA_FILE = contacts.txt.tmp

COMMON_SOURCES = contact_book.c storage.c
APP_SOURCES = main.c input.c $(COMMON_SOURCES)
TEST_SOURCES = tests.c $(COMMON_SOURCES)

HEADERS = contact_book.h input.h storage.h

.PHONY: all run test debug-test rebuild clean clean-data help

all: $(APP)

$(APP): $(APP_SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(APP_SOURCES) -o $(APP)

$(TESTS): $(TEST_SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(TEST_SOURCES) -o $(TESTS)

run: $(APP)
	$(RUN_APP)

test: $(TESTS)
	$(RUN_TESTS)

debug-test:
	$(CC) -std=c11 -Wall -Wextra -Wpedantic -g -fsanitize=address,undefined \
		-finput-charset=UTF-8 -fexec-charset=UTF-8 $(TEST_SOURCES) -o tests_san
	./tests_san

rebuild: clean all

clean:
ifeq ($(OS),Windows_NT)
	-$(REMOVE_FILES) "$(APP)" "$(TESTS)" "tests_san.exe" 2>NUL
else
	$(REMOVE_FILES) "$(APP)" "$(TESTS)" "tests_san"
endif

clean-data:
ifeq ($(OS),Windows_NT)
	-$(REMOVE_FILES) "$(DATA_FILE)" "$(TEMP_DATA_FILE)" 2>NUL
else
	$(REMOVE_FILES) "$(DATA_FILE)" "$(TEMP_DATA_FILE)"
endif

help:
	@echo Available commands:
	@echo make            - build phonebook
	@echo make run        - build and run phonebook
	@echo make test       - build and run tests
	@echo make debug-test - run tests with sanitizers on Linux/WSL
	@echo make rebuild    - clean and rebuild phonebook
	@echo make clean      - remove executable files
	@echo make clean-data - delete saved contacts
