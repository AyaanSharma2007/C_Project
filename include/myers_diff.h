#ifndef MYERS_DIFF_H
#define MYERS_DIFF_H

#include <glib.h>

typedef enum {
    DIFF_OP_EQUAL,
    DIFF_OP_INSERT,
    DIFF_OP_DELETE
} DiffOpType;

typedef struct {
    DiffOpType type;
    char* text;
} DiffOp;

GArray* myers_diff(const char* text1, const char* text2);

#endif // MYERS_DIFF_H
