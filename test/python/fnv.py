#!/bin/env python3

import argparse
import fnvhash

def compute_fnv1a_hash(file_path, bits):
    h = 0
    if bits == 32:
        h = 2166136261
        fnv = lambda b, h: fnvhash.fnv1a_32(b, h)
    elif bits == 64:
        h = 14695981039346656037
        fnv = lambda b, h: fnvhash.fnv1a_64(b, h)
    else:
        raise ValueError("Only 32 or 64 bits are supported")

    with open(file_path, 'rb') as f:
        while chunk := f.read(4096):
            h = fnv(chunk, h)

    return h

def main():
    parser = argparse.ArgumentParser(description="Compute FNV-1a hash of a file.")
    parser.add_argument("file", help="Path to the file to hash.")
    parser.add_argument("--bits", type=int, choices=[32, 64], default=32, help="Hash size in bits (default: 32)")

    args = parser.parse_args()
    hash_val = compute_fnv1a_hash(args.file, args.bits)

    print(f"{hash_val:0{args.bits // 4}x}  {args.file}")

if __name__ == "__main__":
    main()
