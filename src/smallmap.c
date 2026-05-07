#include "smallmap.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 8

void map_init(Map* map) {
    map->entries = NULL;
    map->count = 0;
    map->cap = 0;
}

static bool ensure_cap(Map* map) {
    if (map->count < map->cap)
        return true;
    size_t ncap = map->cap ? map->cap * 2 : INITIAL_CAPACITY;
    MapEntry* nentries = realloc(map->entries, sizeof(MapEntry) * ncap);
    if (!nentries)
        return false;
    map->entries = nentries;
    map->cap = ncap;
    return true;
}

void map_put(Map* map, const char* key, const char* value) {
    if (!key || !value)
        return;

    for (size_t i = 0; i < map->count; i++) {
        if (strcmp(map->entries[i].key, key) == 0) {
            char* nval = strdup(value);
            if (!nval)
                return;
            free(map->entries[i].val);
            map->entries[i].val = nval;
            return;
        }
    }

    if (!ensure_cap(map))
        return;

    char* new_key = strdup(key);
    char* new_val = strdup(value);
    if (!new_key || !new_val) {
        free(new_key);
        free(new_val);
        return;
    }

    map->entries[map->count].key = new_key;
    map->entries[map->count].val = new_val;
    map->count++;
}

char* map_get(Map* map, const char* key) {
    if (!map || !key)
        return NULL;
    for (size_t i = 0; i < map->count; i++) {
        if (strcmp(map->entries[i].key, key) == 0)
            return map->entries[i].val;
    }
    return NULL;
}

int map_contains(Map* map, const char* key) {
    return map_get(map, key) != NULL;
}

void map_remove(Map* map, const char* key) {
    for (size_t i = 0; i < map->count; i++) {
        if (strcmp(map->entries[i].key, key) == 0) {
            free(map->entries[i].key);
            free(map->entries[i].val);
            for (size_t j = i; j < map->count - 1; j++)
                map->entries[j] = map->entries[j + 1];
            map->count--;
            return;
        }
    }
}

size_t map_size(Map* map) { return map ? map->count : 0; }
MapEntry* last_entry(Map* map) { return map->count ? &map->entries[map->count - 1] : NULL; }

void map_free(Map* map) {
    if (!map)
        return;
    for (size_t i = 0; i < map->count; i++) {
        free(map->entries[i].key);
        free(map->entries[i].val);
    }
    free(map->entries);
    map->entries = NULL;
    map->count = 0;
    map->cap = 0;
}
