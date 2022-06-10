Modified Robin Hood Hash Table
==============================

This repository contains a Rust implementation of a modified Robin Hood hash table. Instead of storing the offset from the ideal position, we invert this idea and for each 'home' location, we store the offset to the first of its item.

I believe this will improve search speed but may lead to poorer performance for deleting.

Purpose
-------

The purpose of this repository is two-fold. First, I have been meaning to explore an idea I had for improving the Robin Hood hash table. Second, I wanted to learn a little bit of Rust in a useful setting.

Definitions
-----------

* **Key** : a key is ... (c.f. Python).
* **Value** : a value is ... (c.f. Python).
* **Hashcode** : a hashcode is ...
* **Item** : an item is the collection of key-value-hashcode as well as the offset (see below).

* **Hash Table** : (abbrev "table") This word is overloaded. It can either refer to the whole table data structure or just the table of items.
* **Length** : (abbrev "len") the number of items in the hash table.
* **Capacity** : (abbrev "cap") the number of items that be fit contiguously in the hash table.

* **Home** : this refers to the location that item would ideally be if no other items were present.
* **Offset** : this is the difference between where an item is stored and its home.

Data Structure
--------------

**TODO**

Algorithms
----------

### Insertion

**TODO**

### Search

**TODO**

### Deletion

**TODO**
