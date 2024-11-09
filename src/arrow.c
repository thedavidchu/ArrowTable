#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>

#include "arrow.h"
#include "logger.h"

static size_t const DEFAULT_INIT_SIZE = 8;

/// @brief  The bounds of some index.
///
/// The 'start_idx' is index of the first element.
/// The 'stop_idx' is one past the index of the last element.
struct Bounds
{
    int start_idx;
    // NOTE stop_idx may be less than start_idx if it wraps around!
    int stop_idx;
};

static size_t
hash(int const key)
{
    assert(key >= 0);
    return (size_t)key;
}

static bool
is_ok(struct ArrowTable const *const me)
{
    // Short-circuit is OK! We won't dereference a NULL pointer.
    return me != NULL && me->data != NULL && me->capacity > 0;
}

/// @brief  Return whether there is a valid key/value pair residing in the cell.
static bool
cell_filled(struct ArrowTable const *const me, size_t const idx)
{
    assert(is_ok(me) && idx < me->capacity);
    return me->data[idx].key != -1;
}

/// @brief  Return whether there are valid arrows in the cell idx and idx+1.
/// @note   This says nothing about whether any valid elements hash%capacity to idx.
///         This is because we can have the following example:
///
///         ---------------------------------------
///         | CELL 0 | CELL 1  | CELL 2 | CELL 3  |
///         ---------------------------------------
///         | key: 0 | key: -1 | key: 2 | key: -1 |     * key = "key"
///         | val: 0 | val: -1 | val: 2 | val: -1 |     * val = "value"
///         | -->: 0 | -->: 0  | -->: 0 | -->: 0  |     * --> = "arrow"
///         ---------------------------------------
///
///         where we can see that cells 1 and 3 are invalid but their
///         arrows appear to be valid.
static bool
valid_arrows(struct ArrowTable const *const me, size_t const idx)
{
    size_t next_idx = 0;
    int my_arrow = 0, next_arrow = 0;

    assert(is_ok(me) && idx < me->capacity);

    next_idx = (idx + 1) % me->capacity;
    my_arrow = me->data[idx].arrow;
    next_arrow = me->data[next_idx].arrow;
    return my_arrow != -1 && next_arrow != -1;
}

/// @brief  Count the hash collisions (i.e. how big the bucket's array is).
static size_t
count_collisions(struct ArrowTable const *const me, size_t const idx)
{
    size_t next_idx = 0;
    int my_arrow = 0, next_arrow = 0;
    size_t cnt = 0;

    assert(is_ok(me) && idx < me->capacity);

    if (!cell_filled(me, idx) || !valid_arrows(me, idx)) {
        // TODO We can do a bunch of assertions to make sure everything is as expected.
        return 0;
    }
    next_idx = (idx + 1) % me->capacity;
    my_arrow = me->data[idx].arrow;
    next_arrow = me->data[next_idx].arrow;
    cnt = 1 + next_arrow - my_arrow;
    assert(cnt >= 0);
    return cnt;
}

static struct Bounds
get_bounds(struct ArrowTable const *const me, size_t const idx)
{
    size_t next_idx = 0;
    int my_arrow = 0, next_arrow = 0;

    assert(is_ok(me) && idx < me->capacity);

    if (count_collisions(me, idx) == 0) {
        // TODO We can do a bunch of assertions to make sure everything is as expected.
        return (struct Bounds){0, 0};
    }
    next_idx = (idx + 1) % me->capacity;
    my_arrow = me->data[idx].arrow;
    next_arrow = me->data[next_idx].arrow;
    return (struct Bounds){(idx + my_arrow) % me->capacity, (next_idx + next_arrow) % me->capacity};
}

/// @note   Arbitrarily set the threshold to grow at 90% full.
static bool
is_full_enough_to_grow(struct ArrowTable const *const me)
{
    assert(is_ok(me));
    return (double)(me->length + 1) / me->capacity >= 0.90;
}

