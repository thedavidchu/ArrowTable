/** Arrow Table Implementation
 * 
 * Coding Style
 * ------------
 * 
 * You may notice that I am trying to conform closely to the C programming
 * language. This is because would prefer to write this in pure C, but I did not
 * want to implement my own "vector". */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "common.h"



#define NUM2IDX(tbl, num) ((num) % (tbl.cap))


static void _get_bounds_nocheck(Table *me, size_t home, size_t bounds[2]) {


}

/** Get the number of items belonging to 'home'.
 * 
 * If start is empty or invalid (i.e. points to nothing), then we have zero
 * items belonging to the home.
 * 
 * Next, we want to check if the end is valid. If start is zero, then 
*/
static size_t atbl_get_count_nocheck(Table *me, size_t home) {
    const size_t start_offset = me->table[home].arrow;
    const size_t end_offset = me->table[(home + 1) % me->cap].arrow;
    size_t count = 0;

    if (start_offset == EMPTY || start_offset == NOWHERE) {
        return 0;
    }

    if (start_offset == 0 && end_offset == EMPTY) {
        return 1;
    } else if (end_offset == EMPTY || end_offset == NOWHERE) {
        return 0;
    }

    count = (end_offset + 1 - start_offset);
    return count;
}


size_t atbl_get_firstemptyidx_nocheck(const Table *const me, size_t home) {
    size_t i = 0;

    for (i = 0; i < me->cap; ++i) {
        const size_t table_idx = (home + i) % me->cap;
        if (me->table[table_idx].arrow == EMPTY) {
            return table_idx;
        }
    }

    assert(0 && "impossible");
}

void atbl_moveidx_nocheck(Table *me, size_t src_idx, size_t dst_idx) {
    TableItem *src = &me->table[src_idx];
    TableItem *dst = &me->table[dst_idx];

    dst->hashcode = src->hashcode;
    dst->key = src->key;
    dst->value = src->value;
}

void atbl_insert_knownhashcode_nocheck(Table *me, KeyType key, size_t hashcode, size_t home, ValueType value) {
    size_t empty_idx = atbl_get_firstemptyidx_nocheck(me, home);

    me->table[empty_idx].arrow = NOWHERE;

    for (size_t table_idx = empty_idx; table_idx != home; table_idx = (table_idx - 1) % me->cap) {
        size_t count = atbl_get_count_nocheck(me, table_idx);

        if (count > 0) {
            size_t start_idx = table_idx + me->table[table_idx].arrow;
            size_t end_idx = (table_idx + 1 + me->table[(table_idx + 1) % me->cap].arrow) % me->cap;

            assert(end_idx == empty_idx && "end is not empty");

            atbl_moveidx_nocheck(me, start_idx, end_idx);

            empty_idx = start_idx;

            ++me->table[end_idx].arrow;
            if (atbl_get_count_nocheck(me, (table_idx - 1) % me->cap) == 0) {
                ++me->table[(table_idx - 1) % me->cap].arrow;
            }
        }
    }

    me->table[empty_idx].hashcode = hashcode;
    me->table[empty_idx].key = key;
    me->table[empty_idx].value = value;

    if (me->table[home].arrow == EMPTY || me->table[home].arrow == NOWHERE) {
        me->table[home].arrow = (empty_idx - home) % me->cap;
    }
    if (me->table[(home + 1) % me->cap].arrow == EMPTY) {
        /* no operation */
    } else if (me->table[(home + 1) % me->cap].arrow == NOWHERE) {
        /* offset (i.e. arrow) is the same between start and end if there is
        only one element. */
        me->table[(home + 1) % me->cap].arrow = me->table[home].arrow;
    } else {
        ++me->table[(home + 1) % me->cap].arrow;
    }
    ++me->len;
}

bool atbl_insert_knownhashcode(
    Table *me, KeyType key, size_t hashcode, size_t home, ValueType value) {
    size_t count = 0;

    /* Base case */
    if (me->table[home].arrow == EMPTY) {
        me->table[home].arrow = 0;
        me->table[home].hashcode = hashcode;
        me->table[home].key = key;
        me->table[home].value = value;
        /* me->table[home].arrow = 0 or EMPTY */

        ++me->len;
        return false;
    }
    
    /* If there are items belonging to 'home', check for the key. */
    count = atbl_get_count_nocheck(me, home);
    if (count > 0) {
        size_t offset = 0;
        for (offset = 0; offset < count; ++offset) {
            const size_t table_idx = (home + offset) % me->cap;
            TableItem *item_p = &me->table[table_idx];

            if (hashcode == item_p->hashcode && key_eq(key, item_p->key)) {
                item_p->value = value;
                return true;
            }
        }

        /* key not found */
        goto insert;
    }

insert:
    /* pass */
    atbl_insert_knownhashcode_nocheck(me, key, hashcode, home, value);

    return false;
}

bool atbl_insert(Table *me, KeyType key, ValueType value) {
    const size_t hashcode = key_hash(key);
    const size_t home = hashcode % me->cap;

    if (me->len >= GROWTH_THRESHOLD * me->cap) {
        TableItem *old_table = me->table;
        const size_t old_cap = me->cap;
        const size_t new_cap = 2 * me->cap;
        const size_t new_size = new_cap * sizeof(TableItem);
        size_t i = 0;

        assert(new_cap == new_size / sizeof(TableItem) && "overflow!");

        me->table = malloc(new_size);
        me->len = 0;
        me->cap = new_cap;

        for (i = 0; i < new_cap; ++i) {
            me->table[i].arrow = EMPTY;
        }

        for (i = 0; i < old_cap; ++i) {
            const TableItem *item = &old_table[i];
            if (item->arrow != EMPTY) {
                const size_t home = item->hashcode / me->cap;
                atbl_insert_knownhashcode(
                    me, item->key, item->hashcode, home, item->value);
            }
        }
    }

    return atbl_insert_knownhashcode(me, key, hashcode, home, value);
}


TableItem *atbl_search(Table *me, KeyType key) {
    const size_t hashcode = key_hash(key);
    const size_t home = hashcode % me->cap;
    const size_t count = atbl_get_count_nocheck(me, home);
    if (count > 0) {
        size_t offset = 0;
        for (offset = 0; offset < count; ++offset) {
            const size_t table_idx = (home + offset) % me->cap;
            TableItem *item_p = &me->table[table_idx];

            if (hashcode == item_p->hashcode && key_eq(key, item_p->key)) {
                return item_p;
            }
        }

        goto not_found;
    }
not_found:
    return NULL;
}


int main(void) {
    int r = 0;
    Table me = { 0 };

    r = tbl_init(&me, 10);
    assert(!r);

    atbl_insert(&me, 0, 0);
    atbl_insert(&me, 10, 0);
    atbl_insert(&me, 20, 0);
    atbl_insert(&me, 30, 0);
    atbl_insert(&me, 40, 0);
    atbl_insert(&me, 50, 0);
    atbl_insert(&me, 60, 0);
    atbl_insert(&me, 70, 0);
    tbl_debug_verbose(&me);

    atbl_insert(&me, 80, 0);
    tbl_debug_verbose(&me);

    atbl_insert(&me, 90, 0);
    tbl_debug_verbose(&me);
    atbl_insert(&me, 100, 0);
    tbl_debug_verbose(&me);
    atbl_insert(&me, 110, 0);
    tbl_debug_verbose(&me);
    atbl_insert(&me, 120, 0);
    tbl_debug_verbose(&me);


    return 0;
}