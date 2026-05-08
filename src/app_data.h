#ifndef APP_DATA_H
#define APP_DATA_H

#include <gtk/gtk.h>

typedef struct {
    GtkApplication *app;
    int width;
    int height;
} AppData;

#endif // !APP_DATA_H
