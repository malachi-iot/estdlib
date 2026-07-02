#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Part of https://github.com/malachi-iot/estdlib and subject to its APACHE license

Emit git describe parts in c++ header file format
Usage:
git-describe-to-header [project_name] [template-file]
"""

script_version = "0.0.0"

import argparse
import datetime as dt
import git
import logging
import re
import semver as sv
import subprocess
import sys

from pathlib import Path
from typing import TextIO

script_dir = Path(__file__).resolve().parent
root_dir = script_dir.parent.parent

parser = argparse.ArgumentParser(
    description="git-describe C++ header repackager"
)

parser.add_argument('project_name', help="Name of project (#define prefix)");
parser.add_argument('template_file',
    # Neat idea, but positional arguments naturally don't want to have defaults
    #default=root_dir / "tools" / "cmake" / "in" / "git-version.in.h",
    help="Name of input .h template file");

parser.add_argument(
    "--version",
    help="Version string",
    action="version",
    version=f"%(prog)s {script_version}"
)

parser.add_argument(
    "-v",
    "--verbose",
    action="count",
    default=0,
    help="Emit extra diagnostic messages (-v, -vv)"
)

parser.add_argument(
    "--timestamps",
    action="store_true",
    help="Include timestamps in log messages"
)

# Not ready yet
parser.add_argument(
    "--cmake-out",
    help="CMake compatible variables file"
)

# Not ready yet
parser.add_argument(
    "--out",
    help="Output to this file rather than stdout"
)

# Not ready yet
parser.add_argument(
    "--soft-fail",
    help="Always return success code, even if we fail."
)

from string import Template

def emit_cmake_helper(core: str, word: str, ostream: TextIO) -> None:
    ostream.write(f'GIT_TAG_SEMVER={core}\n')
    ostream.write(f'GIT_TAG_SEMVER_IDENTIFIER={word}\n')


def configure_logging(verbosity: int, timestamps: bool) -> None:
    levels = [
        logging.WARNING,
        logging.INFO,
        logging.DEBUG,
    ]

    level = levels[min(verbosity, len(levels) - 1)]

    if timestamps:
        log_format = "%(asctime)s %(levelname)s %(message)s"
        date_format = "%Y-%m-%dT%H:%M:%S%z"
    else:
        log_format = "%(levelname)s %(message)s"
        date_format = None

    logging.basicConfig(
        level=level,
        format=log_format,
        datefmt=date_format
    )

def main():
    # 06JUN26 MB - TODO: Beef up argument parsing so we can do things like:
    # 1. Specify stdin vs file for input template
    # 2. Query version and usage
    # 3. Emit other forms of git describe breakdown such as cmake or env flavors
    args = parser.parse_args()

    configure_logging(args.verbose, args.timestamps)

    logging.info("git-describe-to-header v%s", script_version)

    project_name = args.project_name
    infile = args.template_file

    desc = git.describe()

    semver = sv.parse(desc)
    #print(semver)

    # Remove leading 'v' only (convention, not semantic)
    if desc.startswith("v"):
        desc = desc[1:]

    core, suffix, word = sv.split(desc)

    #emit_cmake_helper(core, word, sys.stdout)

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

    now = dt.datetime.now(dt.timezone.utc)

    output = template.substitute(
        # DEBT: Un-hardwire from ESTD
        PROJECT_NAME_UPPER=project_name.upper(),
        TIMESTAMP_ISO8601=now.isoformat(timespec='seconds'),
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