"""
Part of https://github.com/malachi-iot/estdlib and subject to its APACHE license
"""

import re

regex=r"^v([0-9]+)\.([0-9]+)\.([0-9]+)?(-)?([0-9A-Za-z.-]+)"

def parse(desc: str):
    return re.findall(regex, desc)[0]

# Given a word# combo, say alpha5, parse it into alpha, 5
def parse_prerelease(s: str) -> tuple[str, int]:
    m = re.match(r"^([a-zA-Z]+)(\d*)$", s)
    # TODO: Bring this back with a kind of strict mode where perhaps caller passes
    # in permissible alpha, beta, rc, etc.
    #m = re.match(r"^(alpha|beta|rc)(\d*)$", s)
    if not m:
        raise ValueError(f"Invalid prerelease format: {s}")

    release = m.group(1)
    num_str = m.group(2)

    num = int(num_str) if num_str else 0
    return release, num

# Given a git-described SemVer, split out:
# 1. SemVer itself sans prerelease
# 2. prelease portion, split by alpha/beta/rc
# 3. git-describe suffix (distance marker with hash, INCLUDES prerelease portion)
def split(desc: str):
    # Find first "-" after semantic version prefix
    # We assume SemVer core is always x.y.z
    parts = desc.split("-", 1)

    if len(parts) == 1:
        suffix = ""
        id = (None, 0)
    else:
        suffix = parts[1]
        id = parse_prerelease(suffix.split("-", 1)[0])

    return parts[0], id, suffix

