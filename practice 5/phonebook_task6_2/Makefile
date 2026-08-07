CC = gcc
BASE_CFLAGS = -std=c11 -Wall -Wextra -Wpedantic

ifeq ($(OS),Windows_NT)
EXEEXT = .exe
CHARSET_FLAGS = -finput-charset=UTF-8 -fexec-charset=CP1251
APP = phonebook_dynamic$(EXEEXT)
TESTS = tests$(EXEEXT)
SHARED_LIBRARY = contactbook.dll
IMPORT_LIBRARY = libcontactbook.dll.a
LIB_OBJECT = contact_book_dll.o
SHARED_CFLAGS = $(CFLAGS) -DCONTACT_BOOK_BUILD_DLL
SHARED_LINK = $(CC) -shared $(LIB_OBJECT) -Wl,--out-implib,$(IMPORT_LIBRARY) -o $(SHARED_LIBRARY)
RUN_APP = $(APP)
RUN_TESTS = $(TESTS)
REMOVE_FILES = cmd /C del /Q
else
EXEEXT =
CHARSET_FLAGS = -finput-charset=UTF-8 -fexec-charset=UTF-8
APP = phonebook_dynamic
TESTS = tests
SHARED_LIBRARY = libcontactbook.so
IMPORT_LIBRARY =
LIB_OBJECT = contact_book_pic.o
SHARED_CFLAGS = $(CFLAGS) -fPIC -DCONTACT_BOOK_BUILD_DLL
SHARED_LINK = $(CC) -shared $(LIB_OBJECT) -o $(SHARED_LIBRARY)
RUN_APP = LD_LIBRARY_PATH=. ./$(APP)
RUN_TESTS = LD_LIBRARY_PATH=. ./$(TESTS)
REMOVE_FILES = rm -f
endif

CFLAGS = $(BASE_CFLAGS) $(CHARSET_FLAGS)

DATA_FILE = contacts.txt
TEMP_DATA_FILE = contacts.txt.tmp
LEGACY_STATIC_LIBRARY = libcontactbook.a

APP_OBJECTS = main.o input.o storage.o
TEST_OBJECTS = tests.o storage.o

.PHONY: all run test rebuild clean clean-data library-info help

all: $(APP)

# Задание 6.2: реализация двухсвязного упорядоченного списка
# собирается в динамическую библиотеку: contactbook.dll (Windows)
# или libcontactbook.so (Linux).
$(LIB_OBJECT): contact_book.c contact_book.h
	$(CC) $(SHARED_CFLAGS) -c contact_book.c -o $@

$(SHARED_LIBRARY): $(LIB_OBJECT)
	$(SHARED_LINK)

main.o: main.c contact_book.h input.h storage.h
	$(CC) $(CFLAGS) -c main.c -o $@

input.o: input.c input.h contact_book.h
	$(CC) $(CFLAGS) -c input.c -o $@

storage.o: storage.c storage.h contact_book.h
	$(CC) $(CFLAGS) -c storage.c -o $@

tests.o: tests.c contact_book.h storage.h
	$(CC) $(CFLAGS) -c tests.c -o $@

# На Windows -lcontactbook использует импортную библиотеку
# libcontactbook.dll.a и оставляет реальные функции в contactbook.dll.
# На Linux -lcontactbook связывает программу с libcontactbook.so.
$(APP): $(APP_OBJECTS) $(SHARED_LIBRARY)
	$(CC) $(CFLAGS) $(APP_OBJECTS) -L. -lcontactbook -o $@

$(TESTS): $(TEST_OBJECTS) $(SHARED_LIBRARY)
	$(CC) $(CFLAGS) $(TEST_OBJECTS) -L. -lcontactbook -o $@

run: $(APP)
	$(RUN_APP)

test: $(TESTS)
	$(RUN_TESTS)

library-info: $(APP) $(SHARED_LIBRARY)
ifeq ($(OS),Windows_NT)
	@echo Dynamic library:
	@dir /B "$(SHARED_LIBRARY)" "$(IMPORT_LIBRARY)"
	@echo.
	@echo DLL dependencies of $(APP):
	@objdump -p "$(APP)" | findstr /C:"DLL Name"
else
	@echo Dynamic library:
	@ls -l "$(SHARED_LIBRARY)"
	@echo
	@echo Shared-library dependencies of $(APP):
	@LD_LIBRARY_PATH=. ldd "./$(APP)" | grep contactbook
endif

rebuild: clean all

clean:
ifeq ($(OS),Windows_NT)
	-$(REMOVE_FILES) "$(APP)" "$(TESTS)" "$(SHARED_LIBRARY)" "$(IMPORT_LIBRARY)" "$(LEGACY_STATIC_LIBRARY)" "contact_book_dll.o" "main.o" "input.o" "storage.o" "tests.o" 2>NUL
else
	$(REMOVE_FILES) "$(APP)" "$(TESTS)" "$(SHARED_LIBRARY)" "$(LEGACY_STATIC_LIBRARY)" contact_book_pic.o main.o input.o storage.o tests.o
endif

clean-data:
ifeq ($(OS),Windows_NT)
	-$(REMOVE_FILES) "$(DATA_FILE)" "$(TEMP_DATA_FILE)" 2>NUL
else
	$(REMOVE_FILES) "$(DATA_FILE)" "$(TEMP_DATA_FILE)"
endif

help:
	@echo Available commands:
	@echo mingw32-make              - build phonebook_dynamic.exe and contactbook.dll
	@echo mingw32-make run          - build and run application
	@echo mingw32-make test         - build DLL, tests and run 100 autotests
	@echo mingw32-make library-info - prove that EXE depends on contactbook.dll
	@echo mingw32-make rebuild      - clean and rebuild
	@echo mingw32-make clean        - remove build artifacts
	@echo mingw32-make clean-data   - delete contacts.txt
