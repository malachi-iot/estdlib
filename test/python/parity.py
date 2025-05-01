#!/bin/env python3

from fnv import compute_fnv1a_hash

resource_dir="../catch/resources/"
file_and_hash = [
    ("hello.txt", 0x325d0001),
    ("blob1", 0x774dd18d),
]

bits = 32

for (file, hash) in file_and_hash:
    computed = compute_fnv1a_hash(resource_dir + file, bits)
    matched = computed == hash
    print(f"{computed:0{bits // 4}x}  ", end="")
    if matched == True:
        print("matched", end="")
    else:
        print(f"expected {hash:0{bits // 4}x}", end="")
    print(f"  {file}")
