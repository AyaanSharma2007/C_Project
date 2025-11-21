#ifndef DIFF_LOGIC_H
#define DIFF_LOGIC_H

#include "myers_diff.h"
#include <glib.h>

GArray* perform_diff(const char* file1_path, const char* file2_path);

#endif // DIFF_LOGIC_H
