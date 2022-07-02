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

#include <vector>

#include "common.hpp"



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
static size_t _get_count_nocheck(Table *me, size_t home) {
    const size_t start_offset = me->table[home].arrow;
    const size_t end_offset = me->table[(home + 1) % me->cap].arrow;
    size_t count = 0;

    if (start_offset == EMPTY || start_offset == INVALID) {
        return 0;
    }

    if (start_offset == 0 && end_offset == INVALID) {
        return 1;
    } else if (end_offset == EMPTY || end_offset == INVALID) {
        return 0;
    }

    count = (end_offset + 1 - start_offset);

    /* This is a debugging assertion to make sure that my math is correct. Also
    it is good to just check that I didn't make any errors elsewhere. */
    assert(count >= 0); /* may be zero if start is end-pointer but end is a
    start pointer. */
    return count;
}




bool atbl_insertion(Table *me, KeyType key, ValueType value) {
    const size_t hashcode = key_hash(key);
    const size_t home = hashcode % me->cap;
    size_t offset = 0;
    size_t count = 0;

    if (me->len > GROWTH_THRESHOLD * me->cap) {
        assert(0 && "todo: allow expansion!");
    }
    
    /* If there are items belonging to 'home', check for the key. */
    count = _get_count_nocheck(me, home);
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

    return false;
}