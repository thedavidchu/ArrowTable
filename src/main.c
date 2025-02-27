#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arrow.h"
#include "logger.h"

/// @brief  Wrapper around 'perror' and 'strerror' functions.
static void
print_error(int const errno_)
{
    perror(strerror(errno_));
}

static int
run_simple_trace()
{
    int err = 0;
    struct ArrowTable a = {0};
    if ((err = ArrowTable_init(&a))) {
        print_error(err);
        return err;
    }

    ArrowTable_print(&a, stdout, true);
    printf("GET %d = %d\n", 0, ArrowTable_get(&a, 0)); 
    printf("GET %d = %d\n", 1, ArrowTable_get(&a, 1)); 
    printf("GET %d = %d\n", 2, ArrowTable_get(&a, 2)); 
    printf("GET %d = %d\n", 3, ArrowTable_get(&a, 3)); 

    printf("PUT %d %d = %d\n", 0, 0, ArrowTable_put(&a, 0, 0));
    printf("PUT %d %d = %d\n", 1, 0, ArrowTable_put(&a, 1, 0));
    printf("PUT %d %d = %d\n", 2, 0, ArrowTable_put(&a, 2, 0));
    printf("PUT %d %d = %d\n", 3, 0, ArrowTable_put(&a, 3, 0));
    printf("PUT %d %d = %d\n", 8, 0, ArrowTable_put(&a, 8, 0));
    printf("PUT %d %d = %d\n", 16, 0, ArrowTable_put(&a, 16, 0));
    ArrowTable_print(&a, stdout, true);
    printf("---\n");

    printf("PUT %d %d = %d\n", 0, 1, ArrowTable_put(&a, 0, 1));
    printf("PUT %d %d = %d\n", 1, 1, ArrowTable_put(&a, 1, 1));
    printf("PUT %d %d = %d\n", 2, 1, ArrowTable_put(&a, 2, 1));
    printf("PUT %d %d = %d\n", 3, 1, ArrowTable_put(&a, 3, 1));
    printf("PUT %d %d = %d\n", 8, 1, ArrowTable_put(&a, 8, 1));
    printf("PUT %d %d = %d\n", 16, 1, ArrowTable_put(&a, 16, 1));
    ArrowTable_print(&a, stdout, true);
    printf("---\n");
    
    if ((err = ArrowTable_destroy(&a))) {
        print_error(err);
        return err;
    }
    print_error(err);
    return 0;
}

static int
run_trace(char const *const trace_path)
{
    char op_str[4] = {0};
    int key = 0, value = 0;
    int err = 0;
    struct ArrowTable a = {0};

    assert(trace_path != NULL);

    if ((err = ArrowTable_init(&a))) {
        print_error(err);
        return err;
    }

    FILE *fp = fopen(trace_path, "r");
    if (fp == NULL) {
        printf("'%s' path DNE\n", trace_path);
        print_error(errno);
        // TODO Reset errno?
        return errno;
    }

    while (1) {
        if (fscanf(fp, "%3s %d %d", op_str, &key, &value) != 3)
            break;
        LOGGER_TRACE("%s, %d, %d", op_str, key, value);
        if (strcmp(op_str, "GET") == 0) {
            assert(ArrowTable_get(&a, key) == value);
        } else if (strcmp(op_str, "PUT") == 0) {
            assert(ArrowTable_put(&a, key, value) == 0);
        } else {
            assert(0 && "IMPOSSIBLE!");
        }
    }

    if ((err = ArrowTable_destroy(&a))) {
        print_error(err);
        return err;
    }
    print_error(err);
    return 0;
}

int
main(int argc, char *argv[])
{
    if (false)
        assert(run_simple_trace() == 0);
    if (true)
        for (size_t i = 1; i < argc; ++i) {
            assert(run_trace(argv[i]) == 0);
        }
    return 0;
}
