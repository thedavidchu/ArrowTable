#pragma once

#include <stdlib.h>
#include <stddef.h>

#define EMPTY ((size_t)-2)
#define INVALID ((size_t)-1)

#define GROWTH_THRESHOLD ((double)0.8)

typedef int KeyType;
typedef int ValueType;


typedef struct {
    size_t arrow;

    size_t hashcode;
    KeyType key;
    ValueType value;
} TableItem;

typedef struct {
    TableItem *table;
    size_t len;
    size_t cap;
} Table;


int tbl_init(Table *const me, size_t cap) {
    const size_t size = cap * sizeof(TableItem);
    size_t i = 0;

    if (me == NULL) {
        return -1;
    }
    if (cap != size / sizeof(TableItem)) {
        return -1;
    }

    me->table = (TableItem *)malloc(size);
    if (me->table == NULL) {
        return -1;
    }

    me->cap = cap;
    me->len = 0;

    for (i = 0; i < cap; ++i) {
        me->table[i].arrow = EMPTY;
    }

    return 0;
}

void tbl_del(Table *const me) {
    if (me == NULL) {
        return;
    }
    free(me->table);
    me->len = 0;    /* Maybe check that len is zero before free? */
    me->cap = 0;

    return;
}

size_t key_hash(KeyType key) {
    return (size_t)key;
}

int key_eq(KeyType a, KeyType b) {
    return a == b;
}