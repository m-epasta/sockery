#include "settings.h"
#include "smallmap.h"
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* settings_keys[] = {"w_width", "w_height"};
#define SIZEOF_SETTINGS_KEYS (sizeof(settings_keys) / sizeof(settings_keys[0]))

static const IntSettingEntry int_settings[] = {
    {"w_height", offsetof(Settings, w_height)},
    {"w_width", offsetof(Settings, w_width)},
};
#define SIZEOF_ISETTINGS (sizeof(int_settings) / sizeof(int_settings[0]))

int contains(const char* array[], size_t size, const char* target) {
    for (size_t i = 0; i < size; i++) {
        if (strcmp(array[i], target) == 0) {
            return 0;
        }
    }
    return -1;
}

char* take_until(char until, const char** buffer_ptr) {
    if (!buffer_ptr || !*buffer_ptr)
        return NULL;
    const char* buffer = *buffer_ptr;
    const char* pos = strchr(buffer, until);
    size_t len = pos ? (size_t)(pos - buffer) : strlen(buffer);

    char* out = (char*)malloc(len + 1);
    if (!out)
        return NULL;

    memcpy(out, buffer, len);
    out[len] = '\0';

    *buffer_ptr = pos ? pos + 1 : buffer + len;
    return out;
}

Settings use_settings() {
    char* home = getenv("HOME");
    if (home == NULL) {
        fprintf(stderr, "Could not retrieve home directory\n");
        exit(1);
    }
    char conf_path[1024];
    snprintf(conf_path, sizeof(conf_path), "%s/.config/sockery/settings.conf",
             home);
    FILE* config = fopen(conf_path, "r");
    if (config == NULL) {
        fprintf(stderr,
                "Could not open settings file. You can create one by "
                "running the installation script or the settings script.\n");
        exit(1);
    }

    char buffer[MAX_CONF_BUF_SIZE];
    Settings s = {0};
    Map settings;
    map_init(&settings);
    while (fgets(buffer, sizeof(buffer), config)) {
        buffer[strcspn(buffer, "\n")] = '\0';
        parse_line(buffer, &settings);
        MapEntry* last = last_entry(&settings);
        if (last == NULL) {
            fprintf(stderr, "Empty settings map after parse_line\n");
            exit(1);
        }
        Value val = treat_value(last->val);
        push_setting(&val, last, &s);
        if (val.type == TYPE_STRING)
            free(val.value.string);
    }

    if (fclose(config) != 0) {
        perror("fclose failed");
    }

    map_free(&settings);
    return s;
}

void parse_line(const char* line, Map* map) {
    char* key = take_until(' ', &line);
    if (!key) {
        fprintf(stderr, "Could not get key in config file\n");
        exit(1);
    }
    if (contains(settings_keys, SIZEOF_SETTINGS_KEYS, key) != 0) {
        fprintf(stderr, "given field *%s* is not recognized\n", key);
        free(key);
        exit(1);
    }

    while (*line == ' ')
        line++;
    if (*line != '=') {
        fprintf(stderr, "Expected = got %c\n", *line);
        free(key);
        exit(1);
    }
    line++;
    while (*line == ' ')
        line++;

    char* val = strdup(line);
    if (!val) {
        fprintf(stderr, "Could not get value in config file\n");
        free(key);
        exit(1);
    }
    map_put(map, key, val);
    free(val);
    free(key);
}

Value treat_value(char* val) {
    Value result = {0};
    char* endptr;
    errno = 0;

    if (!val) {
        result.type = TYPE_STRING;
        result.value.string = strdup("");
        return result;
    }

    if (val[0] == '"') {
        const char* temp = val + 1;
        char* quoted = take_until('"', &temp);
        if (quoted) {
            result.type = TYPE_STRING;
            result.value.string = quoted;
        } else {
            result.type = TYPE_STRING;
            result.value.string = strdup(val + 1);
        }
        return result;
    }

    long ival = strtol(val, &endptr, 10);
    if (errno != ERANGE && endptr != val && *endptr == '\0') {
        result.type = TYPE_INT;
        result.value.i = (int)ival;
        return result;
    }

    errno = 0;
    float fval = strtof(val, &endptr);
    if (errno != ERANGE && endptr != val && *endptr == '\0') {
        result.type = TYPE_FLOAT;
        result.value.f = fval;
        return result;
    }

    result.type = TYPE_STRING;
    result.value.string = strdup(val);
    return result;
}

// TODO: Add matrtching float and strings logiv when necessary
void push_setting(Value* value, MapEntry* field, Settings* settings) {
    for (size_t i = 0; i < SIZEOF_ISETTINGS; i++) {
        if (strcmp(field->key, int_settings[i].key) == 0) {
            if (value->type == TYPE_INT) {
                int* target = (int*)((char*)settings + int_settings[i].offset);
                *target = value->value.i;
            }
            return;
        }
    }
}
