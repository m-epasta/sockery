#ifndef RENDERER_H
#define RENDERER_H

#include "glib.h"
#include "gtk/gtk.h"
#include <stdbool.h>
static bool ACTIVE = true;

void render(GtkApplication *app, gpointer user_data);

#endif // !RENDERER_H
