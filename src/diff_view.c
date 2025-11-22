#include "diff_view.h"
#include "diff_logic.h"
#include <gtk/gtk.h>
#include <string.h>

void create_diff_window(GtkWindow* parent, const char* file1_path, const char* file2_path) {
    GtkWidget *window, *grid, *scrolled_window1, *scrolled_window2, *view1, *view2, *gutter;
    GtkWidget *label1, *label2;
    GtkTextBuffer *buffer1, *buffer2;

    window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(window), "Compare Files");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);
    gtk_window_set_transient_for(GTK_WINDOW(window), parent);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);

    grid = gtk_grid_new();
    gtk_window_set_child(GTK_WINDOW(window), grid);

    // Add labels for file names
    gchar *basename1 = g_path_get_basename(file1_path);
    gchar *basename2 = g_path_get_basename(file2_path);
    
    label1 = gtk_label_new(basename1);
    gtk_widget_set_halign(label1, GTK_ALIGN_START);
    gtk_widget_set_margin_start(label1, 10);
    gtk_widget_set_margin_top(label1, 5);
    gtk_widget_set_margin_bottom(label1, 5);
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 1, 1);
    
    label2 = gtk_label_new(basename2);
    gtk_widget_set_halign(label2, GTK_ALIGN_START);
    gtk_widget_set_margin_start(label2, 10);
    gtk_widget_set_margin_top(label2, 5);
    gtk_widget_set_margin_bottom(label2, 5);
    gtk_grid_attach(GTK_GRID(grid), label2, 2, 0, 1, 1);
    
    g_free(basename1);
    g_free(basename2);

    // Create two text views
    view1 = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view1), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view1), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(view1), TRUE);
    buffer1 = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view1));
    scrolled_window1 = gtk_scrolled_window_new();
    gtk_widget_set_hexpand(scrolled_window1, TRUE);
    gtk_widget_set_vexpand(scrolled_window1, TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window1), view1);
    gtk_grid_attach(GTK_GRID(grid), scrolled_window1, 0, 1, 1, 1);

    gutter = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_size_request(gutter, 2, -1);
    gtk_grid_attach(GTK_GRID(grid), gutter, 1, 1, 1, 1);

    view2 = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view2), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view2), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(view2), TRUE);
    buffer2 = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view2));
    scrolled_window2 = gtk_scrolled_window_new();
    gtk_widget_set_hexpand(scrolled_window2, TRUE);
    gtk_widget_set_vexpand(scrolled_window2, TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window2), view2);
    gtk_grid_attach(GTK_GRID(grid), scrolled_window2, 2, 1, 1, 1);

    // Apply CSS for better styling
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider,
                                      "textview { font-family: monospace; font-size: 11pt; padding: 8px; }");
    gtk_style_context_add_provider_for_display(gdk_display_get_default(),
                                               GTK_STYLE_PROVIDER(provider),
                                               GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

    // Create tags for highlighting differences with underlines
    gtk_text_buffer_create_tag(buffer2, "diff-insert",
                              "underline", PANGO_UNDERLINE_SINGLE,
                              "foreground", "#b30000",
                              NULL);

    // Read file contents
    gchar *contents1 = NULL, *contents2 = NULL;
    gsize length1, length2;
    
    if (!g_file_get_contents(file1_path, &contents1, &length1, NULL)) {
        g_printerr("Failed to read file: %s\n", file1_path);
        contents1 = g_strdup("[Error reading file]");
    }
    
    if (!g_file_get_contents(file2_path, &contents2, &length2, NULL)) {
        g_printerr("Failed to read file: %s\n", file2_path);
        contents2 = g_strdup("[Error reading file]");
    }

    // Populate buffers with full file contents first
    gtk_text_buffer_set_text(buffer1, contents1 ? contents1 : "", -1);
    gtk_text_buffer_set_text(buffer2, contents2 ? contents2 : "", -1);

    // Highlight words in the latest file that differ at the same word positions
    const gchar *p1 = contents1 ? contents1 : "";
    const gchar *p2 = contents2 ? contents2 : "";

    // Tokenize previous file into words array (words are non-whitespace runs)
    GPtrArray *words1 = g_ptr_array_new_with_free_func(g_free);
    const gchar *q = p1;
    while (*q) {
        gunichar c = g_utf8_get_char(q);
        if (g_unichar_isspace(c)) { q = g_utf8_next_char(q); continue; }
        const gchar *start = q;
        while (*q) {
            gunichar n = g_utf8_get_char(q);
            if (g_unichar_isspace(n)) break;
            q = g_utf8_next_char(q);
        }
        gchar *w = g_strndup(start, q - start);
        g_ptr_array_add(words1, w);
    }

    // Walk through latest file, tracking character offsets so tag ranges line up
    gint latest_offset = 0; // character offset into buffer2
    gint word_index = 0;
    const gchar *r = p2;
    while (*r) {
        gunichar c = g_utf8_get_char(r);
        if (g_unichar_isspace(c)) {
            r = g_utf8_next_char(r);
            latest_offset++;
            continue;
        }

        // extract next word in latest file
        const gchar *start = r;
        while (*r) {
            gunichar n = g_utf8_get_char(r);
            if (g_unichar_isspace(n)) break;
            r = g_utf8_next_char(r);
        }
        gchar *w2 = g_strndup(start, r - start);
        gint wchars = g_utf8_strlen(w2, -1);

        gboolean highlight = TRUE;
        if (word_index < (gint)words1->len) {
            gchar *w1 = g_ptr_array_index(words1, word_index);
            if (g_strcmp0(w1, w2) == 0) highlight = FALSE;
        }

        if (highlight && wchars > 0) {
            GtkTextIter s, e;
            gtk_text_buffer_get_iter_at_offset(buffer2, &s, latest_offset);
            gtk_text_buffer_get_iter_at_offset(buffer2, &e, latest_offset + wchars);
            gtk_text_buffer_apply_tag_by_name(buffer2, "diff-insert", &s, &e);
        }

        latest_offset += wchars;
        word_index++;
        g_free(w2);
    }

    g_ptr_array_free(words1, TRUE);

    g_free(contents1);
    g_free(contents2);

    gtk_window_present(GTK_WINDOW(window));
}
