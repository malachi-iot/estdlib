#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Emit git describe parts in a cmake-friendly format
NOT USED, cmake version.cmake uses regex's git-describe out instead.  Keeping for novelty
"""

#!/usr/bin/env python3

import subprocess
import sys


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


def split_semver(desc: str):
    # Find first "-" after semantic version prefix
    # We assume SemVer core is always x.y.z
    parts = desc.split("-", 1)

    if len(parts) == 1:
        suffix = ""
        word = ""
    else:
        suffix = parts[1]
        word = suffix.split("-", 1)[0]

    return parts[0], suffix, word


def main():
    desc = run_git_describe()

    # Remove leading 'v' only (convention, not semantic)
    if desc.startswith("v"):
        desc = desc[1:]

    core, suffix, word = split_semver(desc)

    print(f"set(GIT_TAG_SEMVER {core})")
    print(f"set(GIT_TAG_SEMVER_PRERELEASE {suffix})")
    print(f"set(GIT_TAG_SEMVER_IDENTIFIER {word})")


if __name__ == "__main__":
    main()