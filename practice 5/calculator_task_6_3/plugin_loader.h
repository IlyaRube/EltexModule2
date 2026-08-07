#ifndef PLUGIN_LOADER_H
#define PLUGIN_LOADER_H

#include "calculator.h"

#include <stddef.h>

typedef enum PluginLoaderStatus {
    PLUGIN_LOADER_OK = 0,
    PLUGIN_LOADER_INVALID_ARGUMENT,
    PLUGIN_LOADER_DIRECTORY_ERROR,
    PLUGIN_LOADER_OUT_OF_MEMORY
} PluginLoaderStatus;

typedef struct LoadedPlugin {
    void *handle;
    char *path;
} LoadedPlugin;

typedef struct PluginRegistry {
    CalculatorMenu menu;
    LoadedPlugin *plugins;
    size_t count;
    size_t capacity;
    size_t skipped_count;
} PluginRegistry;

void plugin_registry_init(PluginRegistry *registry);
void plugin_registry_destroy(PluginRegistry *registry);

PluginLoaderStatus plugin_registry_load_directory(
    PluginRegistry *registry,
    const char *directory
);

#endif
