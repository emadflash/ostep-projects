// put: The format is p,key,value, where key is an integer, and value an arbitrary string (without commas in it).
//
// get: The format is g,key, where key is an integer.
//    If the key is present, the system should print out the key, followed by a comma, followed by the value, followed by a newline (\n).
//    If not present, print an error message on a line by itself, of the form K not found where K is the actual value of the key, i.e., some integer.
//
// delete: The format is d,key, which either deletes the relevant key-value pair (and prints nothing),
//    or fails to do so (and prints K not found where K is the actual value of the key, i.e., some integer).
//
// clear: The format is c. This command simply removes all key-value pairs from the database.
//
// all: The format is a. This command prints out all key-value pairs in the database, in any order,
//    with one key-value pair per line, each key and value separated by a comma.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>


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

bool match_string(char* cstr, char* string, size_t length) {
    if (strlen(cstr) != length) return false;
    for (size_t i = 0; i < length; ++i) {
        if (string[i] != *cstr) return false;
        ++cstr;
    }

    return true;
}


////////////////////////////////
// ~ hashmap_t

typedef struct bucket_t bucket_t;
struct bucket_t {
    int key;
    char *value;  // NULL means bucket empty
    bucket_t *next; // NULL next means no chain
};

typedef struct {
    bucket_t buckets[1024];
} hashmap_t;

hashmap_t *make_hashmap() {
    hashmap_t *hm = xmalloc(sizeof(hashmap_t));
    memset(hm->buckets, 0, sizeof(bucket_t) * 1024);
    return hm;
}
void free_hashmap(hashmap_t *hm) {
    bucket_t *curr, *temp;

    for (int i = 0; i < 1024; ++i) {
         curr = &hm->buckets[i];
         if (curr->value) {
             free(curr->value);
         }

         if (curr->next) {
             curr = curr->next;
             while (curr) {
                 temp = curr->next;
                 free(curr);
                 curr = temp;
             }
         }
    }

    free(hm);
}
bucket_t *get_hashmap(hashmap_t *hm, int key) {
    int pos = key % 1024;
    bucket_t *curr = &hm->buckets[pos];

    if (curr->key == key)
        return curr;

    while (curr->next) {
        curr = curr->next;
        if (curr->key == key)
            return curr;
    }

    return NULL;
}
void put_hashmap(hashmap_t *hm, int key, char *value) {
    int pos = key % 1024;
    bucket_t *curr = &hm->buckets[pos];

    if (!curr->value) {
        curr->key = key;
        curr->value = value;
    } else {
        while (curr->next) {
            curr = curr->next;
        }

        curr->next = xmalloc(sizeof(bucket_t));
        curr->next->key = key;
        curr->next->value = value;
        curr->next->next = NULL;
    }
}
bool del_hashmap(hashmap_t *hm, int key) {
    int pos = key % 1024;
    bucket_t *curr = &hm->buckets[pos];

    if (curr->key == key) {
        curr->value = NULL;
        return true;
    }

    bucket_t *prev = curr; // In single linked list keep track of previous 

    while (curr->next) {
        prev = curr;
        curr = curr->next;

        if (curr->key == key) {
            prev->next = curr->next;
            free(curr);
            curr = prev->next;
            return true;
        }
    }

    return false;
}


////////////////////////////////
// ~ Operations on key-value store

enum {
    GET = 0,
    PUT,
    DELETE,
    CLEAR,
    ALL,
};

int find_cmd(char *cmd, size_t n) {
    if (match_string("g", cmd, n)) return GET;
    if (match_string("p", cmd, n)) return PUT;
    if (match_string("d", cmd, n)) return DELETE;
    if (match_string("c", cmd, n)) return CLEAR;
    if (match_string("a", cmd, n)) return ALL;
    return -1;
}

hashmap_t *load_db(char *db_file, int *keys) {
    int key, count = 0;
    char *chunk;

    char *buf = xmalloc(1024);
    hashmap_t *hm = make_hashmap();
    FILE* f = xfopen(db_file, "r");

    while (fgets(buf, 1024, f)) {
        buf[strlen(buf) - 1] = '\0';  // NOTE(madflash) - set second last byte to '\0', which was previously '\n'

        chunk = strsep(&buf, ",");
        assert(chunk);

        key = atoi(chunk);
        keys[count++] = key;

        chunk = strsep(&buf, ",");
        assert(chunk);

        put_hashmap(hm, key, strdup(chunk));
    }

    free(buf);
    fclose(f);
    return hm;
}

void write_db(hashmap_t *hm, char *db_file) {
    FILE* f;
    bucket_t *curr;

    f = xfopen(db_file, "w");

    for (int i = 0; i < 1024; ++i) {
        curr = &hm->buckets[i];
        if (curr->value) {
            fprintf(f, "%d,%s\n", curr->key, curr->value);
        } else {
            // NOTE(madflash) - check if it contains chains
            if (curr->next) {
                do {
                    curr = curr->next;
                    fprintf(f, "%d,%s\n", curr->key, curr->value);
                } while (curr->next);
            }
        }
    }

    fclose(f);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        return 0;
    }

    assert(argc < BUFSIZ && "operations must be less than BUFSIZ");

    int cmd, key, keys[BUFSIZ];
    char *chunk, *value;
    hashmap_t* hm;

    hm = load_db("database.txt", keys);

    for (int i = 1; i < argc; ++i) {
        chunk = strsep(&argv[i], ",");
        cmd = find_cmd(chunk, strlen(chunk));

        if (cmd < 0) {
            printf("bad command");
            continue;
        }

        switch (cmd) {
            case GET: {
                chunk = strsep(&argv[i], ",");
                assert(chunk);
                key = atoi(chunk);

                bucket_t *r = get_hashmap(hm, key);
                if (!r)
                    printf("%d not found\n", key);
                else
                    printf("%d,%s\n", key, r->value);
            }
            break;

            case PUT: {
                chunk = strsep(&argv[i], ",");
                assert(chunk);
                key = atoi(chunk);
                value = strsep(&argv[i], ",");
                assert(value);

                put_hashmap(hm, key, value);
            }
            break;

            case DELETE: {
                chunk = strsep(&argv[i], ",");
                assert(chunk);
                key = atoi(chunk);
            }
            break;

            case CLEAR: {
            }
            break;

            case ALL: {
            }
            break;

            default: assert(false);
        }
    }

    
    write_db(hm, "database.txt");
    free_hashmap(hm);

    return 0;
}
