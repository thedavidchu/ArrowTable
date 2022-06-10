ArrowTable - A Modified Robin Hood Hash Table
=============================================

This repository contains a Rust implementation of a modified Robin Hood hash table. Instead of storing the offset from the ideal position, we invert this idea and for each 'home' location, we store the offset to the first of its item.

I believe this will improve search speed but may lead to poorer performance for deleting.

Naming
------

The key difference between this implementation and the original Robin Hood hash table is where the latter tracks the "wealth" of squares (i.e. how far an item is away from its "home" square), this one's "home" square tracks its items with an _arrow_-like pointer. Also, Robin Hood uses a bow and arrow, so of course the name pays homage to the implementation.

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

Potential Optimizations
-----------------------

This is a list of potential optimizations. Not every "optimization" is a good idea. Each has its own tradeoffs.

### Definitions

The following definitions may be a bit circular.

* **Sparse (Index) Lookup (Table)** : this is the sparse lookup where a hash code is converted to an **primary index** that we use to lookup a **secondary index** that will be used to find the corresponding item in the **dense item lookup table** (see below). This is what one thinks of as a conventional hash table.
* **Primary Index** : the index used to find **secondary indices** in the **sparse index lookup table**.
* **Dense (Item) Lookup (Table)** : this is simply an expandable array that stores the items (i.e. hashcode, key, value). There may be missing values (which Rust will not like).
* **Secondary Index** : the index used to find items in the **dense item lookup table**.

### Optimizations

* Python compact dict
  * Store the hash (or some part of the hash) here so we can skip ones that definitely don't match (use part not required to compute "home" if using a subset, because we've already clusted with the bits used to compute the "home")
  * Store the arrow in the lookup table. This increases the amount of memory we need to load from cache, but also if we don't get an immediate hit, then this is no good.
  * Change the size of the **primary index** for the lookup. Smaller tables need fewer indices.
  * Figure out amortized deletion for the dense item lookup. How will we delete items? Rust does not like dangling pointers, so this might be a bit _unsafe_ (i.e. I'll have to learn unsafe Rust).

References
----------

### Coding

This code was made thanks to the Rust Book.

### Algorithm and Data Structures

Inspiration comes from:

* Robin Hood Hash Table
  * Structurally, the Robin Hood Hash Table and this are almost identical except for the "wealth" versus the "arrow".
* Python's Compact, Ordered Dict 
  * I'm not actually sure if this is the best inspiration, because the index lookup table is twice as long in my version because we may need to store the "arrow" in the sparse lookup table because we are not guaranteed that a square with a valid "arrow" will have a valid item.

License
-------

This repository will remain private until it's ready to show the world (or I run out of free Github Actions compute or storage).
