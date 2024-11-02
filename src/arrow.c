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
    return me != NULL && me->data != NULL && me->capacity != 0;
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
    size_t h = 0, idx = 0;
    // NOTE I assume no integer overflow in the length!
    assert(is_ok(me) && me->length + 1 < me->capacity);

    h = hash(key);
    idx = h % me->capacity;

    struct Bounds bounds = get_bounds(me, idx);
    
    if (1) {
        me->data[idx] = (struct ArrowCell){.key = key, .value = value, .arrow = 0};
        ++me->length;
        return 0;
    }
    
    int victim_key = 0;
    int victim_value = 0;
    return insert_with_enough_room(me, victim_key, victim_value);
}

/// @brief  Double the size of the hash table.
/// @note   We don't support shrinking the hash table. Too bad, so sad!
static int
grow_hash_table(struct ArrowTable const *const me)
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

    // Fill new data
    for (size_t i = 0; i < old_table.capacity; ++i) {
        struct Bounds bounds = get_bounds(&old_table, i);
        if (bounds.start_idx == bounds.stop_idx) continue;
        for (size_t j = bounds.start_idx; j != bounds.stop_idx; j = (j + 1) % old_table.capacity) {
            insert_with_enough_room(&new_table, old_table.data[i].key, old_table.data[i].value);
        }
    }
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

