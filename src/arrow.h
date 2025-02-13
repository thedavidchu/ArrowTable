/** @brief  This is a prototype for the Arrow Table, a modified version of the
 *          Robinhood Hash Table.
 */
#pragma once

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

/// NOTE    Keys and values must be non-negative! This is simply for ease of implementation.
struct ArrowCell {
    // A key of -1 signals an INVALID cell.
    int key;
    // A value of -1 would signal an error in the 'get' function.
    int value;
    // An arrow of -1 signals INVALID
    int arrow;
};

struct ArrowTable {
    struct ArrowCell *data;
    // Number of elements in the ArrowTable
    size_t length;
    // Number of slots in the ArrowTable
    size_t capacity;
};


int
ArrowTable_init(struct ArrowTable *const me);

int
ArrowTable_destroy(struct ArrowTable *const me);

void
ArrowTable_print(struct ArrowTable const *const me, FILE *const stream, bool const newline);

/// @brief  Get a value from the ArrowTable.
/// @return Returns the value or -1 on failure.
int
ArrowTable_get(struct ArrowTable const *const me, int const key);

/// @brief  Put a value into the ArrowTable.
/// @return Return 0 on success; other codes result from failure.
int
ArrowTable_put(struct ArrowTable *const me, int const key, int const value);

/// @brief  Delete a key, value pair from the ArrowTable.
/// @return Return 0 on success; other codes result from failure.
int
ArrowTable_remove(struct ArrowTable *const me, int const key);
