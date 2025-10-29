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
    
    // 1. Create the File Chooser Dialog (same as before)
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Open File",
        data->parent_window,
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Open", GTK_RESPONSE_ACCEPT,
        NULL
    );

    // 2. Run the dialog (same as before)
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));

    // 3. Check response and update list
    if (response == GTK_RESPONSE_ACCEPT) {
        char *full_path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        char *basename = g_path_get_basename(full_path);

        GtkWidget *list_row_label = gtk_label_new(basename);
        gtk_widget_set_halign(list_row_label, GTK_ALIGN_START);

        // --- NEW: Make the label text wrap ---
        // This makes the file name responsive if the sidebar is shrunk
        gtk_label_set_line_wrap(GTK_LABEL(list_row_label), TRUE);
        
        gtk_list_box_insert(GTK_LIST_BOX(data->list_box), list_row_label, -1);
        gtk_widget_show(list_row_label);

        g_free(basename);
        g_free(full_path);
    }

    // 4. Destroy the dialog (same as before)
    gtk_widget_destroy(dialog);
}

// This is the public function defined in sidebar.h
GtkWidget *create_sidebar(GtkWindow *parent_window) {
    // 1. Create the main vertical box for the sidebar
    GtkWidget *sidebar_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    // 2. --- MODIFIED: Create the button with text AND icon ---
    GtkWidget *browse_button = gtk_button_new_with_label("Add Files");
    GtkWidget *icon = gtk_image_new_from_icon_name(
        "document-open-symbolic",
        GTK_ICON_SIZE_BUTTON // Use a standard button-sized icon
    );
    gtk_button_set_image(GTK_BUTTON(browse_button), icon);
    gtk_button_set_image_position(GTK_BUTTON(browse_button), GTK_POS_LEFT); // Icon on the left
    
    // --- NEW: Make button responsive ---
    // GTK_ALIGN_FILL tells the button to stretch horizontally
    gtk_widget_set_halign(browse_button, GTK_ALIGN_FILL);
    // Keep the old CSS name
    gtk_widget_set_name(browse_button, "sidebar-browse-button");

    // 3. Create the list box and scrolled window
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    GtkWidget *list_box = gtk_list_box_new();
    
    // --- NEW: Give the list a CSS name ---
    gtk_widget_set_name(list_box, "file-list-box");

    // 4. Set up the callback data (same as before)
    SidebarCallbackData *callback_data = g_new(SidebarCallbackData, 1);
    callback_data->parent_window = parent_window;
    callback_data->list_box = list_box;

    // 5. Connect signals (same as before)
    g_signal_connect(browse_button, "clicked", 
                     G_CALLBACK(on_browse_clicked), callback_data);
    g_signal_connect(browse_button, "destroy",
                     G_CALLBACK(g_free), callback_data);

    // 6. Pack the sidebar widgets (same as before)
    gtk_container_add(GTK_CONTAINER(scrolled_window), list_box);
    gtk_box_pack_start(GTK_BOX(sidebar_vbox), browse_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(sidebar_vbox), scrolled_window, TRUE, TRUE, 0);

    return sidebar_vbox;
}