"""Implementation of a modified Robin Hood hash table.

## Notes on Sub-Optimality

This is a very naive and sub-optimal implementation. Some of the reasons are as
follows:

1. I use twice the memory overhead to store information on offsets. We only need
to store the start offset information; the ending offset information can be
inferred from the start information of the next element.
2. I have separate data structures to store the elements and offset information,
which would require at least 2 cache misses per access. However, this simplifies
the implementation, since it does not need to carry a garbage value if there is
an element in that space, but no valid offset.
3. It's in Python. Duh. This is really nice for debugging and the ready-to-use
data structures, however.
"""

from typing import List, Tuple, Union


class Node:
    def __init__(self, key, hashcode: int, value):
        self.key = key
        self.hashcode = hashcode
        self.value = value

    def __eq__(self, other):
        """Compares key and hashcode only."""
        return self.hashcode == other.hashcode and self.key == other.key

    def __repr__(self):
        return f"key: {self.key}, hashcode: {self.hashcode}, value: {self.value}"


class Offset:
    """A glorified named tuple."""
    def __init__(self, start: int, end: int):
        self.start = start
        self.end = end

    def __len__(self):
        """This method is also used to determine the __bool__ value."""
        return self.end - self.start

    def __eq__(self, other):
        return self.start == other.start and self.end == other.end

    def __repr__(self):
        return f"({self.start}, {self.end})"


class Table:
    def __init__(self, size: int):
        # This stores the key-hashcode-value tuple.
        self.elements = [None] * size
        # This stores the start and end offset from the "home bucket" to the
        # continous run of elements belonging to this bucket.
        self.offsets = [Offset(0, 0)] * size
        # This is the number of elements that can be stored if we fill it up
        # completely.
        self.size = size
        # This is the number of key-hashcode-value tuples in the elements buffer.
        self.length = 0

    def _debug(self):
        length = sum(node is not None for node in self.elements)
        assert length == self.length

    def _get_index_or_none(self, key, hashcode: int, home_index: int) -> Union[int, None]:
        node = Node(key, hashcode, value=None)
        start, end = self.offsets[home_index].start, self.offsets[home_index].end
        for offset in range(start, end):
            index = (home_index + offset) % self.size
            if self.elements[index] == node:
                return index
        return None

    def _insert(self, key, hashcode: int, home_index: int, value) -> int:
        """Insert and return the offset."""
        # Assumes key is both hashable and has equality defined -- in Rust, this
        # would be a trait!
        node = Node(key, hashcode, value)
        self._debug()
        if not self.offsets[home_index]:
            # NOTE(dchu): the following if condition is unnecessary. The first
            # option is just the 0 case of the for-loop that follows. I have
            # separated them for algorithmic clarity.
            if self.elements[home_index] is None:
                self.elements[home_index] = node
                self.offsets[home_index] = Offset(0, 1)
                self.length += 1
                return 0
            else:
                for offset in range(self.size):
                    index = (home_index + offset) % self.size
                    if self.elements[index] is None:
                        self.elements[index] = node
                        self.offsets[home_index] = Offset(offset, offset + 1)
                        self.length += 1
                        return offset
                    # NOTE(dchu): the following is an optimization and is not
                    # necessary for correctness.
                    elif self.offsets[index]:
                        jump = self.offsets[index].start
                        insertion_index = (index + jump) % self.size
                        replaced_node = self.elements[insertion_index]
                        self.elements[insertion_index] = node
                        self.offsets[home_index] = Offset(offset + jump, offset + jump + 1)
                        self.offsets[index].start += 1
                        self._insert(replaced_node.key, replaced_node.hashcode, index, replaced_node.value)
                        return offset + jump
                raise Exception("no room!")
        else:
            start, end = self.offsets[home_index].start, self.offsets[home_index].end
            for offset in range(start, end):
                index = (home_index + offset) % self.size
                if self.elements[index] == node:
                    self.elements[index] = node
                    return offset
            index = (home_index + end) % self.size
            replaced_node = self.elements[index]
            if replaced_node is None:
                self.elements[index] = node
                self.offsets[home_index].end += 1
                self.length += 1
                return end
            else:
                self.elements[index] = node
                self.offsets[home_index].end += 1
                # NOTE(dchu): the following has poor cache locality. We could
                # process all of the arrows that need to be moved in a single
                # pass (and match the Robin Hood hash table), but this is both
                # more complex and does not allow us to take advantage of the
                # skip forward if the key is found.
                replaced_home_index = replaced_node.hashcode % self.size
                self.offsets[replaced_home_index].start += 1
                self._insert(replaced_node.key, replaced_node.hashcode, replaced_home_index, replaced_node.value)
                return end

    def insert(self, key, value):
        hashcode = hash(key)
        home_index = hashcode % self.size
        self._insert(key, hashcode, home_index, value)

    def search(self, key):
        """If the stored value is None, then we will not be able to tell if it is
        indeed found in the hash table."""
        hashcode = hash(key)
        home_index = hashcode % self.size
        index = self._get_index_or_none(key, hashcode, home_index)
        if index is None:
            return None
        return self.elements[index].value

    def delete(self, key):
        hashcode = hash(key)
        home_index = hashcode % self.size
        index = self._get_index_or_none(key, hashcode, home_index)
        self._debug()
        if index is None:
            return
        # Remove the element since it is present
        if len(self.offsets[home_index]) > 1:
            end_index = (home_index + self.offsets[home_index].end - 1) % self.size
            self.elements[index] = self.elements[end_index]
            self.elements[end_index] = None
        else:
            self.elements[index] = None
        self.offsets[home_index].end -= 1
        for offset in range(1, self.size):
            index = (home_index + offset) % self.size
            # Exit loop if empty element or start offset is 0 for a non-empty bucket
            if self.elements[index] is None or (self.offsets[index].start == 0 and self.offsets[index].end != 0):
                break
            elif self.offsets[index]:
                start_index = (index + self.offsets[index].start - 1) % self.size
                end_index = (index + self.offsets[index].end - 1) % self.size
                self.elements[start_index] = self.elements[end_index]
                self.elements[end_index] = None
                self.offsets[index].start -= 1
                self.offsets[index].end -= 1
        self.length -= 1
        return

    def __repr__(self):
        accum = ["{"]
        for i, (element, offset) in enumerate(zip(self.elements, self.offsets)):
            if element is not None:
                accum.append(f"\t{i}: ({element}, offsets: {offset}),\n")
        accum.append("}")
        return "".join(accum)


