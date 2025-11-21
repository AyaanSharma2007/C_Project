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
    gtk_text_buffer_create_tag(buffer1, "diff-delete", 
                              "background", "#FFE6E6",
                              "underline", PANGO_UNDERLINE_SINGLE,
                              "underline-rgba", &(GdkRGBA){1.0, 0.0, 0.0, 1.0},
                              NULL);
    gtk_text_buffer_create_tag(buffer2, "diff-insert", 
                              "background", "#E6FFE6",
                              "underline", PANGO_UNDERLINE_SINGLE,
                              "underline-rgba", &(GdkRGBA){0.0, 0.5, 0.0, 1.0},
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

    // Get character-level diff data and apply highlighting spans
    GArray* diffs = perform_diff(file1_path, file2_path);
    if (diffs) {
        gint left_offset = 0;
        gint right_offset = 0;

        for (guint i = 0; i < diffs->len; i++) {
            DiffOp *op = &g_array_index(diffs, DiffOp, i);
            if (!op->text) continue;

            gint char_len = (gint)g_utf8_strlen(op->text, -1);
            if (char_len == 0) {
                g_free(op->text);
                continue;
            }

            switch (op->type) {
                case DIFF_OP_EQUAL:
                    left_offset += char_len;
                    right_offset += char_len;
                    break;

                case DIFF_OP_DELETE: {
                    GtkTextIter start, end;
                    gtk_text_buffer_get_iter_at_offset(buffer1, &start, left_offset);
                    gtk_text_buffer_get_iter_at_offset(buffer1, &end, left_offset + char_len);
                    gtk_text_buffer_apply_tag_by_name(buffer1, "diff-delete", &start, &end);
                    left_offset += char_len;
                    break;
                }

                case DIFF_OP_INSERT: {
                    GtkTextIter start, end;
                    gtk_text_buffer_get_iter_at_offset(buffer2, &start, right_offset);
                    gtk_text_buffer_get_iter_at_offset(buffer2, &end, right_offset + char_len);
                    gtk_text_buffer_apply_tag_by_name(buffer2, "diff-insert", &start, &end);
                    right_offset += char_len;
                    break;
                }
            }

            g_free(op->text);
        }

        g_array_free(diffs, TRUE);
    }

    g_free(contents1);
    g_free(contents2);

    gtk_window_present(GTK_WINDOW(window));
}
