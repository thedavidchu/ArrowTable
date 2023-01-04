# Arrow Table

## Description and Inspiration

This is a hash table based on the Robin Hood hash table. In my description, I will assume the reader is familiar with the concepts of the Robinhood hash table. 

The key difference is that instead of counting how far an item is away from its ideal location (henceforth called "home"), each home tracks the first element belonging to it.

This means that we can begin searching useful searches no later than the third location access. The access pattern is as follows:

1. Access the home to find the location of the first (we can actually start search as soon as we know this!)
2. Access the location after home to see where the region of interest is guaranteed to end
3. Search the range between the start and end, as indicated by the above

This eliminates the probabilistic "organ pipe search" in the Robinhood hash table (recall that the organ pipe search says that the first square will inhabit a location with a given probability, so we search the most probable location and work our way to the least probable location).

The Arrow Table (named in homage to the Robin Hood hash table, since our 'arrows' point to the starting location, and Robinhood uses a bow and arrow, duh!) inherits many properties from the Robin Hood hash table, such as that all items wanting to occupy a given location (i.e. sharing a home) will be in a contiguous block. We make use of this property.

## Future Work

I would love to benchmark my implementation against the Robinhood hash table.

This would involve optimizing both implementations.
