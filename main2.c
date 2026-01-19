/*  Zxc Terminal — Full Featured Version with Themes + Closeable Tabs
 *
 *  Features:
 *   - Tabs with close buttons (GNOME Terminal style)
 *   - Drag‑and‑drop file support
 *   - Font selector dialog
 *   - Transparency slider
 *   - Theme loader (fg/bg/palette) from ~/.config/zxc-terminal/themes.conf
 *   - Theme combo box to switch themes
 *   - Dark mode toggle
 *   - About dialog
 */

#include <gtk-3.0/gtk/gtk.h>
#include <vte/vte.h>

typedef struct {
    GtkWidget *window;
    GtkWidget *header_bar;
    GtkWidget *notebook;
    GtkWidget *dark_mode_switch;
    GKeyFile  *theme_file;
    GdkRGBA    current_bg;
    gchar     *current_theme;
    double     current_alpha;
} AppWidgets;

/* -----------------------------------------------------------
   Load theme from config
   ----------------------------------------------------------- */
static gboolean
load_theme(GKeyFile *kf, const char *name, GdkRGBA *fg, GdkRGBA *bg, GdkRGBA palette[16])
{
    if (!kf || !name)
        return FALSE;

    g_autofree gchar *fg_hex = g_key_file_get_string(kf, name, "fg", NULL);
    g_autofree gchar *bg_hex = g_key_file_get_string(kf, name, "bg", NULL);
    g_autofree gchar *palette_hex = g_key_file_get_string(kf, name, "palette", NULL);

    if (!fg_hex || !bg_hex) {
        g_warning("Theme '%s' missing fg/bg", name);
        return FALSE;
    }

    if (!gdk_rgba_parse(fg, fg_hex) || !gdk_rgba_parse(bg, bg_hex)) {
        g_warning("Theme '%s' has invalid fg/bg color", name);
        return FALSE;
    }

    for (int i = 0; i < 16; i++)
        gdk_rgba_parse(&palette[i], "#000000");

    if (palette_hex) {
        gchar **colors = g_strsplit(palette_hex, ",", 16);
        for (int i = 0; i < 16 && colors[i]; i++)
            gdk_rgba_parse(&palette[i], colors[i]);
        g_strfreev(colors);
    }

    return TRUE;
}

/* -----------------------------------------------------------
   Drag‑and‑drop file support
   ----------------------------------------------------------- */
static void
on_drag_data_received(GtkWidget *widget,
                      GdkDragContext *context,
                      gint x, gint y,
                      GtkSelectionData *data,
                      guint info, guint time,
                      gpointer user_data)
{
    (void)context; (void)x; (void)y; (void)info; (void)time; (void)user_data;

    gchar **uris = gtk_selection_data_get_uris(data);
    if (!uris)
        return;

    for (int i = 0; uris[i]; i++) {
        gchar *path = g_filename_from_uri(uris[i], NULL, NULL);
        if (path) {
            vte_terminal_feed_child(VTE_TERMINAL(widget), path, strlen(path));
            vte_terminal_feed_child(VTE_TERMINAL(widget), " ", 1);
            g_free(path);
        }
    }

    g_strfreev(uris);
}

/* -----------------------------------------------------------
   Close tab button callback
   ----------------------------------------------------------- */
static void
on_tab_close_clicked(GtkButton *btn, gpointer user_data)
{
    GtkNotebook *notebook = GTK_NOTEBOOK(user_data);
    GtkWidget *tab_box = gtk_widget_get_parent(GTK_WIDGET(btn));

    gint pages = gtk_notebook_get_n_pages(notebook);
    for (int i = 0; i < pages; i++) {
        GtkWidget *label = gtk_notebook_get_tab_label(notebook,
                                                      gtk_notebook_get_nth_page(notebook, i));
        if (label == tab_box) {
            gtk_notebook_remove_page(notebook, i);
            return;
        }
    }
}

/* Forward declaration */
static void on_theme_changed(GtkComboBox *combo, gpointer user_data);

/* -----------------------------------------------------------
   Create a new terminal tab (with close button)
   ----------------------------------------------------------- */
