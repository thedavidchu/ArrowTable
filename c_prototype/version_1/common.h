#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#define EMPTY ((size_t)-1)
#define NOWHERE ((size_t)-2)

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

    if (cap == 0) {
        me->len = 0;
        me->cap = 0;
        me->table = NULL;
        return 0;
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
    me->table = NULL;
    me->len = 0;    /* Maybe check that len is zero before free? */
    me->cap = 0;

    return;
}

void tbl_debug_verbose(const Table *const me) {
    size_t i = 0;

    assert(me && "me is null");
    printf("(len: %zu, cap: %zu) {\n", me->len, me->cap);
    assert(me->cap == 0 && me->table == NULL || me->cap != 0 && me->table != NULL);

    for (i = 0; i < me->cap; ++i) {
        const TableItem *item = &me->table[i];
        if (item->arrow == EMPTY) {
            printf("\t%zu: [arrow: EMPTY, hashcode: (%zu), key: (%d), value: (%d)],\n",
                i, item->hashcode, item->key, item->value);
        } else {
            printf("\t%zu: [arrow: ", i);
            if (item->arrow == NOWHERE) {
                printf("NOWHERE");
            } else {
                printf("%zu", item->arrow);
            }
            printf(", hashcode: %zu, key: %d, value: %d],\n",
                item->hashcode, item->key, item->value);
        }
    }

    if (me->cap == 0) {
        printf("\tNULL\n");
    }

    printf("}\n");
}

size_t key_hash(KeyType key) {
    return (size_t)key;
}

int key_eq(KeyType a, KeyType b) {
    return a == b;
}