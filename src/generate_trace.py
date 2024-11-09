import random
from pathlib import Path


def generate_trace(seed: int, max_num_unique: int, length: int) -> list[tuple[str, int, int]]:
    """
    @brief  Generate a trace with item format:
            ("PUT", <key>, <value>)
            or
            ("GET", <key>, <expected-value>)
    """
    prng = random.Random(seed)
    trace = []
    oracle = {}
    unique_value = 0
    for i in range(length):
        op = prng.randint(0, 1)
        key = prng.randint(0, max_num_unique - 1)
        if op == 0:
            op_str = "GET"
            value = oracle.get(key, -1)
        else:
            op_str = "PUT"
            value = unique_value
            oracle[key] = unique_value
            unique_value += 1
        trace.append((op_str, key, value))
    return trace


def write_trace(trace: list[tuple[str, int, int]], path: Path):
    with open(path, "w") as f:
        f.write("\n".join([f"{op_str} {key} {val}" for op_str, key, val in trace]))


def main():
    trace = generate_trace(0, 3, 10)
    write_trace(trace, "trace.txt")


if __name__ == "__main__":
    main()