static GtkWidget *
create_terminal_tab(AppWidgets *app)
{
    GtkWidget *terminal = vte_terminal_new();
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), terminal);

    vte_terminal_set_scrollback_lines(VTE_TERMINAL(terminal), 5000);
    vte_terminal_set_bold_is_bright(VTE_TERMINAL(terminal), TRUE);
    vte_terminal_set_cursor_shape(VTE_TERMINAL(terminal), VTE_CURSOR_SHAPE_BLOCK);

    if (app->theme_file && app->current_theme) {
        GdkRGBA fg, bg;
        GdkRGBA palette[16];

        if (load_theme(app->theme_file, app->current_theme, &fg, &bg, palette)) {
            bg.alpha = app->current_alpha;
            vte_terminal_set_colors(VTE_TERMINAL(terminal), &fg, &bg, palette, 16);
            app->current_bg = bg;
        }
    } else {
        GdkRGBA bg = app->current_bg;
        bg.alpha = app->current_alpha;
        vte_terminal_set_color_background(VTE_TERMINAL(terminal), &bg);
    }

    gtk_drag_dest_set(terminal, GTK_DEST_DEFAULT_ALL, NULL, 0, GDK_ACTION_COPY);
    GtkTargetEntry targets[] = { { "text/uri-list", 0, 0 } };
    gtk_drag_dest_set_target_list(terminal, gtk_target_list_new(targets, 1));
    g_signal_connect(terminal, "drag-data-received",
                     G_CALLBACK(on_drag_data_received), NULL);

    const char *shell = g_getenv("SHELL");
    if (!shell || shell[0] == '\0')
        shell = "/bin/bash";
    char *argv[] = { (char *)shell, NULL };

    vte_terminal_spawn_async(
        VTE_TERMINAL(terminal),
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

    /* -------------------------
       Create tab header
       ------------------------- */
    GtkWidget *tab_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    GtkWidget *tab_label = gtk_label_new("Tab");
    GtkWidget *close_btn = gtk_button_new_from_icon_name("window-close", GTK_ICON_SIZE_MENU);

    gtk_button_set_relief(GTK_BUTTON(close_btn), GTK_RELIEF_NONE);
    gtk_widget_set_focus_on_click(close_btn, FALSE);

    gtk_box_pack_start(GTK_BOX(tab_box), tab_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(tab_box), close_btn, FALSE, FALSE, 0);

    gint page = gtk_notebook_append_page(GTK_NOTEBOOK(app->notebook), scroll, tab_box);

    g_signal_connect(close_btn, "clicked",
                     G_CALLBACK(on_tab_close_clicked),
                     app->notebook);

    gtk_widget_show_all(tab_box);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(app->notebook), page);

    return terminal;
}

/* -----------------------------------------------------------
   New tab button
   ----------------------------------------------------------- */
static void
on_new_tab_clicked(GtkButton *btn, gpointer user_data)
{
    (void)btn;
    create_terminal_tab(user_data);
}

/* -----------------------------------------------------------
   Font selector
   ----------------------------------------------------------- */
static void
on_font_clicked(GtkButton *btn, gpointer user_data)
{
    (void)btn;

    AppWidgets *app = user_data;

    GtkWidget *dialog = gtk_font_chooser_dialog_new("Select Terminal Font",
                                                    GTK_WINDOW(app->window));

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        gchar *font = gtk_font_chooser_get_font(GTK_FONT_CHOOSER(dialog));
        PangoFontDescription *desc = pango_font_description_from_string(font);

        gint pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(app->notebook));
        for (int i = 0; i < pages; i++) {
            GtkWidget *scroll = gtk_notebook_get_nth_page(GTK_NOTEBOOK(app->notebook), i);
            GtkWidget *term = gtk_bin_get_child(GTK_BIN(scroll));
            vte_terminal_set_font(VTE_TERMINAL(term), desc);
        }

        pango_font_description_free(desc);
        g_free(font);
    }

    gtk_widget_destroy(dialog);
}

/* -----------------------------------------------------------
   Transparency slider
   ----------------------------------------------------------- */
static void
on_transparency_changed(GtkRange *range, gpointer user_data)
{
    AppWidgets *app = user_data;
    double alpha = gtk_range_get_value(range);
    app->current_alpha = alpha;

    GdkRGBA bg = app->current_bg;
    bg.alpha = alpha;

    gint pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(app->notebook));
    for (int i = 0; i < pages; i++) {
        GtkWidget *scroll = gtk_notebook_get_nth_page(GTK_NOTEBOOK(app->notebook), i);
        GtkWidget *term = gtk_bin_get_child(GTK_BIN(scroll));
        vte_terminal_set_color_background(VTE_TERMINAL(term), &bg);
    }
}

/* -----------------------------------------------------------
   Dark mode toggle
   ----------------------------------------------------------- */
static void
on_dark_mode_toggled(GtkSwitch *sw, gboolean state, gpointer user_data)
{
    (void)sw; (void)user_data;

    GtkSettings *settings = gtk_settings_get_default();
    g_object_set(settings, "gtk-application-prefer-dark-theme", state, NULL);
}

/* -----------------------------------------------------------
   About dialog
   ----------------------------------------------------------- */
static void
on_about_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;

    AppWidgets *app = user_data;
    const gchar *authors[] = { "Zap735335", NULL };

    gtk_show_about_dialog(GTK_WINDOW(app->window),
                          "program-name", "Zxc Terminal",
                          "version", "2.0",
                          "comments", "A full-featured GTK + VTE terminal emulator.",
                          "authors", authors,
                          "website", "https://github.com/Hi7548388/Zxc-terminal",
                          NULL);
}

/* -----------------------------------------------------------
   Theme combo box changed
   ----------------------------------------------------------- */
