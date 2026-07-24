CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -O2

ifeq ($(OS),Windows_NT)
TARGET := ipv4_simulator.exe
TEST_TARGET := tests.exe
RUN_PREFIX :=
CLEAN_COMMAND := del /Q $(TARGET) $(TEST_TARGET) *.o 2>NUL || exit 0
else
TARGET := ipv4_simulator
TEST_TARGET := tests
RUN_PREFIX := ./
CLEAN_COMMAND := rm -f $(TARGET) $(TEST_TARGET) *.o
endif

COMMON_OBJECTS := ipv4.o simulation.o
APP_OBJECTS := main.o $(COMMON_OBJECTS)
TEST_OBJECTS := tests.o $(COMMON_OBJECTS)

GATEWAY ?= 192.168.1.1
MASK ?= 255.255.255.0
PACKETS ?= 10

.PHONY: all run test clean

all: $(TARGET)

$(TARGET): $(APP_OBJECTS)
	$(CC) $(CFLAGS) $(APP_OBJECTS) -o $(TARGET)

$(TEST_TARGET): $(TEST_OBJECTS)
	$(CC) $(CFLAGS) $(TEST_OBJECTS) -o $(TEST_TARGET)

main.o: main.c ipv4.h simulation.h
	$(CC) $(CFLAGS) -c main.c -o main.o

ipv4.o: ipv4.c ipv4.h
	$(CC) $(CFLAGS) -c ipv4.c -o ipv4.o

simulation.o: simulation.c simulation.h ipv4.h
	$(CC) $(CFLAGS) -c simulation.c -o simulation.o

tests.o: tests.c ipv4.h simulation.h
	$(CC) $(CFLAGS) -c tests.c -o tests.o

run: $(TARGET)
	$(RUN_PREFIX)$(TARGET) $(GATEWAY) $(MASK) $(PACKETS)

test: $(TEST_TARGET)
	$(RUN_PREFIX)$(TEST_TARGET)

clean:
	$(CLEAN_COMMAND)
