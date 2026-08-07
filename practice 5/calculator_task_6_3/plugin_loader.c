#include "plugin_loader.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <dlfcn.h>
#endif

#ifdef _WIN32
#define PLUGIN_EXTENSION ".dll"
#define DIRECTORY_SEPARATOR '\\'
#else
#define PLUGIN_EXTENSION ".so"
#define DIRECTORY_SEPARATOR '/'
#endif

#define INITIAL_PLUGIN_CAPACITY 4U

typedef struct FileNameList {
    char **items;
    size_t count;
    size_t capacity;
} FileNameList;

static char *copy_string(const char *text)
{
    size_t length;
    char *copy;

    if (text == NULL) {
        return NULL;
    }

    length = strlen(text);
    copy = malloc(length + 1U);

    if (copy == NULL) {
        return NULL;
    }

    memcpy(copy, text, length + 1U);
    return copy;
}

#ifdef _WIN32
static int char_equal_ignore_case(char first, char second)
{
    return tolower((unsigned char)first) == tolower((unsigned char)second);
}
#endif

static int has_extension(const char *name, const char *extension)
{
    size_t name_length;
    size_t extension_length;
    size_t index;
    const char *tail;

    if (name == NULL || extension == NULL) {
        return 0;
    }

    name_length = strlen(name);
    extension_length = strlen(extension);

    if (name_length <= extension_length) {
        return 0;
    }

    tail = name + name_length - extension_length;

    for (index = 0U; index < extension_length; index++) {
#ifdef _WIN32
        if (!char_equal_ignore_case(tail[index], extension[index])) {
            return 0;
        }
#else
        if (tail[index] != extension[index]) {
            return 0;
        }
#endif
    }

    return 1;
}

static void file_name_list_destroy(FileNameList *list)
{
    size_t index;

    if (list == NULL) {
        return;
    }

    for (index = 0U; index < list->count; index++) {
        free(list->items[index]);
    }

    free(list->items);
    list->items = NULL;
    list->count = 0U;
    list->capacity = 0U;
}

static int file_name_list_add(FileNameList *list, const char *name)
{
    char **new_items;
    char *name_copy;
    size_t new_capacity;

    if (list == NULL || name == NULL) {
        return 0;
    }

    if (list->count == list->capacity) {
        new_capacity = list->capacity == 0U
            ? INITIAL_PLUGIN_CAPACITY
            : list->capacity * 2U;

        new_items = realloc(
            list->items,
            new_capacity * sizeof(*new_items)
        );

        if (new_items == NULL) {
            return 0;
        }

        list->items = new_items;
        list->capacity = new_capacity;
    }

    name_copy = copy_string(name);

    if (name_copy == NULL) {
        return 0;
    }

    list->items[list->count] = name_copy;
    list->count++;
    return 1;
}

static int compare_file_names(const void *first, const void *second)
{
    const char *const *first_name = first;
    const char *const *second_name = second;

    return strcmp(*first_name, *second_name);
}

static char *build_path(const char *directory, const char *file_name)
{
    size_t directory_length;
    size_t file_name_length;
    int need_separator;
    char *path;

    if (directory == NULL || file_name == NULL) {
        return NULL;
    }

    directory_length = strlen(directory);
    file_name_length = strlen(file_name);

    need_separator = directory_length > 0U &&
        directory[directory_length - 1U] != '/' &&
        directory[directory_length - 1U] != '\\';

    path = malloc(
        directory_length +
        (need_separator ? 1U : 0U) +
        file_name_length +
        1U
    );

    if (path == NULL) {
        return NULL;
    }

    memcpy(path, directory, directory_length);

    if (need_separator) {
        path[directory_length] = DIRECTORY_SEPARATOR;
        directory_length++;
    }

    memcpy(path + directory_length, file_name, file_name_length + 1U);
    return path;
}

static char *file_stem(const char *file_name)
{
    const char *dot;
    size_t length;
    char *stem;

    if (file_name == NULL) {
        return NULL;
    }

    dot = strrchr(file_name, '.');

    if (dot == NULL || dot == file_name) {
        return NULL;
    }

    length = (size_t)(dot - file_name);
    stem = malloc(length + 1U);

    if (stem == NULL) {
        return NULL;
    }

    memcpy(stem, file_name, length);
    stem[length] = '\0';
    return stem;
}