static void
on_theme_changed(GtkComboBox *combo, gpointer user_data)
{
    AppWidgets *app = user_data;
    gchar *scheme = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));

    if (!scheme || !app->theme_file) {
        g_free(scheme);
        return;
    }

    GdkRGBA fg, bg;
    GdkRGBA palette[16];

    if (!load_theme(app->theme_file, scheme, &fg, &bg, palette)) {
        g_free(scheme);
        return;
    }

    g_free(app->current_theme);
    app->current_theme = g_strdup(scheme);

    bg.alpha = app->current_alpha;
    app->current_bg = bg;

    gint pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(app->notebook));
    for (int i = 0; i < pages; i++) {
        GtkWidget *scroll = gtk_notebook_get_nth_page(GTK_NOTEBOOK(app->notebook), i);
        GtkWidget *term = gtk_bin_get_child(GTK_BIN(scroll));
        vte_terminal_set_colors(VTE_TERMINAL(term), &fg, &bg, palette, 16);
    }

    g_free(scheme);
}

/* -----------------------------------------------------------
   Load theme config
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
        g_free(path);
        g_key_file_free(kf);
        return NULL;
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
    (void)user_data;

    AppWidgets *widgets = g_new0(AppWidgets, 1);

    widgets->theme_file   = load_theme_config();
    widgets->current_theme = NULL;
    widgets->current_alpha = 1.0;
    gdk_rgba_parse(&widgets->current_bg, "#000000");
    widgets->current_bg.alpha = 1.0;

    if (widgets->theme_file) {
        gsize length = 0;
        gchar **groups = g_key_file_get_groups(widgets->theme_file, &length);
        if (length > 0) {
            widgets->current_theme = g_strdup(groups[0]);

            GdkRGBA fg, bg;
            GdkRGBA palette[16];
            if (load_theme(widgets->theme_file, widgets->current_theme, &fg, &bg, palette)) {
                bg.alpha = widgets->current_alpha;
                widgets->current_bg = bg;
            }
        }
        g_strfreev(groups);
    }

    widgets->window = gtk_application_window_new(app);
    gtk_window_set_default_size(GTK_WINDOW(widgets->window), 900, 600);
    gtk_window_set_title(GTK_WINDOW(widgets->window), "Zxc Terminal");

    widgets->header_bar = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(widgets->header_bar), "Zxc Terminal");
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(widgets->header_bar), TRUE);
    gtk_window_set_titlebar(GTK_WINDOW(widgets->window), widgets->header_bar);

    GtkWidget *new_tab_btn = gtk_button_new_from_icon_name("tab-new", GTK_ICON_SIZE_BUTTON);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(widgets->header_bar), new_tab_btn);
    g_signal_connect(new_tab_btn, "clicked", G_CALLBACK(on_new_tab_clicked), widgets);

    GtkWidget *font_btn = gtk_button_new_from_icon_name("preferences-desktop-font", GTK_ICON_SIZE_BUTTON);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(widgets->header_bar), font_btn);
    g_signal_connect(font_btn, "clicked", G_CALLBACK(on_font_clicked), widgets);

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

    GtkWidget *transparency = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.0, 1.0, 0.01);
    gtk_widget_set_tooltip_text(transparency, "Terminal Transparency");
    gtk_range_set_value(GTK_RANGE(transparency), widgets->current_alpha);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(widgets->header_bar), transparency);
    g_signal_connect(transparency, "value-changed",
                     G_CALLBACK(on_transparency_changed), widgets);

    GtkWidget *theme_combo = gtk_combo_box_text_new();
    if (widgets->theme_file) {
        gsize length = 0;
        gchar **groups = g_key_file_get_groups(widgets->theme_file, &length);
        for (gsize i = 0; i < length; i++)
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(theme_combo), groups[i]);
        g_strfreev(groups);
    }
    gtk_header_bar_pack_end(GTK_HEADER_BAR(widgets->header_bar), theme_combo);
    g_signal_connect(theme_combo, "changed",
                     G_CALLBACK(on_theme_changed), widgets);

    if (widgets->current_theme && widgets->theme_file) {
        gsize length = 0;
        gchar **groups = g_key_file_get_groups(widgets->theme_file, &length);
        for (gsize i = 0; i < length; i++) {
            if (g_strcmp0(groups[i], widgets->current_theme) == 0) {
                gtk_combo_box_set_active(GTK_COMBO_BOX(theme_combo), (gint)i);
                break;
            }
        }
        g_strfreev(groups);
    } else {
        gtk_combo_box_set_active(GTK_COMBO_BOX(theme_combo), 0);
    }

    widgets->notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(widgets->window), widgets->notebook);

    create_terminal_tab(widgets);

    gtk_widget_show_all(widgets->window);
}

/* -----------------------------------------------------------
   main()
   ----------------------------------------------------------- */
int
main(int argc, char **argv)
{
    GtkApplication *app = gtk_application_new("com.example.ZxcTerminal",
                                              G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
