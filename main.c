#include <gtk-3.0/gtk/gtk.h>
#include <vte/vte.h>

typedef struct {
    GtkWidget *window;
    GtkWidget *terminal;
    GtkWidget *header_bar;
    GtkWidget *dark_mode_switch;
} AppWidgets;

static void
apply_color_scheme(VteTerminal *terminal, const gchar *scheme)
{
    GdkRGBA fg, bg;

    /* -------------------- Light -------------------- */
    if (g_strcmp0(scheme, "Light") == 0) {
        gdk_rgba_parse(&fg, "#000000");
        gdk_rgba_parse(&bg, "#FFFFFF");
        vte_terminal_set_colors(terminal, &fg, &bg, NULL, 0);
    }

    /* -------------------- Dark -------------------- */
    else if (g_strcmp0(scheme, "Dark") == 0) {
        gdk_rgba_parse(&fg, "#FFFFFF");
        gdk_rgba_parse(&bg, "#000000");
        vte_terminal_set_colors(terminal, &fg, &bg, NULL, 0);
    }

    /* -------------------- Solarized -------------------- */
    else if (g_strcmp0(scheme, "Solarized") == 0) {
        static GdkRGBA solarized_palette[16];
        const char *hex[] = {
            "#073642", "#DC322F", "#859900", "#B58900",
            "#268BD2", "#D33682", "#2AA198", "#EEE8D5",
            "#002B36", "#CB4B16", "#586E75", "#657B83",
            "#839496", "#6C71C4", "#93A1A1", "#FDF6E3"
        };

        for (int i = 0; i < 16; i++)
            gdk_rgba_parse(&solarized_palette[i], hex[i]);

        gdk_rgba_parse(&fg, "#839496");
        gdk_rgba_parse(&bg, "#002B36");

        vte_terminal_set_colors(terminal, &fg, &bg, solarized_palette, 16);
    }

    /* -------------------- Dracula -------------------- */
    else if (g_strcmp0(scheme, "Dracula") == 0) {
        static GdkRGBA palette[16];
        const char *hex[] = {
            "#000000", "#FF5555", "#50FA7B", "#F1FA8C",
            "#BD93F9", "#FF79C6", "#8BE9FD", "#BFBFBF",
            "#4D4D4D", "#FF6E67", "#5AF78E", "#F4F99D",
            "#CAA9FA", "#FF92D0", "#9AEDFE", "#E6E6E6"
        };

        for (int i = 0; i < 16; i++)
            gdk_rgba_parse(&palette[i], hex[i]);

        gdk_rgba_parse(&fg, "#F8F8F2");
        gdk_rgba_parse(&bg, "#282A36");

        vte_terminal_set_colors(terminal, &fg, &bg, palette, 16);
    }

    /* -------------------- Gruvbox Dark -------------------- */
    else if (g_strcmp0(scheme, "Gruvbox Dark") == 0) {
        static GdkRGBA palette[16];
        const char *hex[] = {
            "#282828", "#CC241D", "#98971A", "#D79921",
            "#458588", "#B16286", "#689D6A", "#A89984",
            "#928374", "#FB4934", "#B8BB26", "#FABD2F",
            "#83A598", "#D3869B", "#8EC07C", "#EBDBB2"
        };

        for (int i = 0; i < 16; i++)
            gdk_rgba_parse(&palette[i], hex[i]);

        gdk_rgba_parse(&fg, "#EBDBB2");
        gdk_rgba_parse(&bg, "#282828");

        vte_terminal_set_colors(terminal, &fg, &bg, palette, 16);
    }

    /* -------------------- Nord -------------------- */
    else if (g_strcmp0(scheme, "Nord") == 0) {
        static GdkRGBA palette[16];
        const char *hex[] = {
            "#3B4252", "#BF616A", "#A3BE8C", "#EBCB8B",
            "#81A1C1", "#B48EAD", "#88C0D0", "#E5E9F0",
            "#4C566A", "#BF616A", "#A3BE8C", "#EBCB8B",
            "#81A1C1", "#B48EAD", "#8FBCBB", "#ECEFF4"
        };

        for (int i = 0; i < 16; i++)
            gdk_rgba_parse(&palette[i], hex[i]);

        gdk_rgba_parse(&fg, "#D8DEE9");
        gdk_rgba_parse(&bg, "#2E3440");

        vte_terminal_set_colors(terminal, &fg, &bg, palette, 16);
    }

    /* -------------------- Monokai -------------------- */
    else if (g_strcmp0(scheme, "Monokai") == 0) {
        static GdkRGBA palette[16];
        const char *hex[] = {
            "#272822", "#F92672", "#A6E22E", "#F4BF75",
            "#66D9EF", "#AE81FF", "#A1EFE4", "#F8F8F2",
            "#75715E", "#F92672", "#A6E22E", "#F4BF75",
            "#66D9EF", "#AE81FF", "#A1EFE4", "#F9F8F5"
        };

        for (int i = 0; i < 16; i++)
            gdk_rgba_parse(&palette[i], hex[i]);

        gdk_rgba_parse(&fg, "#F8F8F2");
        gdk_rgba_parse(&bg, "#272822");

        vte_terminal_set_colors(terminal, &fg, &bg, palette, 16);
    }
}