static PluginLoaderStatus plugin_registry_reserve(
    PluginRegistry *registry,
    size_t required_capacity
)
{
    LoadedPlugin *new_plugins;
    size_t new_capacity;

    if (registry == NULL) {
        return PLUGIN_LOADER_INVALID_ARGUMENT;
    }

    if (required_capacity <= registry->capacity) {
        return PLUGIN_LOADER_OK;
    }

    new_capacity = registry->capacity == 0U
        ? INITIAL_PLUGIN_CAPACITY
        : registry->capacity;

    while (new_capacity < required_capacity) {
        new_capacity *= 2U;
    }

    new_plugins = realloc(
        registry->plugins,
        new_capacity * sizeof(*new_plugins)
    );

    if (new_plugins == NULL) {
        return PLUGIN_LOADER_OUT_OF_MEMORY;
    }

    registry->plugins = new_plugins;
    registry->capacity = new_capacity;
    return PLUGIN_LOADER_OK;
}

#ifdef _WIN32

static PluginLoaderStatus collect_library_names(
    const char *directory,
    FileNameList *list
)
{
    char *pattern;
    size_t directory_length;
    int need_separator;
    WIN32_FIND_DATAA data;
    HANDLE search_handle;

    directory_length = strlen(directory);
    need_separator = directory_length > 0U &&
        directory[directory_length - 1U] != '/' &&
        directory[directory_length - 1U] != '\\';

    pattern = malloc(directory_length + (need_separator ? 1U : 0U) + 2U);

    if (pattern == NULL) {
        return PLUGIN_LOADER_OUT_OF_MEMORY;
    }

    memcpy(pattern, directory, directory_length);

    if (need_separator) {
        pattern[directory_length] = DIRECTORY_SEPARATOR;
        directory_length++;
    }

    pattern[directory_length] = '*';
    pattern[directory_length + 1U] = '\0';

    search_handle = FindFirstFileA(pattern, &data);
    free(pattern);

    if (search_handle == INVALID_HANDLE_VALUE) {
        return PLUGIN_LOADER_DIRECTORY_ERROR;
    }

    do {
        if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0U &&
            has_extension(data.cFileName, PLUGIN_EXTENSION)) {
            if (!file_name_list_add(list, data.cFileName)) {
                FindClose(search_handle);
                return PLUGIN_LOADER_OUT_OF_MEMORY;
            }
        }
    } while (FindNextFileA(search_handle, &data));

    FindClose(search_handle);
    return PLUGIN_LOADER_OK;
}

static void *open_library(const char *path)
{
    return (void *)LoadLibraryA(path);
}

static void close_library(void *handle)
{
    if (handle != NULL) {
        FreeLibrary((HMODULE)handle);
    }
}

static CalculatorOperation find_operation(void *handle, const char *symbol_name)
{
    FARPROC raw_symbol;
    CalculatorOperation operation = NULL;

    raw_symbol = GetProcAddress((HMODULE)handle, symbol_name);

    if (raw_symbol == NULL) {
        return NULL;
    }

    if (sizeof(operation) != sizeof(raw_symbol)) {
        return NULL;
    }

    memcpy(&operation, &raw_symbol, sizeof(operation));
    return operation;
}

#else

static PluginLoaderStatus collect_library_names(
    const char *directory,
    FileNameList *list
)
{
    DIR *dir;
    struct dirent *entry;

    dir = opendir(directory);

    if (dir == NULL) {
        return PLUGIN_LOADER_DIRECTORY_ERROR;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (has_extension(entry->d_name, PLUGIN_EXTENSION)) {
            if (!file_name_list_add(list, entry->d_name)) {
                closedir(dir);
                return PLUGIN_LOADER_OUT_OF_MEMORY;
            }
        }
    }

    closedir(dir);
    return PLUGIN_LOADER_OK;
}

static void *open_library(const char *path)
{
    return dlopen(path, RTLD_NOW);
}

