#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Part of https://github.com/malachi-iot/estdlib and subject to its APACHE license

Emit git describe parts
"""

import logging
import subprocess
import sys

def describe():
    try:
        result = subprocess.run(
            ["git", "describe", "--tags", "--long", "--dirty"],
            capture_output=True,
            text=True,
            check=True,
        )
        return result.stdout.strip()
    except subprocess.CalledProcessError as e:
        print("git describe failed:", file=sys.stderr)
        print(e.stderr or e.stdout, file=sys.stderr)
        sys.exit(1)