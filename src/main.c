#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "arrow.h"

/// @brief  Wrapper around 'perror' and 'strerror' functions.
void
print_error(int const errno_)
{
    perror(strerror(errno_));
}

int
main(void)
{
    int err = 0;
    struct ArrowTable a = {0};
    if ((err = ArrowTable_init(&a))) {
        print_error(err);
        return EXIT_FAILURE;
    }

    ArrowTable_print(&a, stdout, true);
    printf("GET %d = %d\n", 0, ArrowTable_get(&a, 0)); 
    printf("GET %d = %d\n", 1, ArrowTable_get(&a, 1)); 
    printf("GET %d = %d\n", 2, ArrowTable_get(&a, 2)); 
    printf("GET %d = %d\n", 3, ArrowTable_get(&a, 3)); 

    printf("PUT %d %d = %d\n", 0, 0, ArrowTable_put(&a, 0, 0));
    ArrowTable_print(&a, stdout, true);
    printf("---\n");
    printf("PUT %d %d = %d\n", 1, 0, ArrowTable_put(&a, 1, 0));
    ArrowTable_print(&a, stdout, true);
    printf("---\n");
    printf("PUT %d %d = %d\n", 2, 0, ArrowTable_put(&a, 2, 0));
    ArrowTable_print(&a, stdout, true);
    printf("---\n");
    printf("PUT %d %d = %d\n", 3, 0, ArrowTable_put(&a, 3, 0));
    ArrowTable_print(&a, stdout, true);
    printf("---\n");
    printf("PUT %d %d = %d\n", 8, 0, ArrowTable_put(&a, 8, 0));
    ArrowTable_print(&a, stdout, true);
    printf("---\n");
    
    if ((err = ArrowTable_destroy(&a))) {
        print_error(err);
        return EXIT_FAILURE;
    }
    print_error(err);
    return 0;
}

