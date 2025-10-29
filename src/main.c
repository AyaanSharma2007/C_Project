#include <gtk/gtk.h>
#include "sidebar.h"  // <-- INCLUDE OUR NEW HEADER

// Dark mode callback is unchanged
static gboolean is_dark_mode = FALSE;
static void on_toggle_button_clicked(GtkButton *button, gpointer user_data) {
    is_dark_mode = !is_dark_mode;
    GtkSettings *settings = gtk_settings_get_default();
    g_object_set(settings, "gtk-application-prefer-dark-theme", is_dark_mode, NULL);
    
    if (is_dark_mode) {
        gtk_button_set_label(button, "Light Mode");
    } else {
        gtk_button_set_label(button, "Dark Mode");
    }
}

// Window close callback is unchanged
static void on_destroy(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}

int main(int argc, char **argv) {
    GtkWidget *window;
    GtkWidget *main_vbox;
    GtkWidget *header_box;
    GtkWidget *header_label;
    GtkWidget *toggle_button;
    GtkWidget *main_paned;
    
    // NEW sidebar and content widgets
    GtkWidget *sidebar;
    GtkWidget *content_label; // Right pane is now a content area

    GtkCssProvider *cssProvider;
    GdkDisplay *display;
    GdkScreen *screen;

    gtk_init(&argc, &argv);

    // 1. Create Window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "GyattHub");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(on_destroy), NULL);

    // 2. Create Header (unchanged)
    main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    header_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_name(header_box, "header");

    header_label = gtk_label_new("GyattHub");
    gtk_widget_set_halign(header_label, GTK_ALIGN_START);
    gtk_widget_set_name(header_label, "header-label");

    toggle_button = gtk_button_new_with_label("Dark Mode");
    g_signal_connect(toggle_button, "clicked", G_CALLBACK(on_toggle_button_clicked), NULL);

    gtk_box_pack_start(GTK_BOX(header_box), header_label, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(header_box), toggle_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), header_box, FALSE, FALSE, 0);

    // 3. Create the resizable pane (unchanged)
    main_paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(main_vbox), main_paned, TRUE, TRUE, 0);

    // 4. NEW: Create and add the sidebar
    // This one function call creates the button, list, and all callbacks!
    sidebar = create_sidebar(GTK_WINDOW(window));
    
    // Add margins to the sidebar
    gtk_widget_set_margin_start(sidebar, 10);
    gtk_widget_set_margin_bottom(sidebar, 10);
    gtk_widget_set_margin_top(sidebar, 10);
    
    gtk_paned_add1(GTK_PANED(main_paned), sidebar);
    gtk_widget_set_size_request(sidebar, 250, -1);

    // 5. NEW: Create the Right Pane (Content Area)
    // Since the upload button moved, the right pane is now a content area.
    // We'll add the "Gyatt world" label here.
    content_label = gtk_label_new("Gyatt world");
    gtk_widget_set_halign(content_label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(content_label, GTK_ALIGN_CENTER);
    gtk_widget_set_name(content_label, "big-label"); // Reuse the big font CSS

    gtk_paned_add2(GTK_PANED(main_paned), content_label);

    // 6. Add main_vbox to window (unchanged)
    gtk_container_add(GTK_CONTAINER(window), main_vbox);

    // 7. Load CSS (Updated for new button)
    cssProvider = gtk_css_provider_new();
    // Inside your main() function in main.c

    // 7. Load CSS (Updated for new button and list font)
    cssProvider = gtk_css_provider_new();
    const char *css_data =
        "#header {"
        "    background-color: @theme_fg_color;"
        "    padding: 10px;"
        "}"
        "#header-label {"
        "    color: @theme_bg_color;"
        "    font-size: 30px;"
        "}"
        "#big-label {"
        "    font-size: 30px;"
        "}"
        "#sidebar-browse-button {"
        "    border-radius: 0;"
        "    padding: 8px;"
        "}"
        "#sidebar-browse-button:hover {"
        "    background-image: none;"
        "    background-color: @theme_hover_color;"
        "}"
        ""
        "/* --- NEW: Style the file list labels --- */"
        "#file-list-box row label {"
        "    font-size: 20px; /* Slightly larger font */"
        "    padding-top: 2px; /* Add a little space */"
        "    padding-bottom: 2px;"
        "}";

    gtk_css_provider_load_from_data(cssProvider, css_data, -1, NULL);

    gtk_css_provider_load_from_data(cssProvider, css_data, -1, NULL);

    // 8. Apply CSS (unchanged)
    display = gdk_display_get_default();
    screen = gdk_display_get_default_screen(display);
    gtk_style_context_add_provider_for_screen(screen,
                                              GTK_STYLE_PROVIDER(cssProvider),
                                              GTK_STYLE_PROVIDER_PRIORITY_USER);

    // 9. Show and Run (unchanged)
    gtk_widget_show_all(window);
    g_object_unref(cssProvider);
    gtk_main();

    return 0;
}