#include <assert.h>
#include <stddef.h>

#include <vector>

#define INVALID ((size_t)-1)
#define NUM2IDX(tbl, num) ((num) % (tbl.cap))

typedef int KeyType;
typedef int ValueType;


typedef struct {
    bool valid;
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

size_t hash_key(KeyType key) {
    return (size_t)key;
}

static void _get_bounds_nocheck(Table *me, size_t home, size_t bounds[2]) {


}

static size_t _get_count_nocheck(Table *me, size_t home) {
    
}

Table 

bool Table_insertion(Table *me, KeyType key, ValueType value) {
    size_t hashcode = hash_key(key);
    size_t home = NUM2IDX(me, hashcode);

    if (me.size > INVALID / 2) {
        assert(0 && "too big!");
    }
}