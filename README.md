ArrowTable - A Modified Robin Hood Hash Table
=============================================

This repository contains a C implementation (and Python prototype) of a
modified Robin Hood hash table. Instead of storing the offset from the ideal
position, we invert this idea and for each 'home' location, we store the offset
to the first of its item.

I believe this will improve search speed but may lead to poorer performance for
deletion.

Purpose
-------

This repository explores an idea I had for improving the Robin Hood hash table.

Naming
------

The key difference between this implementation and the original Robin Hood hash
table is where the latter tracks the "wealth" of squares (i.e. how far an item
is away from its "home" square), this one's "home" square tracks its items with
an _arrow_-like pointer. Also, Robin Hood uses a bow and arrow, so of course
the name pays homage to the implementation.

Running
-------

To run this program, first navigate to the `src` directory, but running the
following in any directory within this repository:

```bash
#!/usr/bin/bash

cd "$(git rev-parse --show-toplevel)/src"
```

To generate the trace that the executable will use, run:

```bash
#!/usr/bin/bash

make trace
```

To compile the program, run:

```bash
#!/usr/bin/bash

# Compile the program
make build
```

To run the program, run:

```bash
#!/usr/bin/bash

# Run the program
./a.out trace.txt
```

To clean up the results, run the following:

```bash
#!/usr/bin/bash

make clean
```


Potential Optimizations
-----------------------

This is a list of potential optimizations. Not every "optimization" is a good
idea. Each has its own tradeoffs.

* Python compact dict
  * Store the hash (or some part of the hash) here so we can skip ones that
  definitely don't match (use part not required to compute "home" if using a
  subset, because we already clusted with the bits used to compute the "home")
  * Store the arrow in the lookup table. This increases the amount of memory we
  need to load from cache, but also if we don't get an immediate hit, then this
  is no good.
  * Change the size of the **primary index** for the lookup. Smaller tables need
  fewer indices.
  * Figure out amortized deletion for the dense item lookup. How will we delete
  items? Rust does not like dangling pointers, so this might be a bit _unsafe_
  (i.e. I'll have to learn unsafe Rust).

References
----------

This data structure is inspired by the Robin Hood Hash Table. Structurally, the
Robin Hood Hash Table and this are almost identical except for the "wealth"
versus the "arrow".

License
-------

This software currently does not have a license.

