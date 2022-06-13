Modified Robin Hood Hash Table
==============================

This repository contains a Rust implementation of a modified Robin Hood hash table. Instead of storing the offset from the ideal position, we invert this idea and for each 'home' location, we store the offset to the first of its item.

I believe this will improve search speed but may lead to poorer performance for deleting.

Purpose
-------

The purpose of this repository is two-fold. First, I have been meaning to explore an idea I had for improving the Robin Hood hash table. Second, I wanted to learn a little bit of Rust in a useful setting.

Definitions
-----------

* **Key** : a key is what is used to have a fast lookup (c.f. Python).
* **Value** : a value is what is associated with a key (c.f. Python).
* **Hashcode** : a hashcode is the computed hash value that is deterministically computed from the key.
* **Item** : an item is the collection of key-value-hashcode as well as the offset (see below).

* **Hash Table** : (abbrev "table") This word is overloaded. It can either refer to the whole table data structure or just the table of items.
* **Length** : (abbrev "len") the number of items in the hash table.
* **Capacity** : (abbrev "cap") the number of items that be fit contiguously in the hash table.

* **Home** : this refers to the location that item would ideally be if no other items were present.
* **Offset** : this is the difference between where an item is stored and its home.

Data Structure
--------------

**TODO**

```rust
/// Given the types KeyType and ValueType as generics:

struct TableItem {
    offset: either {INVALID, usize},
    
    item: either {
        INVALID, 
        {hashcode: usize, key: KeyType, value: ValueType}
    },
};

struct Table {
    table: vector<TableItem>,  // All set to INVALID
    len: usize,
    cap: usize,
};
```


Algorithms
----------

### Insertion

```ts
/// Let TableInstance[index] return the `index` element of TableInstance.table.

function get_number_belonging_to_home(self: Table, home: usize) -> usize {
    if self.table[home].offset != INVALID and self.table[home + 1].offset != INVALID {
        return (home + self.table[home].offset) - (home + 1 + self.table[home + 1].offset);
    }
    return 0;
}

function insertion(self: Table, key: KeyType, value: ValueType) -> Result<bool, ErrMsg> {
    let hashcode: usize = hash of key;
    let home: usize = first searched index based on the hashcode;
    
    if number_belonging_to_home(self, home) > 0 {
        let start: usize = home + self.table[home].offset;
        let end: usize = home + 1 + self.table[home + 1].offset;
        
        if key_between(start, end) { // Not including `end` (`end` is the first of the next)
            change_value_of(key);
            return FOUND_KEY;
        }
        
        // generic insert at `end`
        goto generic_insert;
    } else if self.table[home].offset != INVALID { // home is end for other square
        if END_CONDITION {
            put (key, value, hashcode) at end;
            return KEY_NOT_FOUND;
        }
        goto generic_insert;
    } else { // home.offset is INVALID
        goto generic_insert;
    }
    
generic_insert:
    // Recursive algorithm until END_CONDITION (empty spot with no need to displace other items)
        1. find first (home + i) for all i in {1..self.len-1} s.t. (home + i).offset != INVALID -- i.e. first valid offset
        2. insert recursively, expecting not to find key
        3. increment (HOME+i).offset

    return KEY_NOT_FOUND;
}
```

### Search

**TODO**

### Deletion

**TODO**

License
-------

This repository will remain private until it's ready to show the world (or I run out of free Github Actions compute or storage).