/// @brief  Insert with the assumption that there's enough room.
/// @note   This is a clever recursive algorithm (so watch out!).
static int
insert_with_enough_room(struct ArrowTable *const me, int const key, int const value)
{
    // The 'victim' is the one who is kicked out of their current spot,
    // i.e. the 'rich' in Robin Hood lingo.
    size_t h = 0, idx = 0, next_idx = 0, victim_idx = 0;
    int victim_key = 0, victim_value = 0;
    // NOTE I assume no integer overflow in the length!
    assert(is_ok(me) && me->length + 1 < me->capacity);
    assert(key >= 0 && value >= 0);

    h = hash(key);
    idx = h % me->capacity;
    next_idx = (idx + 1) % me->capacity;

    // Cases:
    // 1. Spot empty, invalid arrows: simple insert and update arrows.
    // 2. Spot empty, valid arrows: insert without moving arrows.
    // 3. Spot filled, invalid arrows: search for next free spot.
    // 4. Spot filled, valid arrows: insert at tail, evict victim.
    if (!cell_filled(me, idx) && !valid_arrows(me, idx)) {
        LOGGER_TRACE("Case 1: key=%d, value=%d", key, value);
        me->data[idx] = (struct ArrowCell){.key = key, .value = value, .arrow = 0};
        me->data[next_idx].arrow = 0;
        ++me->length;
        return 0;
    } else if (!cell_filled(me, idx) && valid_arrows(me, idx)) {
        LOGGER_TRACE("Case 2: key=%d, value=%d", key, value);
        assert(me->data[idx].arrow == 0 && me->data[next_idx].arrow == 0);
        me->data[idx].key = key;
        me->data[idx].value = value;
        ++me->length;
        return 0;
    } else if (cell_filled(me, idx) && !valid_arrows(me, idx)) {
        LOGGER_TRACE("Case 3: key=%d, value=%d", key, value);
        // NOTE We already know that the cell at idx is filled.
        for (size_t i = idx + 1; i != idx; i = (i + 1) % me->capacity) {
            if (!cell_filled(me, i)) {
                // TODO Simply fill this spot and adjust the arrows
                size_t diff = (i + me->capacity - idx) % me->capacity;
                me->data[i] = (struct ArrowCell){.key = key, .value = value, .arrow = 0};
                me->data[idx].arrow = diff;
                me->data[next_idx].arrow = diff;
                ++me->length;
                return 0;
            } else if (count_collisions(me, i) != 0) {
                struct Bounds b = get_bounds(me, i);
                // TODO Fill the start_idx spot and adjust the arrows. Recurse on victim.
                size_t diff = (i + me->capacity - idx) % me->capacity;
                victim_idx = b.start_idx;
                victim_key = me->data[victim_idx].key;
                victim_value = me->data[victim_idx].value;
                me->data[victim_idx].key = key;
                me->data[victim_idx].value = value;
                ++me->data[i].arrow;
                me->data[idx].arrow = diff;
                me->data[next_idx].arrow = diff;
                if (victim_key == -1)
                    return 0;
                LOGGER_TRACE("Case 3 (cont'd): victim_key=%d, victim_value=%d", victim_key, victim_value);
                return insert_with_enough_room(me, victim_key, victim_value);
            }
        }
        assert(0 && "IMPOSSIBLE!");
    } else if (cell_filled(me, idx) && valid_arrows(me, idx)) {
        LOGGER_TRACE("Case 4: key=%d, value=%d", key, value);
        victim_idx = get_bounds(me, idx).stop_idx;
        victim_key = me->data[victim_idx].key;
        victim_value = me->data[victim_idx].value;
        me->data[victim_idx].key = key;
        me->data[victim_idx].value = value;
        ++me->data[next_idx].arrow;
        if (victim_key == -1) {
            ++me->length;
            return 0;
        }
        LOGGER_TRACE("Case 4 (cont'd): victim_key=%d, victim_value=%d", victim_key, victim_value);
        return insert_with_enough_room(me, victim_key, victim_value);
    } else {
        assert(0 && "IMPOSSIBLE!");
    }
}

