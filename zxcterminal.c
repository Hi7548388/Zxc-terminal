// ps7_term.c
// GTK+VTE "PowerShell 7-style" terminal for Linux

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

    // Main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "PowerShell 7 Terminal");
    gtk_window_set_default_size(GTK_WINDOW(window), 1100, 650);
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), NULL);

    // Layout box
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), box);

    // Scrolled VTE terminal
    VteTerminal *term = VTE_TERMINAL(vte_terminal_new());
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), GTK_WIDGET(term));
    gtk_box_pack_start(GTK_BOX(box), scroll, TRUE, TRUE, 0);

    // PowerShell 7-like colors
    GdkRGBA bg, fg;
    gdk_rgba_parse(&bg, "#012456");
    gdk_rgba_parse(&fg, "#f2f2f2");
    vte_terminal_set_colors(term, &fg, &bg, NULL, 0);

    // Optional palette
    GdkRGBA palette[16];
    gdk_rgba_parse(&palette[0],  "#0C0C0C");
    gdk_rgba_parse(&palette[1],  "#C50F1F");
    gdk_rgba_parse(&palette[2],  "#13A10E");
    gdk_rgba_parse(&palette[3],  "#C19C00");
    gdk_rgba_parse(&palette[4],  "#0037DA");
    gdk_rgba_parse(&palette[5],  "#881798");
    gdk_rgba_parse(&palette[6],  "#3A96DD");
    gdk_rgba_parse(&palette[7],  "#CCCCCC");
    gdk_rgba_parse(&palette[8],  "#767676");
    gdk_rgba_parse(&palette[9],  "#E74856");
    gdk_rgba_parse(&palette[10], "#16C60C");
    gdk_rgba_parse(&palette[11], "#F9F1A5");
    gdk_rgba_parse(&palette[12], "#3B78FF");
    gdk_rgba_parse(&palette[13], "#B4009E");
    gdk_rgba_parse(&palette[14], "#61D6D6");
    gdk_rgba_parse(&palette[15], "#F2F2F2");
    vte_terminal_set_colors(term, &fg, &bg, palette, 16);

    // Font
    PangoFontDescription *font_desc =
        pango_font_description_from_string("Cascadia Mono 11");
    vte_terminal_set_font(term, font_desc);
    pango_font_description_free(font_desc);

    // Behavior tweaks
    vte_terminal_set_audible_bell(term, FALSE);
    vte_terminal_set_cursor_blink_mode(term, VTE_CURSOR_BLINK_ON);
    vte_terminal_set_scrollback_lines(term, 10000);

    // SAFE, ESCAPE-FREE PROMPT
    const char *cmd =
        "export PS1='PS $(pwd) > '; exec bash -i";

    char *argv_shell[] = {
        "/bin/bash",
        "-i",
        "-c",
        (char *)cmd,
        NULL
    };

    const char *working_dir = g_get_home_dir();

    vte_terminal_spawn_async(
        term,
        VTE_PTY_DEFAULT,
        working_dir,
        argv_shell,
        NULL,
        G_SPAWN_DEFAULT,
        NULL, NULL,
        NULL,
        -1,
        NULL,
        NULL,
        NULL
    );

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
