#include "sidebar.h"
#include <glib/gstdio.h> // For g_path_get_basename

// NEW: This struct now holds all widgets our sidebar needs
typedef struct {
    GtkWindow *parent_window;
    GtkWidget *list_box;
    GtkWidget *delete_button; // So we can enable/disable it
} SidebarData;


// --- "Add Files" callback ---
// This now uses the SidebarData struct
static void on_browse_clicked(GtkButton *button, gpointer user_data) {
    SidebarData *data = (SidebarData *)user_data;
    
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Open File",
        data->parent_window,
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Open", GTK_RESPONSE_ACCEPT,
        NULL
    );

    gint response = gtk_dialog_run(GTK_DIALOG(dialog));

    if (response == GTK_RESPONSE_ACCEPT) {
        char *full_path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        char *basename = g_path_get_basename(full_path);

        GtkWidget *list_row_label = gtk_label_new(basename);
        gtk_widget_set_halign(list_row_label, GTK_ALIGN_START);
        gtk_label_set_line_wrap(GTK_LABEL(list_row_label), TRUE);
        
        gtk_list_box_insert(GTK_LIST_BOX(data->list_box), list_row_label, -1);
        gtk_widget_show(list_row_label);

        g_free(basename);
        g_free(full_path);
    }
    gtk_widget_destroy(dialog);
}

// --- NEW: "Delete File" callback ---
static void on_delete_clicked(GtkButton *button, gpointer user_data) {
    SidebarData *data = (SidebarData *)user_data;

    // 1. Get the currently selected row
    GtkListBoxRow *selected_row = gtk_list_box_get_selected_row(
        GTK_LIST_BOX(data->list_box)
    );

    // This shouldn't happen if the button is disabled, but good to check
    if (selected_row == NULL) {
        return;
    }

    // 2. Create a confirmation dialog
    GtkWidget *dialog = gtk_message_dialog_new(
        data->parent_window,
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_QUESTION,
        GTK_BUTTONS_YES_NO,
        "Do you want to delete this file?"
    );
    gtk_window_set_title(GTK_WINDOW(dialog), "Confirm Delete");

    // 3. Run the dialog and check the response
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));

    if (response == GTK_RESPONSE_YES) {
        // Remove the row from the list box
        gtk_container_remove(GTK_CONTAINER(data->list_box), GTK_WIDGET(selected_row));
    }

    // 4. Destroy the confirmation dialog
    gtk_widget_destroy(dialog);
}

// --- NEW: "List Selection" callback ---
// This enables/disables the delete button
static void on_row_selected(GtkListBox *list_box, GtkListBoxRow *row, gpointer user_data) {
    SidebarData *data = (SidebarData *)user_data;
    
    // If 'row' is not NULL, a row is selected, so we enable the button
    gtk_widget_set_sensitive(data->delete_button, (row != NULL));
}

// --- Main create_sidebar function (Modified) ---
GtkWidget *create_sidebar(GtkWindow *parent_window) {
    GtkWidget *sidebar_vbox, *button_hbox, *browse_button, *delete_button, *scrolled_window, *list_box, *icon;

    // 1. Create main vertical box
    sidebar_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5); // Added 5px spacing

    // 2. NEW: Create horizontal box for buttons
    button_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5); // 5px spacing

    // 3. Create "Add Files" button (as before)
    browse_button = gtk_button_new_with_label("Add Files");
    icon = gtk_image_new_from_icon_name("document-open-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(browse_button), icon);
    gtk_button_set_image_position(GTK_BUTTON(browse_button), GTK_POS_LEFT);
    // NEW: Add a CSS class
    gtk_style_context_add_class(gtk_widget_get_style_context(browse_button), "sidebar-button");

    // 4. NEW: Create "Delete" button
    delete_button = gtk_button_new_with_label("Delete");
    icon = gtk_image_new_from_icon_name("edit-delete-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(delete_button), icon);
    // NEW: Add the same CSS class
    gtk_style_context_add_class(gtk_widget_get_style_context(delete_button), "sidebar-button");
    // NEW: Disable it by default
    gtk_widget_set_sensitive(delete_button, FALSE);

    // 5. Pack buttons into button_hbox
    // Add button fills available space
    gtk_box_pack_start(GTK_BOX(button_hbox), browse_button, TRUE, TRUE, 0); 
    // Delete button only takes its own space
    gtk_box_pack_start(GTK_BOX(button_hbox), delete_button, FALSE, FALSE, 0); 

    // 6. Create list box
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    list_box = gtk_list_box_new();
    gtk_widget_set_name(list_box, "file-list-box"); // For CSS
    gtk_container_add(GTK_CONTAINER(scrolled_window), list_box);

    // 7. NEW: Create and fill the SidebarData struct
    SidebarData *callback_data = g_new(SidebarData, 1);
    callback_data->parent_window = parent_window;
    callback_data->list_box = list_box;
    callback_data->delete_button = delete_button;

    // 8. Connect all signals
    g_signal_connect(browse_button, "clicked", G_CALLBACK(on_browse_clicked), callback_data);
    g_signal_connect(delete_button, "clicked", G_CALLBACK(on_delete_clicked), callback_data);
    g_signal_connect(list_box, "row-selected", G_CALLBACK(on_row_selected), callback_data);
    // Free the data struct when the sidebar is destroyed
    g_signal_connect(sidebar_vbox, "destroy", G_CALLBACK(g_free), callback_data);

    // 9. Pack main sidebar
    // Pack button box at the top (doesn't expand vertically)
    gtk_box_pack_start(GTK_BOX(sidebar_vbox), button_hbox, FALSE, FALSE, 0);
    // Pack list box below (expands to fill)
    gtk_box_pack_start(GTK_BOX(sidebar_vbox), scrolled_window, TRUE, TRUE, 0);

    return sidebar_vbox;
}
