CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -O2 -finput-charset=UTF-8 -fexec-charset=UTF-8 -I.

PLUGIN_NAMES := add subtract multiply divide

ifeq ($(OS),Windows_NT)
EXE := .exe
LIB_EXT := dll
RUN_PREFIX :=
APP_LDLIBS :=
PLUGIN_CFLAGS := $(CFLAGS)
MAKE_PLUGIN_DIR = if not exist "plugins" mkdir "plugins"
else
EXE :=
LIB_EXT := so
RUN_PREFIX := ./
APP_LDLIBS := -ldl
PLUGIN_CFLAGS := $(CFLAGS) -fPIC
MAKE_PLUGIN_DIR = mkdir -p plugins
endif

APP := calculator$(EXE)
TEST_APP := tests$(EXE)

APP_OBJECTS := main.o calculator.o plugin_loader.o
TEST_OBJECTS := tests.o calculator.o plugin_loader.o
PLUGIN_TARGETS := $(addprefix plugins/,$(addsuffix .$(LIB_EXT),$(PLUGIN_NAMES)))

.PHONY: all run test clean rebuild plugins-build

all: $(APP) $(TEST_APP) plugins-build

plugins-build: $(PLUGIN_TARGETS)

$(APP): $(APP_OBJECTS)
	$(CC) $(CFLAGS) $(APP_OBJECTS) -o $@ $(APP_LDLIBS)

$(TEST_APP): $(TEST_OBJECTS)
	$(CC) $(CFLAGS) $(TEST_OBJECTS) -o $@ $(APP_LDLIBS) -lm

main.o: main.c plugin_loader.h calculator.h plugin_api.h
	$(CC) $(CFLAGS) -c main.c -o main.o

calculator.o: calculator.c calculator.h plugin_api.h
	$(CC) $(CFLAGS) -c calculator.c -o calculator.o

plugin_loader.o: plugin_loader.c plugin_loader.h calculator.h plugin_api.h
	$(CC) $(CFLAGS) -c plugin_loader.c -o plugin_loader.o

tests.o: tests.c plugin_loader.h calculator.h plugin_api.h
	$(CC) $(CFLAGS) -c tests.c -o tests.o

plugins/%.$(LIB_EXT): operations/%.c plugin_api.h
	@$(MAKE_PLUGIN_DIR)
	$(CC) $(PLUGIN_CFLAGS) -shared $< -o $@

run: $(APP) plugins-build
	$(RUN_PREFIX)$(APP) plugins

test: $(TEST_APP) plugins-build
	$(RUN_PREFIX)$(TEST_APP)

rebuild: clean all

clean:
ifeq ($(OS),Windows_NT)
	@if exist "$(APP)" del /Q "$(APP)"
	@if exist "$(TEST_APP)" del /Q "$(TEST_APP)"
	@if exist "main.o" del /Q "main.o"
	@if exist "calculator.o" del /Q "calculator.o"
	@if exist "plugin_loader.o" del /Q "plugin_loader.o"
	@if exist "tests.o" del /Q "tests.o"
	@if exist "plugins\*.dll" del /Q "plugins\*.dll"
else
	rm -f $(APP) $(TEST_APP) main.o calculator.o plugin_loader.o tests.o
	rm -f plugins/*.so
endif
