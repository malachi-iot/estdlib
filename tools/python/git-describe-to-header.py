#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Emit git describe parts in c++ header file format
Usage:
git-describe-to-header [project_name] [template-file]
"""

#!/usr/bin/env python3

import subprocess
import sys
import re
import git_describe
import argparse

parser = argparse.ArgumentParser(
    description="git-describe C++ header repackager"
)

parser.add_argument('project_name', help="Name of project (#define prefix)");
parser.add_argument('template_file', help="Name of input .h template file");

parser.add_argument(
    "--version",
    help="Version string",
    action="version",
    version="%(prog)s 0.0.0"
)

# 02JUN26 MB - TODO: Beef up argument parsing so we can do things like:
# 1. Specify stdin vs file for input template
# 2. Query version and usage
# 3. Emit other forms of git describe breakdown such as cmake or env flavors
args = parser.parse_args()

from string import Template

#project_name = sys.argv[1]
#infile = sys.argv[2]
project_name = args.project_name
infile = args.template_file

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
    desc = git_describe.run_git_describe()

    semver = re.findall(git_describe.regex, desc)[0]
    #print(semver)

    # Remove leading 'v' only (convention, not semantic)
    if desc.startswith("v"):
        desc = desc[1:]

    core, suffix, word = split_semver(desc)

    # DEBT: Mishmash of regex and non-regex approaches.  Needs cleaning
    #print(f"set(GIT_TAG_SEMVER {core})")
    #print(f"set(GIT_TAG_SEMVER_MAJOR {semver[0]})")
    #print(f"set(GIT_TAG_SEMVER_MINOR {semver[1]})")
    #print(f"set(GIT_TAG_SEMVER_PATCH {semver[2]})")
    #print(f"set(GIT_TAG_SEMVER_PRERELEASE {suffix})")
    #print(f"set(GIT_TAG_SEMVER_IDENTIFIER {word})")
    #print(f"set(GIT_DESCRIBED {desc})")

    #template = Template(open("../cmake/in/git-version.in.h").read())
    template = Template(open(infile).read())

    output = template.substitute(
        # DEBT: Un-hardwire from ESTD
        PROJECT_NAME_UPPER=project_name.upper(),
        GIT_TAG_SEMVER=core,
        GIT_TAG_SEMVER_MAJOR=semver[0],
        GIT_TAG_SEMVER_MINOR=semver[1],
        GIT_TAG_SEMVER_PATCH=semver[2],
        GIT_TAG_SEMVER_PRERELEASE=suffix,
        GIT_TAG_SEMVER_IDENTIFIER=word,
        GIT_DESCRIBED=desc,
        GIT_HASH="abc123",
    )

    print(output)


if __name__ == "__main__":
    main()