static void
on_color_scheme_changed(GtkComboBox *combo, gpointer user_data)
{
    AppWidgets *app = (AppWidgets *)user_data;
    gchar *scheme = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));

    if (scheme) {
        apply_color_scheme(VTE_TERMINAL(app->terminal), scheme);
        g_free(scheme);
    }
}

static void
on_dark_mode_toggled(GtkSwitch *sw, gboolean state, gpointer user_data)
{
    GtkSettings *settings = gtk_settings_get_default();
    g_object_set(settings, "gtk-application-prefer-dark-theme", state, NULL);
}

static void
on_about_clicked(GtkButton *button, gpointer user_data)
{
    AppWidgets *app = (AppWidgets *)user_data;
    const gchar *authors[] = { "Zap735335", NULL };

    gtk_show_about_dialog(GTK_WINDOW(app->window),
                          "program-name", "Zxc Terminal",
                          "version", "1.1",
                          "comments", "A lightweight GTK + VTE terminal emulator.",
                          "authors", authors,
                          "website", "https://github.com/Hi7548388/Zxc-terminal",
                          NULL);
}

static void
on_child_exited(VteTerminal *terminal, gint status, gpointer user_data)
{
    AppWidgets *app = (AppWidgets *)user_data;
    gtk_window_close(GTK_WINDOW(app->window));
}

static void
activate(GtkApplication *app, gpointer user_data)
{
    AppWidgets *widgets = g_new0(AppWidgets, 1);

    widgets->window = gtk_application_window_new(app);
    gtk_window_set_default_size(GTK_WINDOW(widgets->window), 800, 600);
    gtk_window_set_title(GTK_WINDOW(widgets->window), "Zxc Terminal");

    widgets->header_bar = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(widgets->header_bar), "Zxc Terminal");
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(widgets->header_bar), TRUE);
    gtk_window_set_titlebar(GTK_WINDOW(widgets->window), widgets->header_bar);

    GtkWidget *about_button = gtk_button_new_from_icon_name("help-about", GTK_ICON_SIZE_BUTTON);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(widgets->header_bar), about_button);
    g_signal_connect(about_button, "clicked", G_CALLBACK(on_about_clicked), widgets);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *label = gtk_label_new("Light-Dark");
    widgets->dark_mode_switch = gtk_switch_new();

    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), widgets->dark_mode_switch, FALSE, FALSE, 0);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(widgets->header_bar), box);

    g_signal_connect(widgets->dark_mode_switch, "state-set",
                     G_CALLBACK(on_dark_mode_toggled), NULL);

    /* -------------------- Color Scheme Combo -------------------- */
    GtkWidget *color_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(color_combo), "Light");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(color_combo), "Dark");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(color_combo), "Solarized");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(color_combo), "Dracula");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(color_combo), "Gruvbox Dark");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(color_combo), "Nord");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(color_combo), "Monokai");
    gtk_combo_box_set_active(GTK_COMBO_BOX(color_combo), 0);

    gtk_header_bar_pack_end(GTK_HEADER_BAR(widgets->header_bar), color_combo);

    g_signal_connect(color_combo, "changed",
                     G_CALLBACK(on_color_scheme_changed), widgets);

    /* -------------------- Terminal -------------------- */
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

int
main(int argc, char **argv)
{
    GtkApplication *app = gtk_application_new("com.example.GtkTerminal", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}

