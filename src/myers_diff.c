#include "myers_diff.h"
#include <string.h>
#include <stdlib.h>

GArray* myers_diff(const char* text1, const char* text2) {
    GArray* diffs = g_array_new(FALSE, FALSE, sizeof(DiffOp));
    int n = strlen(text1);
    int m = strlen(text2);
    int max = n + m;
    int* v = g_new(int, 2 * max + 1);
    int i, k;

    for (i = 0; i <= 2 * max; i++) {
        v[i] = -1;
    }
    v[max + 1] = 0;

    for (int d = 0; d <= max; d++) {
        for (k = -d; k <= d; k += 2) {
            int x;
            if (k == -d || (k != d && v[max + k - 1] < v[max + k + 1])) {
                x = v[max + k + 1];
            } else {
                x = v[max + k - 1] + 1;
            }
            int y = x - k;

            while (x < n && y < m && text1[x] == text2[y]) {
                x++;
                y++;
            }

            v[max + k] = x;

            if (x >= n && y >= m) {
                // Backtrack
                int current_x = n;
                int current_y = m;
                int current_k = k;

                for (int prev_d = d - 1; prev_d >= 0; prev_d--) {
                    int prev_k;
                    if (current_k == -prev_d || (current_k != prev_d && v[max + current_k - 1] < v[max + current_k + 1])) {
                        prev_k = current_k + 1;
                    } else {
                        prev_k = current_k - 1;
                    }

                    int prev_x = v[max + prev_k];
                    int prev_y = prev_x - prev_k;

                    while (current_x > prev_x && current_y > prev_y && text1[current_x - 1] == text2[current_y - 1]) {
                        DiffOp op = {DIFF_OP_EQUAL, g_strndup(&text1[current_x - 1], 1)};
                        g_array_prepend_val(diffs, op);
                        current_x--;
                        current_y--;
                    }

                    if (prev_d >= 0) {
                        if (prev_x < current_x) {
                            DiffOp op = {DIFF_OP_DELETE, g_strndup(&text1[current_x - 1], 1)};
                            g_array_prepend_val(diffs, op);
                            current_x--;
                        } else if (prev_y < current_y) {
                            DiffOp op = {DIFF_OP_INSERT, g_strndup(&text2[current_y - 1], 1)};
                            g_array_prepend_val(diffs, op);
                            current_y--;
                        }
                    }
                    current_k = prev_k;
                }

                while (current_x > 0 && current_y > 0 && text1[current_x - 1] == text2[current_y - 1]) {
                    DiffOp op = {DIFF_OP_EQUAL, g_strndup(&text1[current_x - 1], 1)};
                    g_array_prepend_val(diffs, op);
                    current_x--;
                    current_y--;
                }
                
                g_free(v);
                return diffs;
            }
        }
    }

    g_free(v);
    return diffs;
}
