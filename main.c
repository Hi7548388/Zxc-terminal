#include <gtk-3.0/gtk/gtk.h>
#include <vte/vte.h>

typedef struct {
    GtkWidget *window;
    GtkWidget *terminal;
    GtkWidget *header_bar;
    GtkWidget *dark_mode_switch;
    GKeyFile  *theme_file;
} AppWidgets;

/* -----------------------------------------------------------
   Load theme from ~/.config/zxc-terminal/themes.conf
   ----------------------------------------------------------- */
static gboolean
load_theme(GKeyFile *kf, const char *name, GdkRGBA *fg, GdkRGBA *bg, GdkRGBA palette[16])
{
    gchar *fg_hex = g_key_file_get_string(kf, name, "fg", NULL);
    gchar *bg_hex = g_key_file_get_string(kf, name, "bg", NULL);
    gchar *palette_hex = g_key_file_get_string(kf, name, "palette", NULL);

    if (!fg_hex || !bg_hex) {
        g_warning("Theme '%s' missing fg/bg", name);
        g_free(fg_hex);
        g_free(bg_hex);
        g_free(palette_hex);
        return FALSE;
    }

    gdk_rgba_parse(fg, fg_hex);
    gdk_rgba_parse(bg, bg_hex);

    if (palette_hex) {
        gchar **colors = g_strsplit(palette_hex, ",", 16);
        for (int i = 0; i < 16 && colors[i]; i++)
            gdk_rgba_parse(&palette[i], colors[i]);
        g_strfreev(colors);
    }

    g_free(fg_hex);
    g_free(bg_hex);
    g_free(palette_hex);

    return TRUE;
}

/* -----------------------------------------------------------
   Apply theme to terminal
   ----------------------------------------------------------- */
static void
apply_color_scheme(AppWidgets *app, const gchar *scheme)
{
    GdkRGBA fg, bg;
    GdkRGBA palette[16];

    if (load_theme(app->theme_file, scheme, &fg, &bg, palette)) {
        vte_terminal_set_colors(VTE_TERMINAL(app->terminal),
                                &fg, &bg, palette, 16);
    } else {
        g_warning("Failed to load theme '%s'", scheme);
    }
}

/* -----------------------------------------------------------
   Combo box changed
   ----------------------------------------------------------- */
static void
on_color_scheme_changed(GtkComboBox *combo, gpointer user_data)
{
    AppWidgets *app = user_data;
    gchar *scheme = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));

    if (scheme) {
        apply_color_scheme(app, scheme);
        g_free(scheme);
    }
}

/* -----------------------------------------------------------
   Dark mode toggle
   ----------------------------------------------------------- */
static void
on_dark_mode_toggled(GtkSwitch *sw, gboolean state, gpointer user_data)
{
    GtkSettings *settings = gtk_settings_get_default();
    g_object_set(settings, "gtk-application-prefer-dark-theme", state, NULL);
}

/* -----------------------------------------------------------
   About dialog
   ----------------------------------------------------------- */
static void
on_about_clicked(GtkButton *button, gpointer user_data)
{
    AppWidgets *app = user_data;
    const gchar *authors[] = { "Zap735335", NULL };

    gtk_show_about_dialog(GTK_WINDOW(app->window),
                          "program-name", "Zxc Terminal",
                          "version", "1.1",
                          "comments", "A lightweight GTK + VTE terminal emulator.",
                          "authors", authors,
                          "website", "https://github.com/Hi7548388/Zxc-terminal",
                          NULL);
}

/* -----------------------------------------------------------
   Close window when shell exits
   ----------------------------------------------------------- */
static void
on_child_exited(VteTerminal *terminal, gint status, gpointer user_data)
{
    AppWidgets *app = user_data;
    gtk_window_close(GTK_WINDOW(app->window));
}

/* -----------------------------------------------------------
   Load config file
   ----------------------------------------------------------- */
static GKeyFile *
load_theme_config(void)
{
    gchar *path = g_build_filename(g_get_user_config_dir(),
                                   "zxc-terminal",
                                   "themes.conf",
                                   NULL);

    GKeyFile *kf = g_key_file_new();

    if (!g_key_file_load_from_file(kf, path, G_KEY_FILE_NONE, NULL)) {
        g_warning("Could not load theme config: %s", path);
    }

    g_free(path);
    return kf;
}

/* -----------------------------------------------------------
   GTK Application activate
   ----------------------------------------------------------- */
static void
activate(GtkApplication *app, gpointer user_data)
{
    AppWidgets *widgets = g_new0(AppWidgets, 1);

    /* Load theme config */
    widgets->theme_file = load_theme_config();

    /* Window */
    widgets->window = gtk_application_window_new(app);
    gtk_window_set_default_size(GTK_WINDOW(widgets->window), 800, 600);
    gtk_window_set_title(GTK_WINDOW(widgets->window), "Zxc Terminal");

    /* Header bar */
    widgets->header_bar = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(widgets->header_bar), "Zxc Terminal");
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(widgets->header_bar), TRUE);
    gtk_window_set_titlebar(GTK_WINDOW(widgets->window), widgets->header_bar);

    /* About button */
    GtkWidget *about_button = gtk_button_new_from_icon_name("help-about", GTK_ICON_SIZE_BUTTON);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(widgets->header_bar), about_button);
    g_signal_connect(about_button, "clicked", G_CALLBACK(on_about_clicked), widgets);

    /* Dark mode switch */
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *label = gtk_label_new("Light-Dark");
    widgets->dark_mode_switch = gtk_switch_new();

    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), widgets->dark_mode_switch, FALSE, FALSE, 0);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(widgets->header_bar), box);

    g_signal_connect(widgets->dark_mode_switch, "state-set",
                     G_CALLBACK(on_dark_mode_toggled), NULL);

    /* Theme combo box */
    GtkWidget *color_combo = gtk_combo_box_text_new();

    /* Load theme names dynamically */
    gsize length;
    gchar **groups = g_key_file_get_groups(widgets->theme_file, &length);

    for (gsize i = 0; i < length; i++)
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(color_combo), groups[i]);

    g_strfreev(groups);

    gtk_combo_box_set_active(GTK_COMBO_BOX(color_combo), 0);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(widgets->header_bar), color_combo);

    g_signal_connect(color_combo, "changed",
                     G_CALLBACK(on_color_scheme_changed), widgets);

    /* Terminal */
    widgets->terminal = vte_terminal_new();

    const char *default_shell = g_getenv("SHELL");
    if (!default_shell || default_shell[0] == '\0')
        default_shell = "/bin/bash";

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

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled), widgets->terminal);

    gtk_container_add(GTK_CONTAINER(widgets->window), scrolled);

    gtk_widget_show_all(widgets->window);
}

/* -----------------------------------------------------------
   main()
   ----------------------------------------------------------- */
int
main(int argc, char **argv)
{
    GtkApplication *app = gtk_application_new("com.example.GtkTerminal",
                                              G_APPLICATION_FLAGS_NONE);

    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}

