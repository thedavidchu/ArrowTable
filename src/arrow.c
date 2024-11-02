#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>

#include "arrow.h"

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

/// @brief  Count the hash collisions (i.e. how big the bucket's array is).
static size_t
count_collisions(struct ArrowTable const *const me, size_t const idx)
{
    size_t next_idx = 0;
    int my_arrow = 0, next_arrow = 0;
    size_t cnt = 0;

    assert(is_ok(me) && idx < me->capacity);

    next_idx = (idx + 1) % me->capacity;
    my_arrow = me->data[idx].arrow;
    next_arrow = me->data[next_idx].arrow;

    if (my_arrow < 0 || next_arrow == -2) {
        assert(my_arrow == -1 || my_arrow == -2);
        // TODO We can do a bunch of assertions to make sure everything is as expected.
        return 0;
    }
    if (my_arrow == 0 && next_arrow == -1) {
        return 1;
    }

    cnt = 1 + next_arrow - my_arrow;
    assert(cnt > 0);
    return cnt;
}

static struct Bounds
get_bounds(struct ArrowTable const *const me, size_t const idx)
{
    size_t next_idx = 0;
    int my_arrow = 0, next_arrow = 0;

    assert(me != NULL && me->data != NULL && me->capacity > 0);
    assert(idx < me->capacity);

    next_idx = (idx + 1) % me->capacity;
    my_arrow = me->data[idx].arrow;
    next_arrow = me->data[next_idx].arrow;

    if (my_arrow < 0 || next_arrow == -2) {
        assert(my_arrow == -1 || my_arrow == -2);
        // TODO We can do a bunch of assertions to make sure everything is as expected.
        return (struct Bounds){0, 0};
    }
    if (my_arrow == 0 && next_arrow == -1) {
        return (struct Bounds){idx, next_idx};
    }
    return (struct Bounds){(idx + my_arrow) % me->capacity, (next_idx + next_arrow) % me->capacity};
}

/// @brief  Return whether there is a valid key/value pair residing in the cell.
static bool
cell_filled(struct ArrowTable const *const me, size_t const idx)
{
    assert(is_ok(me) && idx < me->capacity);
    return me->data[idx].key != -1;
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
    size_t h = 0, idx = 0, next_idx = 0;
    // NOTE I assume no integer overflow in the length!
    assert(is_ok(me) && me->length + 1 < me->capacity);
    assert(key >= 0 && value >= 0);

    h = hash(key);
    idx = h % me->capacity;
    next_idx = (idx + 1) % me->capacity;


    struct Bounds bounds = get_bounds(me, idx);
    
    // Cases:
    // 1. Spot empty, invalid arrows: simple insert.
    // 2. Spot empty, valid arrows: IMPOSSIBLE!
    // 3. Spot filled, invalid arrows: search for next free spot.
    // 4. Spot filled, valid arrows: insert at tail, evict victim.
    size_t victim_idx = 0;
    int victim_key = 0;
    int victim_value = 0;
    if (!cell_filled(me, idx)) { // Case 1.
        me->data[idx] = (struct ArrowCell){.key = key, .value = value, .arrow = 0};
        me->data[next_idx].arrow = cell_filled(me, next_idx) ? 0 : -1;
        ++me->length;
        return 0;
    } else if (bounds.start_idx == bounds.stop_idx) { // Case 3.
        // NOTE We already know that the cell at idx is filled.
        for (size_t i = idx + 1; i != idx; i = (i + 1) % me->capacity) {
            struct Bounds b = get_bounds(me, i);
            if (!cell_filled(me, i)) {
                // TODO Simply fill this spot and adjust the arrows
                size_t diff = (i + me->capacity - idx) % me->capacity;
                me->data[i] = (struct ArrowCell){.key = key, .value = value, .arrow = 0};
                me->data[idx].arrow = diff;
                me->data[next_idx].arrow = diff;
                ++me->length;
                return 0;
            } else if (b.start_idx != b.stop_idx) {
                // TODO Fill the start_idx spot and adjust the arrows. Recurse on victim.
                size_t diff = (i + me->capacity - idx) % me->capacity;
                victim_key = me->data[b.start_idx].key;
                victim_value = me->data[b.start_idx].value;
                me->data[b.start_idx].key = key;
                me->data[b.start_idx].value = value;
                ++me->data[i].arrow;
                me->data[idx].arrow = diff;
                me->data[next_idx].arrow = diff;
                return insert_with_enough_room(me, victim_key, victim_value);
            }
        }
        assert(0 && "IMPOSSIBLE!");
    } else { // Case 4.
        victim_key = me->data[bounds.stop_idx].key;
        victim_value = me->data[bounds.stop_idx].value;
        me->data[bounds.stop_idx].key = key;
        me->data[bounds.stop_idx].value = value;
        ++me->data[next_idx].arrow;
        if (victim_key == -1) {
            ++me->length;
            return 0;
        }
        // TODO Increment the arrow for the thing we just incremented.
        return insert_with_enough_room(me, victim_key, victim_value);
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
        new_table.data[i].arrow = -2;
    }

    // Fill new table with existing data
    // NOTE I could also simply fill this with all the valid key/value pairs.
    for (size_t i = 0; i < old_table.capacity; ++i) {
        struct Bounds bounds = get_bounds(&old_table, i);
        if (bounds.start_idx == bounds.stop_idx) continue;
        for (size_t j = bounds.start_idx; j != bounds.stop_idx; j = (j + 1) % old_table.capacity) {
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
    for (size_t i = 0; i < 8; ++i) {
        me->data[i].key = -1;
        me->data[i].arrow = -2;
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

