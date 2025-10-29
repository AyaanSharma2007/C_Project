#include "context_menu.h"
#include <stdio.h> // For printf

// --- Private Action Callback Functions ---
// These are the functions that run when a menu item is clicked.

static void on_option1_activate(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    // user_data will be the GtkWidget we passed when adding the actions
    GtkWidget *widget = GTK_WIDGET(user_data);
    printf("Context Menu: Option 1 selected on widget %s\n", 
           gtk_widget_get_name(widget));
}

static void on_option2_activate(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    printf("Context Menu: Option 2 selected!\n");
}

// An array of GActionEntry structs that defines our actions
static const GActionEntry context_menu_actions[] = {
    {"option1", on_option1_activate, NULL, NULL, NULL},
    {"option2", on_option2_activate, NULL, NULL, NULL}
};


// --- Public Function (declared in the .h file) ---

void on_widget_right_click(GtkGestureClick *gesture,
                           int n_press,
                           double x,
                           double y,
                           gpointer user_data) {
    
    // Get the widget that the gesture is attached to
    GtkWidget *widget = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(gesture));

    // Actions must be added to a widget in the hierarchy.
    // The top-level window is the best place for this.
    GtkWidget *toplevel = gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW);

    // Add our actions to the window's action map.
    // This only adds them if they don't already exist,
    // so it's safe to call multiple times.
    // We pass 'widget' as the user_data for the actions.
    g_action_map_add_action_entries(G_ACTION_MAP(toplevel),
                                    context_menu_actions,
                                    G_N_ELEMENTS(context_menu_actions),
                                    widget);

    // --- Create the Menu Model (GMenu) ---
    GMenu *menu_model = g_menu_new();
    
    // Create menu items and link them to our actions.
    // "win.option1" means "call the 'option1' action in the 'win' scope"
    // (the GtkApplicationWindow provides the "win" scope).
    g_menu_append(menu_model, "Option 1", "win.option1");
    g_menu_append(menu_model, "Option 2", "win.option2");
    g_menu_append_section(menu_model, NULL, G_MENU_MODEL(g_menu_new())); // Separator
    g_menu_append(menu_model, "A Third Option", "win.action-does-not-exist"); // Will be greyed out
    
    // --- Create the Popover Menu (GtkPopoverMenu) ---
    // Create the popover widget from the model
    GtkWidget *popover = gtk_popover_menu_new_from_model(G_MENU_MODEL(menu_model));
    
    // Set the popover to be a child of the widget
    // gtk_widget_set_parent(popover, widget);
    // gtk_popover_set_parent(GTK_POPOVER(popover), widget);
    // This is the CORRECT line
gtk_widget_set_parent(popover, widget);
    // --- Position and Show the Popover ---
    // Create a GdkRectangle for the click location
    GdkRectangle pointing_rect = { (int)x, (int)y, 1, 1 };
    
    // Tell the popover to point to that small rectangle
    gtk_popover_set_pointing_to(GTK_POPOVER(popover), &pointing_rect);
    
    // Show the popover
    gtk_popover_popup(GTK_POPOVER(popover));

    // We don't need to hold on to the popover or model,
    // they will be destroyed automatically.
    // g_object_unref(popover);
    g_object_unref(menu_model);
}
