#include "sidebar.h"
#include <glib/gstdio.h> // For g_path_get_basename

// This struct is a "helper" to pass *both* the window and list
// to our callback function, since g_signal_connect only takes one 'data' pointer.
typedef struct {
    GtkWindow *parent_window;
    GtkWidget *list_box;
} SidebarCallbackData;

// This callback function is private to sidebar.c
static void on_browse_clicked(GtkButton *button, gpointer user_data) {
    SidebarCallbackData *data = (SidebarCallbackData *)user_data;
    
    // 1. Create the File Chooser Dialog
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Open File",
        data->parent_window, // Use the parent window we passed in
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Open", GTK_RESPONSE_ACCEPT,
        NULL
    );

    // 2. Run the dialog
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));

    // 3. Check response and update list
    if (response == GTK_RESPONSE_ACCEPT) {
        char *full_path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        char *basename = g_path_get_basename(full_path);

        // Create a new row (a label)
        GtkWidget *list_row_label = gtk_label_new(basename);
        gtk_widget_set_halign(list_row_label, GTK_ALIGN_START);

        // Add the new row to our list_box
        gtk_list_box_insert(GTK_LIST_BOX(data->list_box), list_row_label, -1);
        gtk_widget_show(list_row_label);

        g_free(basename);
        g_free(full_path);
    }

    // 4. Destroy the dialog
    gtk_widget_destroy(dialog);
}

// This is the public function defined in sidebar.h
GtkWidget *create_sidebar(GtkWindow *parent_window) {
    // 1. Create the main vertical box for the sidebar
    GtkWidget *sidebar_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    // 2. Create the "square" browse button with an icon
    GtkWidget *browse_button = gtk_button_new_from_icon_name(
        "document-open-symbolic", // A standard file-open icon
        GTK_ICON_SIZE_LARGE_TOOLBAR
    );
    gtk_widget_set_name(browse_button, "sidebar-browse-button"); // For CSS

    // 3. Create the list box and put it in a scrolled window
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    GtkWidget *list_box = gtk_list_box_new();

    // 4. Set up the callback data
    SidebarCallbackData *callback_data = g_new(SidebarCallbackData, 1);
    callback_data->parent_window = parent_window;
    callback_data->list_box = list_box;

    // 5. Connect the button signal
    // We pass our 'callback_data' struct as the user_data
    g_signal_connect(browse_button, "clicked", 
                     G_CALLBACK(on_browse_clicked), callback_data);

    // When the button is destroyed, free the data struct
    g_signal_connect(browse_button, "destroy",
                     G_CALLBACK(g_free), callback_data);

    // 6. Pack the sidebar widgets
    gtk_container_add(GTK_CONTAINER(scrolled_window), list_box);

    // Pack button at the top (doesn't expand)
    gtk_box_pack_start(GTK_BOX(sidebar_vbox), browse_button, FALSE, FALSE, 0);
    // Pack list box below (expands to fill)
    gtk_box_pack_start(GTK_BOX(sidebar_vbox), scrolled_window, TRUE, TRUE, 0);

    return sidebar_vbox;
}