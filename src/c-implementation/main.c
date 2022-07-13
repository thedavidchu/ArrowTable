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



void atbl_insert_nocheck(Table *me, KeyType key, size_t hashcode, size_t home, ValueType value) {
    size_t empty_idx = atbl_get_firstemptyidx_nocheck(me, home);

    for (size_t table_idx = empty_idx; table_idx != home; table_idx = (table_idx - 1) % me->cap) {
        size_t count = atbl_get_count_nocheck(me, table_idx);

        if (count > 0) {
            size_t start_idx = table_idx + me->table[table_idx].arrow;
            size_t end_idx = (table_idx + 1 + me->table[(table_idx + 1) % me->cap].arrow) % me->cap;

            atbl_moveidx_nocheck(me, start_idx, end_idx);

            ++me->table[end_idx].arrow;
            if (atbl_get_count_nocheck(me, (table_idx - 1) % me->cap) == 0) {
                ++me->table[(table_idx - 1) % me->cap].arrow;
            }
        }
    }

    // /* either home+1 + arrow */
    // switch (me->table[(home + 1) % me->cap].arrow) {
    // case EMPTY:
    //     me->table[(home + 1)];
    // case NOWHERE:
    // default:
    // }
    // atbl_moveidx_nocheck(me, /*src*/0, empty_idx);

    // ++me->len;
}

bool atbl_insertion(Table *me, KeyType key, ValueType value) {
    const size_t hashcode = key_hash(key);
    const size_t home = hashcode % me->cap;
    size_t offset = 0;
    size_t count = 0;

    if (me->len > GROWTH_THRESHOLD * me->cap) {
        assert(0 && "todo: allow expansion!");
    }

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
    atbl_insert_nocheck(me, key, hashcode, home, value);

    return false;
}

int main(void) {
    int r = 0;
    Table me = { 0 };

    r = tbl_init(&me, 10);
    assert(!r);

    atbl_insertion(&me, 0, 0);
    atbl_insertion(&me, 10, 0);
    atbl_insertion(&me, 20, 0);
    tbl_debug_verbose(&me);


    return 0;
}