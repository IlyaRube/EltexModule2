CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -O2

TARGET := permissions_app
TEST_TARGET := tests

COMMON_OBJECTS := permissions.o chmod_parser.o file_info.o
APP_OBJECTS := main.o $(COMMON_OBJECTS)
TEST_OBJECTS := tests.o $(COMMON_OBJECTS)

.PHONY: all run test clean

all: $(TARGET)

$(TARGET): $(APP_OBJECTS)
	$(CC) $(CFLAGS) $(APP_OBJECTS) -o $(TARGET)

$(TEST_TARGET): $(TEST_OBJECTS)
	$(CC) $(CFLAGS) $(TEST_OBJECTS) -o $(TEST_TARGET)

main.o: main.c permissions.h chmod_parser.h file_info.h
	$(CC) $(CFLAGS) -c main.c -o main.o

permissions.o: permissions.c permissions.h
	$(CC) $(CFLAGS) -c permissions.c -o permissions.o

chmod_parser.o: chmod_parser.c chmod_parser.h permissions.h
	$(CC) $(CFLAGS) -c chmod_parser.c -o chmod_parser.o

file_info.o: file_info.c file_info.h permissions.h
	$(CC) $(CFLAGS) -c file_info.c -o file_info.o

tests.o: tests.c permissions.h chmod_parser.h file_info.h
	$(CC) $(CFLAGS) -c tests.c -o tests.o

run: $(TARGET)
	./$(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(TARGET) $(TEST_TARGET) *.o
