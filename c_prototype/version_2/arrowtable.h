#pragma once

#include <stdbool.h>
#include <stddef.h>

/* Define the size to be 100 for now */
#define SIZE (100)

struct Node {
    // We have the valid condition, because 
    bool valid;
    size_t hashcode;
    char *key;
    int value;
    size_t offset;
};

struct Table {
    size_t size;
    size_t length;
    struct Node table[SIZE];
};

void create(struct Table *me);  // Done
void destroy(struct Table *me); // Done
int insert(struct Table *me, char *key, int value); // Done?
int search(struct Table *me, char *key, int *value); // Done?
int delete(struct Table *me, char *key);

void print(struct Table *me);
void debug(struct Table *me);
