"""
Part of https://github.com/malachi-iot/estdlib and subject to its APACHE license
"""

import re

# 'v' prefix is not standard semver - you have to strip that yourself
#regex=r"^v([0-9]+)\.([0-9]+)\.([0-9]+)?(-)?([0-9A-Za-z.-]+)"
regex=r"^([0-9]+)\.([0-9]+)\.([0-9]+)?(-)?([0-9A-Za-z.-]+)"

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

    # Incoming desc takes on a few forms, but mainly:
    # 1. #.#.#-prerel-distance-hash-dirty
    # 2. #.#.#-prerel-distance-hash
    # 3. #.#.#-distance-hash-dirty
    # 4. #.#.#-distance-hash

    # We cheat a little and inspect the first token after '-' and if
    # he's numeric, we presume it's distance meaning we're NOT a prerelease

    if len(parts) == 1:
        suffix = ""
        id = (None, 0)
    else:
        suffix = parts[1]
        prerelease = suffix.split('-', 1)
        try:
            # Finding a numeric git distance means we are full release
            # not prerelease
            distance = int(prerelease[0])
            # 28JUL26 MB DEBT: "release" is stopgap, we may not have
            # identifier of consequence when in actual release mode
            id = "rel", "ease"
        except ValueError:
            # No numeric distance means alpha, beta, rc was found
            # (hopefully)
            id = parse_prerelease(prerelease[0])

    return parts[0], id, suffix

