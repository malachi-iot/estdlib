#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Emit git describe parts in a cmake-friendly format
NOT USED, cmake version.cmake uses regex's git-describe out instead.  Keeping for novelty
"""

#!/usr/bin/env python3

import subprocess
import sys
import re

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

    semver = re.findall(regex, desc)[0]
    #print(semver)

    # Remove leading 'v' only (convention, not semantic)
    if desc.startswith("v"):
        desc = desc[1:]

    core, suffix, word = split_semver(desc)

    # DEBT: Mishmash of regex and non-regex approaches.  Needs cleaning
    print(f"set(GIT_TAG_SEMVER {core})")
    print(f"set(GIT_TAG_SEMVER_MAJOR {semver[0]})")
    print(f"set(GIT_TAG_SEMVER_MINOR {semver[1]})")
    print(f"set(GIT_TAG_SEMVER_PATCH {semver[2]})")
    print(f"set(GIT_TAG_SEMVER_PRERELEASE {suffix})")
    print(f"set(GIT_TAG_SEMVER_IDENTIFIER {word})")
    print(f"set(GIT_DESCRIBED {desc})")


if __name__ == "__main__":
    main()