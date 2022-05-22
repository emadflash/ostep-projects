#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int search(char* buf, size_t buf_len, char* word, size_t word_len) {
    for (int i = 0; i < buf_len; ++i) {
        if (buf[i] == word[0]) {
            int flag = 1;

            for (int j = 0; j < word_len && i < buf_len; ++j) {
                if (buf[i] != word[j]) {
                    flag = 0;
                    break;
                }

                ++i;
            }

            if (flag) return 1;
        }
    }

    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("wgrep: searchterm [file ...]\n");
        return 1;
    }

    char* searchterm = argv[1];
    size_t searchterm_len = strlen(argv[1]);

    if (argc == 2) {
        char buf[512];

        while (fgets(buf, 512, stdin)) {
            if (search(buf, strlen(buf), searchterm, searchterm_len)) {
                printf("%s", buf);
            }

            memset(buf, 0, 512);
        }

        return 0;
    }

    for (int i = 2; i < argc; ++i) {
        FILE* fp = fopen(argv[i], "r");
        
        if (!fp) {
            printf("wgrep: cannot open file\n");
            return 1;
        }

        char buf[512];

        while (fgets(buf, 512, fp)) {
            if (search(buf, strlen(buf), searchterm, searchterm_len)) {
                printf("%s", buf);
            }

            memset(buf, 0, 512);
        }

        fclose(fp);
    }

    return 0;
}
