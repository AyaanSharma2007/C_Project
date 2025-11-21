#include "diff_logic.h"
#include <glib.h>

GArray* perform_diff(const char* file1_path, const char* file2_path) {
    gchar *contents1, *contents2;
    gsize length1, length2;

    // Read file contents
    if (!g_file_get_contents(file1_path, &contents1, &length1, NULL) ||
        !g_file_get_contents(file2_path, &contents2, &length2, NULL)) {
        return NULL;
    }

    GArray* diffs = myers_diff(contents1, contents2);

    g_free(contents1);
    g_free(contents2);

    return diffs;
}