/// @brief  Double the size of the hash table.
/// @note   We don't support shrinking the hash table. Too bad, so sad!
static int
grow_hash_table(struct ArrowTable *const me)
{
    struct ArrowTable old_table = {0};
    struct ArrowTable new_table = {0};

    assert(is_ok(me));

    old_table = *me;
    new_table = (struct ArrowTable){
        .data = calloc(old_table.capacity, sizeof(*new_table.data)),
        .capacity = 2 * old_table.capacity,
        .length = 0,
    };
    if (new_table.data == NULL) {
        assert(errno);
        return errno;
    }
    for (size_t i = 0; i < new_table.capacity; ++i) {
        new_table.data[i].key = -1;
        new_table.data[i].arrow = -1;
    }

    // Fill new table with existing data
    // NOTE I could also simply fill this with all the valid key/value pairs.
    for (size_t i = 0; i < old_table.capacity; ++i) {
        if (cell_filled(me, i)) {
            insert_with_enough_room(&new_table, old_table.data[i].key, old_table.data[i].value);
        }
    }
    // Cleanup temporary structures
    ArrowTable_destroy(&old_table);
    *me = new_table;
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// EXTERNAL FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

int
ArrowTable_init(struct ArrowTable *const me)
{
    if (me == NULL || me->data != NULL || me->length != 0 || me->capacity != 0) {
        return -1;
    }
    // NOTE I don't deal with the errno if it's set before and I don't clean up afterwards.
    me->data = calloc(8, sizeof(*me->data));
    if (me->data == NULL) {
        assert(errno);
        return errno;
    }
    // Set all of the cells to the INVALID state.
    for (size_t i = 0; i < DEFAULT_INIT_SIZE; ++i) {
        me->data[i].key = -1;
        me->data[i].arrow = -1;
    }
    me->length = 0;
    me->capacity = 8;
    return 0;
}

int
ArrowTable_destroy(struct ArrowTable *const me)
{
    if (me == NULL) {
        return -1;
    }
    free(me->data);
    *me = (struct ArrowTable){0};
    return 0;
}

void
ArrowTable_print(struct ArrowTable const *const me, FILE *const stream, bool const newline)
{
    if (!me || stream == NULL) return;
    fprintf(stream, "ArrowTable(.data={\n");
    for (size_t i = 0; i < me->capacity; ++i) {
        fprintf(stream, "\t%zu: {.key=%d,.value=%d,.arrow=%d},\n", i, me->data[i].key, me->data[i].value, me->data[i].arrow);
    }
    fprintf(stream, "}, .length = %zu, .capacity = %zu)", me->length, me->capacity);
    if (newline) fprintf(stream, "\n");
}

int
ArrowTable_get(struct ArrowTable const *const me, int const key)
{
    size_t h = 0;
    struct Bounds bounds = {0};

    if (!is_ok(me) || key < 0) {
        return -1;
    }

    h = hash(key);
    bounds = get_bounds(me, h % me->capacity);
    if (bounds.start_idx == bounds.stop_idx) {
        // Key not found! This shouldn't be an error in the same sense as above...
        return -1;
    }
    for (size_t idx = bounds.start_idx; idx != bounds.stop_idx; idx = (idx + 1) % me->capacity) {
        if (me->data[idx].key == key) {
            return me->data[idx].value;
        }
    }
    return -1;
}

int
ArrowTable_put(struct ArrowTable *const me, int const key, int const value)
{
    int err = 0;
    if (!is_ok(me) || key < 0 || value < 0) {
        return -1;
    }
    if (is_full_enough_to_grow(me)) {
        if ((err = grow_hash_table(me))) {
            return err;
        }
    }
    return insert_with_enough_room(me, key, value);
}

int
ArrowTable_remove(struct ArrowTable *const me, int const key)
{
    return 0;
}

