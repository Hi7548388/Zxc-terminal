// ps7_term.c
// GTK+VTE "PowerShell 7-style" terminal with tabs and top menu

#include <gtk-3.0/gtk/gtk.h>
#include <vte/vte.h>

// Forward declarations
static void spawn_shell(VteTerminal *term);
static void create_tab(GtkNotebook *notebook);
static void close_current_tab(GtkNotebook *notebook);

// ------------------------------------------------------------
// Close button on tab
// ------------------------------------------------------------
static void on_close_button(GtkButton *btn, gpointer user_data)
{
    GtkNotebook *notebook = GTK_NOTEBOOK(user_data);
    GtkWidget *tab_box = gtk_widget_get_parent(GTK_WIDGET(btn));
    GtkWidget *page = gtk_widget_get_parent(tab_box);

    gint page_num = gtk_notebook_page_num(notebook, page);
    if (page_num != -1)
        gtk_notebook_remove_page(notebook, page_num);
}

// ------------------------------------------------------------
// Spawn shell with safe PS1
// ------------------------------------------------------------
static void spawn_shell(VteTerminal *term)
{
    const char *cmd =
        "export PS1='PS $(pwd) > '; exec bash -i";

    char *argv_shell[] = { "/bin/bash", "-i", "-c", (char*)cmd, NULL };

    vte_terminal_spawn_async(
        term,
        VTE_PTY_DEFAULT,
        g_get_home_dir(),
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
}

// ------------------------------------------------------------
// Create a new tab
// ------------------------------------------------------------
static void create_tab(GtkNotebook *notebook)
{
    // Terminal
    VteTerminal *term = VTE_TERMINAL(vte_terminal_new());

    // Scroll container
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), GTK_WIDGET(term));

    // Tab label box
    GtkWidget *tab_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);

    GtkWidget *label = gtk_label_new("Terminal");
    gtk_box_pack_start(GTK_BOX(tab_box), label, FALSE, FALSE, 0);

    // Close button
    GtkWidget *close_btn = gtk_button_new_with_label("×");
    gtk_button_set_relief(GTK_BUTTON(close_btn), GTK_RELIEF_NONE);
    gtk_widget_set_focus_on_click(close_btn, FALSE);
    gtk_box_pack_start(GTK_BOX(tab_box), close_btn, FALSE, FALSE, 0);

    // Add page
    gtk_notebook_append_page(notebook, scroll, tab_box);
    gtk_widget_show_all(scroll);

    // Close button callback
    g_signal_connect(close_btn, "clicked",
                     G_CALLBACK(on_close_button), notebook);

    // Spawn shell
    spawn_shell(term);

    // Switch to new tab
    gint n_pages = gtk_notebook_get_n_pages(notebook);
    gtk_notebook_set_current_page(notebook, n_pages - 1);
}

// ------------------------------------------------------------
// Close current tab (from menu)
// ------------------------------------------------------------
static void close_current_tab(GtkNotebook *notebook)
{
    gint page = gtk_notebook_get_current_page(notebook);
    if (page != -1)
        gtk_notebook_remove_page(notebook, page);
}

// ------------------------------------------------------------
// Menu callbacks
// ------------------------------------------------------------
static void on_new_tab_activate(GtkMenuItem *item, gpointer user_data)
{
    GtkNotebook *notebook = GTK_NOTEBOOK(user_data);
    create_tab(notebook);
}

static void on_close_tab_activate(GtkMenuItem *item, gpointer user_data)
{
    GtkNotebook *notebook = GTK_NOTEBOOK(user_data);
    close_current_tab(notebook);
}

static void on_quit_activate(GtkMenuItem *item, gpointer user_data)
{
    gtk_main_quit();
}

// ------------------------------------------------------------
// Window destroy
// ------------------------------------------------------------
static void on_window_destroy(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
}

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------
int main(int argc, char **argv)
{
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Zxc Terminal");
    gtk_window_set_default_size(GTK_WINDOW(window), 1100, 650);
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), NULL);

    // Main vertical box
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Menu bar
    GtkWidget *menubar = gtk_menu_bar_new();
    GtkWidget *file_menu = gtk_menu_new();
    GtkWidget *file_item = gtk_menu_item_new_with_mnemonic("_File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);

    GtkWidget *mi_new_tab = gtk_menu_item_new_with_label("New Tab");
    GtkWidget *mi_close_tab = gtk_menu_item_new_with_label("Close Tab");
    GtkWidget *mi_quit = gtk_menu_item_new_with_label("Exit");

    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), mi_new_tab);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), mi_close_tab);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), mi_quit);

    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file_item);

    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

    // Notebook (tabs)
    GtkWidget *notebook = gtk_notebook_new();
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);

    // Connect menu actions
    g_signal_connect(mi_new_tab, "activate",
                     G_CALLBACK(on_new_tab_activate), notebook);
    g_signal_connect(mi_close_tab, "activate",
                     G_CALLBACK(on_close_tab_activate), notebook);
    g_signal_connect(mi_quit, "activate",
                     G_CALLBACK(on_quit_activate), NULL);

    // First tab
    create_tab(GTK_NOTEBOOK(notebook));

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
