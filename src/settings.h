#ifndef SETTINGS_H
#define SETTINGS_H
#include "smallmap.h"
#include <stddef.h>

#define MAX_CONF_BUF_SIZE 1024

typedef struct {
    int w_height;
    int w_width;
} Settings;

typedef struct {
    const char *key;
    size_t offset;
} IntSettingEntry;

typedef enum { TYPE_INT, TYPE_FLOAT, TYPE_STRING } ValueType;

typedef struct {
    ValueType type;
    union {
        int i;
        float f;
        char *string;
    } value;
} Value;

/* Returns 0 if found and -1 if not found */
int contains(const char *array[], size_t size, const char *target);
/* MUST BE FREED */
char *take_until(char until, const char **buffer_ptr);

Settings use_settings();
void parse_line(const char *line, Map *map);
Value treat_value(char *val);
void push_setting(Value *value, MapEntry *field, Settings *settings);

#endif // !SETTINGS_H
