CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -O2 -finput-charset=UTF-8 -fexec-charset=UTF-8
LDLIBS := -lm

ifeq ($(OS),Windows_NT)
EXE := .exe
RUN_PREFIX :=
DELETE_FILE = if exist "$(1)" del /Q "$(1)"
else
EXE :=
RUN_PREFIX := ./
DELETE_FILE = rm -f "$(1)"
endif

APP := calculator$(EXE)
TEST_APP := tests$(EXE)

APP_OBJECTS := main.o calculator.o
TEST_OBJECTS := tests.o calculator.o

.PHONY: all run test clean rebuild

all: $(APP) $(TEST_APP)

$(APP): $(APP_OBJECTS)
	$(CC) $(CFLAGS) $(APP_OBJECTS) -o $@

$(TEST_APP): $(TEST_OBJECTS)
	$(CC) $(CFLAGS) $(TEST_OBJECTS) -o $@ $(LDLIBS)

main.o: main.c calculator.h
	$(CC) $(CFLAGS) -c main.c -o main.o

calculator.o: calculator.c calculator.h
	$(CC) $(CFLAGS) -c calculator.c -o calculator.o

tests.o: tests.c calculator.h
	$(CC) $(CFLAGS) -c tests.c -o tests.o

run: $(APP)
	$(RUN_PREFIX)$(APP)

test: $(TEST_APP)
	$(RUN_PREFIX)$(TEST_APP)

rebuild: clean all

clean:
	@$(call DELETE_FILE,$(APP))
	@$(call DELETE_FILE,$(TEST_APP))
	@$(call DELETE_FILE,main.o)
	@$(call DELETE_FILE,calculator.o)
	@$(call DELETE_FILE,tests.o)
