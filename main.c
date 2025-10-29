#include <gtk/gtk.h>
static void
attach_context_menu(GtkWidget *widget, gpointer context)
{
    GtkGesture *right_click;

    // 1. Create a GtkGestureClick controller
    right_click = gtk_gesture_click_new();
    
    // 2. Set it to listen for the secondary button (right-click)
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(right_click), 
                                  GDK_BUTTON_SECONDARY);

    // 3. Connect its "pressed" signal to our modular function
    //    We pass the 'context' as the user_data.
    g_signal_connect(right_click, "pressed",
                     G_CALLBACK(on_widget_right_click),
                     context); // <-- Pass the context string

    // 4. Add the gesture controller to the widget
    gtk_widget_add_controller(widget, GTK_EVENT_CONTROLLER(right_click));
}
