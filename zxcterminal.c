// ps_term.c
// Minimal GTK+VTE "PowerShell-style" terminal for Linux

#include <gtk-3.0/gtk/gtk.h>
#include <vte/vte.h>

static void
on_window_destroy(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
}

int
main(int argc, char **argv)
{
    gtk_init(&argc, &argv);

    // Create main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "PS-Term");
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 600);

    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), NULL);

    // Create a box to hold the terminal (and later a toolbar, tabs, etc.)
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), box);

    // Create VTE terminal
    VteTerminal *term = VTE_TERMINAL(vte_terminal_new());
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), GTK_WIDGET(term));
    gtk_box_pack_start(GTK_BOX(box), scroll, TRUE, TRUE, 0);

    // PowerShell-like color scheme
    // Background: very dark blue, Foreground: light gray
    GdkRGBA bg, fg;
    gdk_rgba_parse(&bg, "#001533");   // dark blue-ish
    gdk_rgba_parse(&fg, "#dcdcdc");   // light gray

    vte_terminal_set_colors(term, &fg, &bg, NULL, 0);

    // Optional: tweak font (adjust to taste)
    PangoFontDescription *font_desc = pango_font_description_from_string("Fira Code 11");
    vte_terminal_set_font(term, font_desc);
    pango_font_description_free(font_desc);

    // Optional: disable audible bell, enable cursor blinking, etc.
    vte_terminal_set_audible_bell(term, FALSE);
    vte_terminal_set_cursor_blink_mode(term, VTE_CURSOR_BLINK_ON);
    vte_terminal_set_scrollback_lines(term, 10000);

    // Build a PowerShell-style prompt via PS1
    // Example: "PS /home/scott> "
    // \w = current working dir, \u = user, \h = host
    // You can get fancier with colors using ANSI escapes.
    const char *ps1 =
        "PS \\w> ";

    // Environment: inherit current env, but override PS1
    // Easiest: build a small env array with PS1 and PATH, then rely on /bin/bash -i
    // For simplicity, we just set PS1 in the command line.
    char *argv_shell[] = {
        "/bin/bash",
        "-i",
        "-c",
        // Use single quotes around PS1 to avoid expansion; then exec bash again
        // so interactive features still work.
        // This spawns: bash -i -c "export PS1='PS \w> '; exec bash -i"
        "export PS1='PS \\w> '; exec bash -i",
        NULL
    };

    // Working directory
    const char *working_dir = g_get_home_dir();

    // Spawn the shell
    GError *error = NULL;
    vte_terminal_spawn_async(
        term,
        VTE_PTY_DEFAULT,
        working_dir,          // working directory
        argv_shell,           // argv
        NULL,                 // envv (inherit)
        G_SPAWN_DEFAULT,
        NULL, NULL,           // child_setup
        NULL,                 // child pid
        -1,                   // timeout
        NULL,                 // cancellable
        NULL,                 // callback
        NULL                  // user_data
    );

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
