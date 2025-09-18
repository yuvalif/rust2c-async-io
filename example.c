#include <stdio.h>
#include <time.h>
#include "async_file_hasher.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file1> [file2] [file3] ...\n", argv[0]);
        return 1;
    }

    printf("\nProcessing %d files using C bindings:\n", argc - 1);

    struct timespec start, end;
    timespec_get(&start, TIME_UTC);

    for (int i = 1; i < argc; i++) {
        char* hash = calculate_md5_hash_sync_c(argv[i]);

        if (hash != NULL) {
            printf("%s: %s\n", argv[i], hash);
            free_string(hash);
        } else {
            fprintf(stderr, "Error processing file: %s\n", argv[i]);
        }
    }

    timespec_get(&end, TIME_UTC);

    double elapsed = (end.tv_sec - start.tv_sec) * 1000.0 +
                     (end.tv_nsec - start.tv_nsec) / 1000000.0;
    printf("C processing time: %.2f ms\n", elapsed);

    return 0;
}
