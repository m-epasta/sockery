#ifndef SMALLMAP_H
#define SMALLMAP_H

#include <stddef.h>
typedef struct {
    char *key;
    char *val;
} MapEntry;

typedef struct {
    MapEntry *entries;
    size_t count;
    size_t cap;
} Map;

void map_init(Map *map);
void map_put(Map *map, const char *key, const char *value);
char *map_get(Map *map, const char *key);
int map_contains(Map *map, const char *key);
void map_remove(Map *map, const char *key);
size_t map_size(Map *map);
MapEntry *last_entry(Map *map);
void map_free(Map *map);

#endif // SMALLMAP_H
