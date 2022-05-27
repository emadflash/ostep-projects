#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

////////////////////////////////
// ~ wrappers

FILE* xfopen(char* fname, char* mode) {
    FILE* f = fopen(fname, mode);
    if (!f) {
        fprintf(stderr, "reverse: cannot open file '%s'\n", fname);
        exit(1);
    }
    return f;
}
void* xmalloc(size_t size) {
    void* m = malloc(size);
    if (!m) {
        fprintf(stderr, "malloc failed");
        exit(1);
    }
    return m;
}
void* xrealloc(void* ptr, size_t size) {
    void* m = realloc(ptr, size);
    if (!m) {
        fprintf(stderr, "realloc failed");
        exit(1);
    }
    return m;
}

////////////////////////////////
// ~ lines_t

typedef struct {
    char** lines;
    unsigned int len, cap;
} lines_t;

lines_t* make_lines() {
    lines_t* lines = xmalloc(sizeof(lines_t));

    lines->lines = xmalloc(32 * sizeof(char*));
    lines->cap = 32;
    lines->len = 0;

    return lines;
}
void free_lines(lines_t* lines) {
    for (int i = 0; i < lines->len; ++i)
        free(lines->lines[i]);

    free(lines->lines);
    free(lines);
}
void push_line(lines_t* lines, char* cstr) {
    if (lines->len >= lines->cap) {
        lines->cap *= 2;
        lines->lines = xrealloc(lines->lines, lines->cap * sizeof(char*));
    }

    lines->lines[lines->len] = cstr;
    lines->len++;
}
lines_t* read_lines(FILE* in) {
    char buf[512];
    lines_t* lines = make_lines();

    while (fgets(buf, 512, in)) {
        push_line(lines, strdup(buf));
        memset(buf, 0, 512);
    }

    return lines;
}
bool match_lines(lines_t* a, lines_t* b) {
    if (a->len != b->len) return false;
    for (int i = 0; i < a->len; ++i) {
        if (strcmp(a->lines[i], b->lines[i]) != 0)
            return true;
    }

    return true;
}

////////////////////////////////
// ~ entry point

int main(int argc, char** argv) {
    int retVal = 0;
    FILE* in, *out;

    if (argc > 3) {
        fprintf(stderr, "usage: reverse <input> <output>\n");
        exit(1);
    }

    if (argc < 2) {
        // NOTE(madflash) - Ctrl-d is used to flush input buffer, and then program will print lines in reversed order in stdout
        in = stdin;
        out = stdout;
    } else {
        in = xfopen(argv[1], "r");
        if (argc > 2) {
            if (strcmp(argv[1], argv[2]) == 0) {
                fprintf(stderr, "reverse: input and output file must differ\n");
                exit(1);
            }

            out = xfopen(argv[2], "w");
        } else {
            out = stdout;
        }
    }

    ////////////////////////////////
    // ~ lines of input and output file
    
    lines_t* lines, *out_lines; 

    lines = read_lines(in);
    out_lines = read_lines(out);

    if (lines->len == 0 || match_lines(lines, out_lines)) {
        retVal = 1;
        fprintf(stderr, "reverse: input and output file must differ\n");
        free_lines(out_lines);
        goto end;
    }

    free_lines(out_lines);

    for (int i = (int) lines->len - 1; i >= 0; --i) {
        fwrite(lines->lines[i], 1, strlen(lines->lines[i]), out);
    }

end:
    fclose(in);
    fclose(out);
    free_lines(lines);

    return retVal;
}
