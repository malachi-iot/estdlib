"""
Part of https://github.com/malachi-iot/estdlib and subject to its APACHE license
"""

import re

regex=r"^v([0-9]+)\.([0-9]+)\.([0-9]+)?(-)?([0-9A-Za-z.-]+)"

def parse(desc: str):
    return re.findall(regex, desc)[0]

def split(desc: str):
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

