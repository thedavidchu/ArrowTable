/** Arrow Table Implementation
 *
 * Language: C99
 * 
 *
 */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "common.h"


/** Get the number of items belonging to 'home'.
 * 
 * If start is empty or invalid (i.e. points to nothing), then we have zero
 * items belonging to the home.
 * 
 * Next, we want to check if the end is valid. If start is zero and end is
 * empty, then we have 1 item. Otherwise, we return the difference between
 * the points in the start and the end.
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

void atbl_moveidx_nocheck(Table *const me, size_t src_idx, size_t dst_idx) {
    TableItem *src = &me->table[src_idx];
    TableItem *dst = &me->table[dst_idx];

    dst->hashcode = src->hashcode;
    dst->key = src->key;
    dst->value = src->value;
}

void atbl_insert_knownhashcode_nocheck(Table *const me, KeyType key, size_t hashcode, const size_t home, ValueType value) {
    const size_t after_home = (home + 1) % me->cap;
    size_t empty_idx = atbl_get_firstemptyidx_nocheck(me, home);
    
    if (empty_idx != home && empty_idx != after_home) {
        if (me->table[(empty_idx - 1) % me->cap].arrow == 0) {
            me->table[empty_idx].arrow = 1;
        } else {
            me->table[empty_idx].arrow = NOWHERE;
        }
    }

    for (size_t table_idx = empty_idx; table_idx != home; table_idx = (table_idx - 1) % me->cap) {
        size_t count = atbl_get_count_nocheck(me, table_idx);
        if (count > 0) {
            size_t start_idx = table_idx + me->table[table_idx].arrow;
            size_t end_idx = (table_idx + 1 + me->table[(table_idx + 1) % me->cap].arrow) % me->cap;

            assert(end_idx == empty_idx && "end is not empty");

            atbl_moveidx_nocheck(me, start_idx, end_idx);

            empty_idx = start_idx;

            ++me->table[(table_idx + 1) % me->cap].arrow;
            /* If the next square we check relies on start being the the correct
            spot, we don't change it yet. The next square promises to change it
            though. */
            if (atbl_get_count_nocheck(me, (table_idx - 1) % me->cap) == 0) {
                ++me->table[table_idx].arrow;
            }
        }
    }

    me->table[empty_idx].hashcode = hashcode;
    me->table[empty_idx].key = key;
    me->table[empty_idx].value = value;
    switch (me->table[home].arrow) {
    case EMPTY:
        me->table[home].arrow = 0;
        break;
    case NOWHERE:
        /* Point to empty_idx */
        me->table[home].arrow = (empty_idx + me->cap - home) % me->cap;
        /* There is guaranteed to be exactly 1 item and the next square is
        guaranteed to be occupied; this means the arrows for home and home+1 are
        identical. */
        me->table[(home + 1) % me->cap].arrow = me->table[home].arrow;
        break;
    default:
        do {
            const size_t home_arrow = me->table[home].arrow;
            const size_t after_home_arrow = me->table[after_home].arrow;

            if (after_home_arrow == EMPTY && home_arrow == 0) {
                /* non-empty */
                me->table[after_home].arrow = 1;
            } else if (after_home_arrow == EMPTY && home_arrow == 1 || after_home_arrow == NOWHERE || after_home_arrow + 1 == home_arrow) {
                /* empty */
                me->table[home].arrow = (empty_idx + me->cap - home) % me->cap;
                me->table[after_home].arrow = me->table[home].arrow; 
            } else {
                /* one-or-more items */
                ++me->table[after_home].arrow; 
            }
        } while (0);
        break;
    }
    ++me->len;
}

bool atbl_insert_knownhashcode(
    Table *me, KeyType key, size_t hashcode, size_t home, ValueType value) {
    size_t count = 0;

    /* If there are items belonging to 'home', check for the key. */
    TableItem *item_p = atbl_search_knownhashcode_nocheck(me, key, hashcode, home);
    if (item_p != NULL) {
        item_p->value = value;
        return true;
    }

insert:
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
                const size_t home = item->hashcode % me->cap;
                atbl_insert_knownhashcode_nocheck(
                    me, item->key, item->hashcode, home, item->value);
            }
        }
    }
    /* pass */
    atbl_insert_knownhashcode_nocheck(me, key, hashcode, home, value);
    return false;
}

bool atbl_insert(Table *me, KeyType key, ValueType value) {
    const size_t hashcode = key_hash(key);
    const size_t home = hashcode % me->cap;

    return atbl_insert_knownhashcode(me, key, hashcode, home, value);
}


TableItem *atbl_search_knownhashcode_nocheck(const Table *const me, const KeyType key, const size_t hashcode, const size_t home) {
    const size_t count = atbl_get_count_nocheck(me, home);
    
    if (count > 0) {
        const size_t start_idx = (home + me->table[home].arrow) % me->cap;
        size_t offset = 0;
        for (offset = 0; offset < count; ++offset) {
            const size_t table_idx = (start_idx + offset) % me->cap;
            TableItem *item_p = &me->table[table_idx];
            printf("%p\n", item_p);
            if (hashcode == item_p->hashcode && key_eq(key, item_p->key)) {
                return item_p;
            }
        }
        goto not_found;
    }
not_found:
    return NULL;
}


TableItem *atbl_search(const Table *const me, const KeyType key) {
    const size_t home = hashcode % me->cap;
    const size_t hashcode = key_hash(key);
    
    return atbl_search_knownhashcode_nocheck(me, key, hashcode, home);
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

    // atbl_insert(&me, 90, 0);
    // tbl_debug_verbose(&me);
    // atbl_insert(&me, 100, 0);
    // tbl_debug_verbose(&me);
    // atbl_insert(&me, 110, 0);
    // tbl_debug_verbose(&me);
    // atbl_insert(&me, 120, 0);
    // tbl_debug_verbose(&me);

    atbl_insert(&me, 1, 0);
    atbl_insert(&me, 21, 0);
    atbl_insert(&me, 41, 0);
    atbl_insert(&me, 61, 0);
    atbl_insert(&me, 81, 0);
    atbl_insert(&me, 101, 0);
    atbl_insert(&me, 121, 0);
    tbl_debug_verbose(&me);
    atbl_insert(&me, 121, 1);
    tbl_debug_verbose(&me);
    atbl_insert(&me, 121, 2);
    tbl_debug_verbose(&me);
    atbl_insert(&me, 121, 3);
    tbl_debug_verbose(&me);
    atbl_insert(&me, 121, 4);
    tbl_debug_verbose(&me);
    
    assert(atbl_search(&me, 121)->key == 121);
    assert(atbl_search(&me, 122) == NULL);

    return 0;
}
