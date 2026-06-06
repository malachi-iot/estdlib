#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Emit git describe parts
"""

import logging
import subprocess
import sys

regex=r"^v([0-9]+)\.([0-9]+)\.([0-9]+)?(-)?([0-9A-Za-z.-]+)"

def run_git_describe():
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