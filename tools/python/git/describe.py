#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Part of https://github.com/malachi-iot/estdlib and subject to its APACHE license

Emit git describe parts
"""

import logging
import subprocess
import sys
from typing import Optional

def describe(abbrev: Optional[int] = None, dirty: bool = False) -> str:
    cmd = ["git", "describe", "--tags", "--long"]

    if abbrev is not None:
        cmd.append(f"--abbrev={abbrev}")

    if dirty:
        cmd.append("--dirty")

    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            check=True,
        )
        return result.stdout.strip()
    except subprocess.CalledProcessError as e:
        print("git describe failed:", file=sys.stderr)
        print(e.stderr or e.stdout, file=sys.stderr)
        sys.exit(1)