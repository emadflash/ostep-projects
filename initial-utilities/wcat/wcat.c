#include <stdio.h>
#include <stdlib.h>

int print_file(char* fname) {
    FILE* fp = fopen(fname, "r");

    if (!fp) {
        return -1;
    }

    long buf_length;

    fseek(fp, 0, SEEK_END);
    buf_length = ftell(fp);
    rewind(fp);

    char* buf = calloc(1, buf_length + 1);
    fread(buf, 1, buf_length, fp);

    printf("%s", buf);

    free(buf);
    fclose(fp);

    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        return 0;
    }

    for (int i = 1; i < argc; ++i) {
        if (print_file(argv[i]) < 0) {
            printf("wcat: cannot open file\n");
            return 1;
        }
    }

    return 0;
}
