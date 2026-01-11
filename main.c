#include <gtk-3.0/gtk/gtk.h>
#include <vte/vte.h>

typedef struct {
    GtkWidget *window;
    GtkWidget *terminal;
    GtkWidget *header_bar;
    GtkWidget *dark_mode_switch;
} AppWidgets;

static void
on_dark_mode_toggled(GtkSwitch *sw, gboolean state, gpointer user_data)
{
    (void) sw;
    (void) user_data;

    GtkSettings *settings = gtk_settings_get_default();
    g_object_set(settings, "gtk-application-prefer-dark-theme", state, NULL);
}

static void
on_about_clicked(GtkButton *button, gpointer user_data)
{
    (void) button;
    AppWidgets *app = (AppWidgets *)user_data;

    const gchar *authors[] = { "Zap735335", NULL };

    gtk_show_about_dialog(GTK_WINDOW(app->window),
                          "program-name", "Zxc Terminal",
                          "version", "1.0",
                          "comments", "A lightweight GTK + VTE terminal emulator.",
                          "authors", authors, 
                          "website", "https://github.com/Hi7548388/Zxc-terminal",
                          NULL);
}

static void
on_child_exited(VteTerminal *terminal, gint status, gpointer user_data)
{
    (void) terminal;
    (void) status;
    AppWidgets *app = (AppWidgets *)user_data;
    gtk_window_close(GTK_WINDOW(app->window));
}

static void
activate(GtkApplication *app, gpointer user_data)
{
    (void) user_data;

    AppWidgets *widgets = g_new0(AppWidgets, 1);

    widgets->window = gtk_application_window_new(app);
    gtk_window_set_default_size(GTK_WINDOW(widgets->window), 800, 600);
    gtk_window_set_title(GTK_WINDOW(widgets->window), "Zxc Terminal");

    // Header bar
    widgets->header_bar = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(widgets->header_bar), "Zxc Terminal");
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(widgets->header_bar), TRUE);
    gtk_window_set_titlebar(GTK_WINDOW(widgets->window), widgets->header_bar);

    // About button
    GtkWidget *about_button = gtk_button_new_from_icon_name("help-about", GTK_ICON_SIZE_BUTTON);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(widgets->header_bar), about_button);
    g_signal_connect(about_button, "clicked",
                     G_CALLBACK(on_about_clicked), widgets);

    // Dark mode switch
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *label = gtk_label_new("Light-Dark");
    widgets->dark_mode_switch = gtk_switch_new();

    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), widgets->dark_mode_switch, FALSE, FALSE, 0);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(widgets->header_bar), box);

    g_signal_connect(widgets->dark_mode_switch, "state-set",
                     G_CALLBACK(on_dark_mode_toggled), NULL);

    // Terminal
    widgets->terminal = vte_terminal_new();

    const char *default_shell = g_getenv("SHELL");
    if (!default_shell || default_shell[0] == '\0') {
        default_shell = "/bin/bash";
    }

    char *argv[] = { (char *)default_shell, NULL };

    vte_terminal_spawn_async(
        VTE_TERMINAL(widgets->terminal),
        VTE_PTY_DEFAULT,
        NULL,
        argv,
        NULL,
        G_SPAWN_DEFAULT,
        NULL, NULL,
        NULL,
        -1,
        NULL,
        NULL,
        NULL
    );

    g_signal_connect(widgets->terminal, "child-exited",
                     G_CALLBACK(on_child_exited), widgets);

    // Scroll window
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled), widgets->terminal);

    gtk_container_add(GTK_CONTAINER(widgets->window), scrolled);

    gtk_widget_show_all(widgets->window);
}

int
main(int argc, char **argv)
{
    GtkApplication *app;
    int status;

    app = gtk_application_new("com.example.GtkTerminal", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}

