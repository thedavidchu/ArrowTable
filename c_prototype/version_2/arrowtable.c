#include <assert.h>
#include <stdbool.h>
#include <string.h>     // strcmp

#include "arrowtable.h"

#define MAX_SIZE ((size_t)(-1))
#define NOWHERE (MAX_SIZE)

struct Range {
    size_t start, end;
};

/* Taken from http://www.cse.yorku.ca/~oz/hash.html */
static size_t
sdbm_hash(const char *const buf) {
    size_t hash = 0;
    for (size_t i = 0; buf[i] != '\0'; ++i) {
        hash = buf[i] + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}

/* Return true if hashcodes and non-null strings match. It assumes the strings are not NULL. */
static bool
key_eq(const char *const key_a, const size_t hashcode_a, const char *const key_b, const size_t hashcode_b) {
    return hashcode_a == hashcode_b &&
            // Premature optimization, but if key_a and key_b are the same pointer, then they are equal
            (key_a == key_b || strcmp(key_a, key_b) == 0);
}

/* Get the range where item can be inserted or found relative to the home_idx. */
static struct Range
get_range(struct Table *me, const size_t home_idx) {
    const size_t after_home_idx = (home_idx + 1) % me->size;    // Index 1 past home
    const bool valid_a = me->table[home_idx].valid;
    const size_t offset_a = me->table[home_idx].offset;
    const bool valid_b = me->table[after_home_idx].valid;
    const size_t offset_b = me->table[after_home_idx].offset;
    // Check first for validity and useful offset
    if (!valid_a) {
        return (struct Range){.start = 0, .end = 0};
    }
    if (offset_a == NOWHERE) {
        if (!valid_b) { return (struct Range){.start = 1, .end = 1}; }
        if (offset_b != NOWHERE) { return (struct Range){.start = offset_b + 1, .end = offset_b + 1}; }
        // TODO(dchu): figure out math! I believe we just find the first empty or arrow?
        for (size_t i = 0; i < me->size; ++i) {
            const struct Node n = me->table[(home_idx + i) % me->size];
            if (!n.valid) { return (struct Range){.start = i, .end = i}; }
            if (n.offset != NOWHERE) { return (struct Range){.start = n.offset, .end = n.offset}; }
        }
        assert(0 && "no room!");
    }
    // Check second for validity and useful offset
    if (!valid_b) {
        if (offset_a == 0) { return (struct Range){.start = 0, .end = 1}; }
        if (offset_a == 1) { return (struct Range){.start = 1, .end = 1}; }
        // We cannot have the first offset point past an empty square (i.e. the second square).
        assert(0 && "impossible!");
    }
    if (offset_b == NOWHERE) {
        return (struct Range){.start = offset_a, .end = offset_a};
    }
    return (struct Range){.start = offset_a, .end = offset_b + 1};
}

/* Find the index of a valid null-terminated string and hashcode. Return -1 if not found. */
static size_t
find_index(struct Table *me, char *key, const size_t hashcode) {
    assert(key != NULL && "key is NULL!");
    size_t home_idx = hashcode % me->size;
    struct Range range = get_range(me, home_idx);
    size_t start_idx = (home_idx + range.start) % me->size;
    size_t end_idx = (home_idx + range.end) % me->size;
    for (size_t idx = start_idx; idx != end_idx; idx = (idx + 1) % me->size) {
        struct Node *n = &me->table[idx];
        if (n->valid && key_eq(n->key, n->hashcode, key, hashcode)) {
            return idx;
        }
    }
    return MAX_SIZE;
}

/* Identical to find_index except return end if not found */
static size_t
find_index_or_end(struct Table *me, char *key, const size_t hashcode) {
    assert(key != NULL && "key is NULL!");
    size_t home_idx = hashcode % me->size;
    struct Range range = get_range(me, home_idx);
    size_t start_idx = (home_idx + range.start) % me->size;
    size_t end_idx = (home_idx + range.end) % me->size;
    for (size_t idx = start_idx; idx != end_idx; idx = (idx + 1) % me->size) {
        struct Node *n = &me->table[idx];
        if (n->valid && key_eq(n->key, n->hashcode, key, hashcode)) {
            return idx;
        }
    }
    // For insertions, we can just return `end_idx`
    return end_idx;
}

////////////////////////////////////////////////////////////////////////////////
/// PUBLIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

void create(struct Table *me) {
    assert(me != NULL && "NULL pointer");
    me->size = SIZE;
    me->length = 0;

    for (size_t i = 0; i < me->size; ++i) {
        me->table[i] = (struct Node){.valid = false, .hashcode = 0, .key = NULL, .value = 0, .offset = 0};
    }
}

/* Destroys what is assumed to be a valid Table, since we use the size attribute. */
void destroy(struct Table *me) {
    for (size_t i = 0; i < me->size; ++i) {
        me->table[i] = (struct Node){.valid = false, .hashcode = 0, .key = NULL, .value = 0, .offset = 0};
    }
    me->size = 0;
    me->length = 0;
}

/* We reuse the key that is passed to this function, which means that it must
remain both constant and must not be freed until we free it from this hash table. */
int insert(struct Table *me, char *key, int value) {
    if (key == NULL) { return -1; }
    const size_t hashcode = sdbm_hash(key);
    const size_t idx = find_index_or_end(me, key, hashcode);
    debug(me);
    if (me->table[idx].valid && key_eq(me->table[idx].key, me->table[idx].hashcode, key, hashcode)) {
        me->table[idx].value = value;
        return 0;
    }
    debug(me);
    if (!me->table[idx].valid) {
        me->table[idx].valid = true;
        me->table[idx].hashcode = hashcode;
        me->table[idx].key = key;
        me->table[idx].value = value;
        me->table[idx].offset = 0;
        // Set next if applicable
        if (me->table[(hashcode + 1) % me->size].valid) {
            me->table[(hashcode + 1) % me->size].offset += 1;
        }
        return 0;
    }
    debug(me);
    struct Node tmp = me->table[idx];
    me->table[idx].hashcode = hashcode;
    me->table[idx].key = key;
    me->table[idx].value = value;
    // Set next if applicable
    if (me->table[(hashcode + 1) % me->size].valid) {
        me->table[(hashcode + 1) % me->size].offset += 1;
    }
    // TODO(dchu): we can optimize by not recomputing the hash again!
    return insert(me, tmp.key, tmp.value);
}

int search(struct Table *me, char *key, int *value) {
    if (key == NULL) { return -1; }

    const size_t hashcode = sdbm_hash(key);
    const size_t idx = find_index(me, key, hashcode);

    if (idx == MAX_SIZE) { return -1; }
    
    *value = me->table[idx].value;
    return 0;
}

int delete(struct Table *me, char *key) {
    assert(0 && "not implemented");
}

////////////////////////////////////////////////////////////////////////////////
/// DEBUGGING FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
void print(struct Table *me) {
    printf("{\n");
    for (size_t i = 0; i < me->size; ++i) {
        struct Node n = me->table[i];
        printf("\t%zu. valid: %s, hashcode: %zu, key: \"%s\", value: %d, offset: %zu\n",
                i, n.valid ? "true" : "false", n.hashcode, n.key, n.value, n.offset);
    }
    printf("}\n");
}

void debug(struct Table *me) {
    for (size_t i = 0; i < me->size; ++i) {
        struct Range r = get_range(me, i);
        size_t start_idx = (i + r.start) % me->size;
        size_t end_idx = (i + r.end) % me->size;

        for (size_t j = start_idx; j != end_idx; j = (j + 1) % me->size) {
            assert(me->table[j].hashcode % me->size == i);
        }
    }
}