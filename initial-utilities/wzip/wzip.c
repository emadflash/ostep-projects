#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

char* file_as_string(char* fname) {
    FILE* fp = fopen(fname, "r");

    if (!fp) {
        return NULL;
    }

    long buf_length;

    fseek(fp, 0, SEEK_END);
    buf_length = ftell(fp);
    rewind(fp);

    char* buf = calloc(1, buf_length + 1);
    fread(buf, 1, buf_length, fp);

    fclose(fp);
    return buf;
}

// RLE is quite simple: when you encounter n characters of the same type in a row,
// the compression tool (wzip) will turn that into the number n and a single instance of the character.
//
// Thus, if we had a file with the following contents:
//
// aaaaaaaaaabbbb

// Hmmm ?
// This function is not used, because i would have to first concatenate all the input files into input_buffer
// and perform RLE, but it would consume memory i guess
// So, i ended up reading input files incrementally, rather than reading all files at once

void rl_encode(char* input_buffer) {
    uint32_t count = 1;

    char* input = input_buffer;
    char* curr = input;

    if (*input == '\0') {
        return;
    }

try_counting:
    input++;

    while (*input && *input == *curr) {
        count++;
        input++;
    }

    fwrite(&count, 1, sizeof(uint32_t), stdout);
    fwrite(curr, 1, 1, stdout);

    if (*input) {
        curr = input;
        count = 1;
        goto try_counting;
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("wzip: file1 [file2 ...]\n");
        return 1;
    }

    // Read first file and set curr to begining of first file
    char* buf = file_as_string(argv[1]);
    char* input = buf;

    uint32_t count = 0;
    char curr = *input;
    int input_idx = 2;

rl_encode:
        while (*input && *input == curr) {
            input++;
            count++;
        }

        if (*input) {
            fwrite(&count, 1, sizeof(uint32_t), stdout);
            fwrite(&curr, 1, 1, stdout);

            curr = *input;
            count = 0;
            goto rl_encode;
        } else {
            free(buf);

            if (input_idx >= argc) {
                fwrite(&count, 1, sizeof(uint32_t), stdout);
                fwrite(&curr, 1, 1, stdout);
                return 0;
            }

            buf = file_as_string(argv[input_idx]);
            input = buf;

            if (*input != curr) {
                fwrite(&count, 1, sizeof(uint32_t), stdout);
                fwrite(&curr, 1, 1, stdout);

                curr = *input;
                count = 0;
            }

            input_idx++;
            goto rl_encode;
        }

    return 0;
}
