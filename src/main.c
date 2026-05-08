#include "app_data.h"
#include "glib.h"
#include "renderer.h"
#include "settings.h"
#include <gtk/gtk.h>
#include <stdio.h>

#define DEFAULT_WINDOW_HEIGHT 600
#define DEFAULT_WINDOW_WIDTH 800

void instant_settings(Settings* settings);

static gboolean render_idle_cb(gpointer user_data) {
    AppData* data = user_data;
    render(data->app, data);

    return G_SOURCE_CONTINUE;
}

static void activate(GtkApplication* app, gpointer user_data) {
    AppData* data = (AppData*)user_data;

    GtkWidget* window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "sockery");
    gtk_window_set_default_size(GTK_WINDOW(window), data->width, data->height);
    gtk_window_present(GTK_WINDOW(window));

    g_idle_add(render_idle_cb, user_data);
}

int main(int argc, char* argv[]) {
    Settings settings = use_settings();
    instant_settings(&settings);

    GtkApplication* app = gtk_application_new("org.sockery.explorer",
                                              G_APPLICATION_DEFAULT_FLAGS);

    AppData app_data = {
        .app = app, .width = settings.w_width, .height = settings.w_height};
    g_signal_connect(app, "activate", G_CALLBACK(activate), &app_data);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}

void instant_settings(Settings* settings) {
    if (settings->w_height == 0) {
        printf("WARN: no window height defined: using default height 600\n");
        settings->w_height = DEFAULT_WINDOW_HEIGHT;
    }
    if (settings->w_width == 0) {
        printf("WARN: no window width defined: using default width 800\n");
        settings->w_width = DEFAULT_WINDOW_WIDTH;
    }
}
