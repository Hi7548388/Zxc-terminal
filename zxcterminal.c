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
    // Background: near-black with blue tint
    // Foreground: light gray
    GdkRGBA bg, fg;
    gdk_rgba_parse(&bg, "#012456");   // PS7-ish dark blue
    gdk_rgba_parse(&fg, "#f2f2f2");   // light foreground
    vte_terminal_set_colors(term, &fg, &bg, NULL, 0);

    // Optional: tweak palette to be closer to PS7 / Windows Terminal
    GdkRGBA palette[16];
    gdk_rgba_parse(&palette[0],  "#0C0C0C"); // black
    gdk_rgba_parse(&palette[1],  "#C50F1F"); // dark red
    gdk_rgba_parse(&palette[2],  "#13A10E"); // dark green
    gdk_rgba_parse(&palette[3],  "#C19C00"); // dark yellow
    gdk_rgba_parse(&palette[4],  "#0037DA"); // dark blue
    gdk_rgba_parse(&palette[5],  "#881798"); // dark magenta
    gdk_rgba_parse(&palette[6],  "#3A96DD"); // dark cyan
    gdk_rgba_parse(&palette[7],  "#CCCCCC"); // gray
    gdk_rgba_parse(&palette[8],  "#767676"); // dark gray
    gdk_rgba_parse(&palette[9],  "#E74856"); // red
    gdk_rgba_parse(&palette[10], "#16C60C"); // green
    gdk_rgba_parse(&palette[11], "#F9F1A5"); // yellow
    gdk_rgba_parse(&palette[12], "#3B78FF"); // blue
    gdk_rgba_parse(&palette[13], "#B4009E"); // magenta
    gdk_rgba_parse(&palette[14], "#61D6D6"); // cyan
    gdk_rgba_parse(&palette[15], "#F2F2F2"); // white
    vte_terminal_set_colors(term, &fg, &bg, palette, 16);

    // Font (similar to what many use with PS7)
    PangoFontDescription *font_desc =
        pango_font_description_from_string("Cascadia Mono 11");
    vte_terminal_set_font(term, font_desc);
    pango_font_description_free(font_desc);

    // Behavior tweaks
    vte_terminal_set_audible_bell(term, FALSE);
    vte_terminal_set_cursor_blink_mode(term, VTE_CURSOR_BLINK_ON);
    vte_terminal_set_scrollback_lines(term, 10000);

    // PowerShell 7-style prompt (approximation in bash)
    //
    // Example:
    //   PS /home/scott>
    //
    // With colors:
    //   "PS" in cyan, path in gray, ">" in white.
    //
    // ANSI:
    //   \e[38;5;45m  -> cyan-ish
    //   \e[38;5;250m -> gray
    //   \e[0m        -> reset
    //
    // We export PS1 then exec bash -i so readline etc. still work.
    const char *cmd =
        "export PS1='\

\[\\e[38;5;45m\\]

PS "
        "\

\[\\e[38;5;250m\\]

\\w"
        "\

\[\\e[0m\\]

> '; "
        "exec bash -i";

    char *argv_shell[] = {
        "/bin/bash",
        "-i",
        "-c",
        (char *)cmd,
        NULL
    };

    const char *working_dir = g_get_home_dir();
    GError *error = NULL;

    vte_terminal_spawn_async(
        term,
        VTE_PTY_DEFAULT,
        working_dir,
        argv_shell,
        NULL,                 // inherit env
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
