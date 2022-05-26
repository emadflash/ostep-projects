#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

void expand_and_dump(uint32_t count, char ch) {
    for (uint32_t i = 0; i < count; ++i) {
        printf("%c", ch);
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("wunzip: file1 [file2 ...]\n");
        return 1;
    }

    FILE*    fp;
    uint32_t count, peek_count;
    char     curr, peek_next;

    int i_argc = 1;

    fp = fopen(argv[i_argc], "r");
    if (!fp) {
        return 1;
    }
    i_argc++;

    if (feof(fp)) goto next_input_file;

    if (fread(&count, 4, 1, fp) != 1) assert(false && "expected a count");
    if (fread(&curr, 1, 1, fp) != 1) assert(false && "expected a character");

    while (!feof(fp)) {
        if (fread(&count, 4, 1, fp) != 1) break;
        if (fread(&curr, 1, 1, fp) != 1) assert(false && "expected a character");

        expand_and_dump(count, curr);
    }

next_input_file:
    fclose(fp);

    if (i_argc >= argc) {
        expand_and_dump(count, curr);
    } else {
        fp = fopen(argv[i_argc], "r");
        if (!fp) {
            return 1;
        }
        i_argc++;

        if (fread(&peek_count, 4, 1, fp) != 1) assert(false && "expected a count");;
        if (fread(&peek_next, 1, 1, fp) != 1) assert(false && "expected a character");

        if (peek_next == curr) {
            count += peek_count;

            if (feof(fp)) goto next_input_file;
            else expand_and_dump(count, curr);

            if (fread(&count, 4, 1, fp) != 1) assert(false && "expected a count");
            if (fread(&curr, 1, 1, fp) != 1) assert(false && "expected a character");

            while (!feof(fp)) {
                if (fread(&count, 4, 1, fp) != 1) break;
                if (fread(&curr, 1, 1, fp) != 1) assert(false && "expected a character");

                expand_and_dump(count, curr);
            }

            goto next_input_file;
        } else {
            expand_and_dump(peek_count, peek_next);

            if (fread(&count, 4, 1, fp) != 1) assert(false && "expected a count");
            if (fread(&curr, 1, 1, fp) != 1) assert(false && "expected a character");

            while (!feof(fp)) {
                if (fread(&count, 4, 1, fp) != 1) break;
                if (fread(&curr, 1, 1, fp) != 1) assert(false && "expected a character");

                expand_and_dump(count, curr);
            }

            goto next_input_file;
        }
    }
    
    return 0;
}