static void close_library(void *handle)
{
    if (handle != NULL) {
        dlclose(handle);
    }
}

static CalculatorOperation find_operation(void *handle, const char *symbol_name)
{
    void *raw_symbol;
    CalculatorOperation operation = NULL;

    dlerror();
    raw_symbol = dlsym(handle, symbol_name);

    if (raw_symbol == NULL || dlerror() != NULL) {
        return NULL;
    }

    if (sizeof(operation) != sizeof(raw_symbol)) {
        return NULL;
    }

    memcpy(&operation, &raw_symbol, sizeof(operation));
    return operation;
}

#endif

void plugin_registry_init(PluginRegistry *registry)
{
    if (registry == NULL) {
        return;
    }

    calculator_menu_init(&registry->menu);
    registry->plugins = NULL;
    registry->count = 0U;
    registry->capacity = 0U;
    registry->skipped_count = 0U;
}

void plugin_registry_destroy(PluginRegistry *registry)
{
    size_t index;

    if (registry == NULL) {
        return;
    }

    /*
     * Сначала уничтожаем меню, затем выгружаем библиотеки.
     * Пока меню существует, его указатели на функции должны оставаться
     * действительными.
     */
    calculator_menu_destroy(&registry->menu);

    for (index = 0U; index < registry->count; index++) {
        close_library(registry->plugins[index].handle);
        free(registry->plugins[index].path);
    }

    free(registry->plugins);
    registry->plugins = NULL;
    registry->count = 0U;
    registry->capacity = 0U;
    registry->skipped_count = 0U;
}

PluginLoaderStatus plugin_registry_load_directory(
    PluginRegistry *registry,
    const char *directory
)
{
    FileNameList names = {NULL, 0U, 0U};
    PluginLoaderStatus loader_status;
    size_t index;

    if (registry == NULL || directory == NULL || directory[0] == '\0') {
        return PLUGIN_LOADER_INVALID_ARGUMENT;
    }

    loader_status = collect_library_names(directory, &names);

    if (loader_status != PLUGIN_LOADER_OK) {
        file_name_list_destroy(&names);
        return loader_status;
    }

    qsort(
        names.items,
        names.count,
        sizeof(*names.items),
        compare_file_names
    );

    for (index = 0U; index < names.count; index++) {
        char *path = build_path(directory, names.items[index]);
        char *symbol_name;
        void *handle;
        CalculatorOperation operation;
        CalculatorStatus menu_status;

        if (path == NULL) {
            file_name_list_destroy(&names);
            return PLUGIN_LOADER_OUT_OF_MEMORY;
        }

        symbol_name = file_stem(names.items[index]);

        if (symbol_name == NULL) {
            free(path);
            registry->skipped_count++;
            continue;
        }

        handle = open_library(path);

        if (handle == NULL) {
            free(symbol_name);
            free(path);
            registry->skipped_count++;
            continue;
        }

        operation = find_operation(handle, symbol_name);

        if (operation == NULL) {
            close_library(handle);
            free(symbol_name);
            free(path);
            registry->skipped_count++;
            continue;
        }

        loader_status = plugin_registry_reserve(
            registry,
            registry->count + 1U
        );

        if (loader_status != PLUGIN_LOADER_OK) {
            close_library(handle);
            free(symbol_name);
            free(path);
            file_name_list_destroy(&names);
            return loader_status;
        }

        menu_status = calculator_menu_add(
            &registry->menu,
            (int)registry->menu.count + 1,
            symbol_name,
            operation
        );

        free(symbol_name);

        if (menu_status == CALCULATOR_OUT_OF_MEMORY) {
            close_library(handle);
            free(path);
            file_name_list_destroy(&names);
            return PLUGIN_LOADER_OUT_OF_MEMORY;
        }

        if (menu_status != CALCULATOR_OK) {
            close_library(handle);
            free(path);
            registry->skipped_count++;
            continue;
        }

        registry->plugins[registry->count].handle = handle;
        registry->plugins[registry->count].path = path;
        registry->count++;
    }

    file_name_list_destroy(&names);
    return PLUGIN_LOADER_OK;
}
