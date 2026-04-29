#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys

#source = sys.stdin.buffer

#b = source.read()

# 29APR26 MB: tyvm chatgpt

def parse_test_summary(lines):
    saw_separator = False

    for line in lines:
        line = line.strip()

        if line.startswith("----"):
            saw_separator = True
            continue

        if saw_separator:
            parts = line.split()

            # DEBT: Do a namedtuple unless somehow we can do an "anonymous class"
            # Expect: "<tests> Tests <failures> Failures <ignored> Ignored"
            if len(parts) == 6 and parts[1] == "Tests" and parts[3] == "Failures" and parts[5] == "Ignored":
                try:
                    return {
                        "tests": int(parts[0]),
                        "failures": int(parts[2]),
                        "ignored": int(parts[4]),
                    }
                except ValueError:
                    pass  # fall through if integers aren't valid

            # If the next line after "----" isn't the summary, reset
            saw_separator = False

    return None

r = parse_test_summary(sys.stdin)

if r["failures"] > 0 or r == None:
    print(f"Unit tests FAIL: {r["tests"]} ran, {r["failures"]} failed")
    exit(1)

print(f"Unit tests OK: {r["tests"]} passed")