################################################################################
### MAIN FUNCTIONS
################################################################################


def test_full_table_without_collisions():
    # Simple test (full table, no collisions)
    t = Table(100)
    for i in range(100):
        t.insert(i, i * 10)
    for i in range(100):
        assert t.search(i) == i * 10


def test_replacement_without_collisions():
    # Test Replacement
    t = Table(100)
    t.insert(1, "A")
    assert t.elements[hash(1) % t.size] == Node(1, hash(1), "A")
    assert t.search(1) == "A"
    assert t.offsets[hash(1) % t.size] == Offset(0, 1)
    t.insert(1, "B")
    assert t.elements[hash(1) % t.size] == Node(1, hash(1), "B")
    assert t.search(1) == "B"
    assert t.offsets[hash(1) % t.size] == Offset(0, 1)

def test_full_table_with_collisions():
    # Test Collision
    t = Table(100)
    for i in range(100):
        t.insert(100 * i + 1, chr(ord("!") + i))
    assert all([t.elements[(1 + i) % t.size] == Node(100 * i + 1, hash(100 * i + 1), chr(ord("!") + i)) for i in range(100)])
    assert all([t.search(100 * i + 1) == chr(ord("!") + i) for i in range(100)])
    assert t.offsets[0] == Offset(0, 0)
    assert t.offsets[1] == Offset(0, 100)
    assert all([t.offsets[i] == Offset(0, 0) for i in range(2, 100)])

def test_full_table_with_replacement_and_collisions():
    # Test Replacement with Collision
    t = Table(100)
    for i in range(100):
        t.insert(100 * i + 1, chr(ord("!") + i))
    assert all([t.search(100 * i + 1) == chr(ord("!") + i) for i in range(100)])
    for i in range(100):
        t.insert(100 * i + 1, 0)
    assert all([t.elements[(1 + i) % t.size] == Node(100 * i + 1, hash(100 * i + 1), 0) for i in range(100)])
    assert all([t.search(100 * i + 1) == 0 for i in range(100)])
    assert t.offsets[0] == Offset(0, 0)
    assert t.offsets[1] == Offset(0, 100)
    assert all([t.offsets[i] == Offset(0, 0) for i in range(2, 100)])

def test_fill_and_empty_without_collisions():
    t = Table(100)
    for i in range(100):
        t.insert(i, "A")
    for i in range(100):
        t.delete(i)
    assert all([t.elements[i] == None for i in range(100)])
    assert t.length == 0

def test_random():
    import random

    random.seed(0)
    
    oracle = {}
    t = Table(100)
    
    for i in range(10000):
        if t.length == 0:
            key = random.randrange(1000)
            t.insert(key, "foo")
            oracle[key] = "foo"
        elif t.length == 100:
            key = t.elements[random.randrange(100)].key
            t.delete(key)
            del oracle[key]
        else:
            switch = random.randrange(2)
            if switch == 0:
                key = random.randrange(1000)
                t.insert(key, "foo")
                oracle[key] = "foo"
            else:
                randint = random.randrange(100)
                node = t.elements[randint]
                for i in range(100):
                    node = t.elements[(randint + i) % t.size]
                    if node is not None:
                        break
                key = node.key
                t.delete(key)
                del oracle[key]
    output = {node.key: node.value for node in t.elements if node is not None}
    assert output == oracle
                

def main():
    test_full_table_without_collisions()
    test_replacement_without_collisions()
    test_full_table_with_collisions()
    test_full_table_with_replacement_and_collisions()
    test_fill_and_empty_without_collisions()
    test_random()


if __name__ == "__main__":
    main()
    t = Table(100)
    t.insert(0, "A")
    t.insert(100, "B")
    t.insert(200, "C")
    t.insert(1, "D")
    t.insert(2, "E")
    t.insert(300, "C2")
