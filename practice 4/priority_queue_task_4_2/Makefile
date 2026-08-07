CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2

ifeq ($(OS),Windows_NT)
    EXE = .exe
    RM_CMD = del /Q
else
    EXE =
    RM_CMD = rm -f
endif

APP = priority_queue$(EXE)
TEST_APP = priority_queue_tests$(EXE)

APP_OBJECTS = main.o priority_queue.o tests.o
TEST_OBJECTS = tests_main.o priority_queue.o

.PHONY: all run test clean

all: $(APP)

$(APP): $(APP_OBJECTS)
	$(CC) $(CFLAGS) $(APP_OBJECTS) -o $(APP)

main.o: main.c priority_queue.h tests.h
	$(CC) $(CFLAGS) -c main.c -o main.o

priority_queue.o: priority_queue.c priority_queue.h
	$(CC) $(CFLAGS) -c priority_queue.c -o priority_queue.o

tests.o: tests.c tests.h priority_queue.h
	$(CC) $(CFLAGS) -c tests.c -o tests.o

tests_main.o: tests.c tests.h priority_queue.h
	$(CC) $(CFLAGS) -DTEST_MAIN -c tests.c -o tests_main.o

$(TEST_APP): $(TEST_OBJECTS)
	$(CC) $(CFLAGS) $(TEST_OBJECTS) -o $(TEST_APP)

run: $(APP)
	./$(APP)

test: $(TEST_APP)
	./$(TEST_APP)

clean:
	$(RM_CMD) *.o $(APP) $(TEST_APP)
