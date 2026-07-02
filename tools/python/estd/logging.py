#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Part of https://github.com/malachi-iot/estdlib and subject to its APACHE license

Emit git describe parts
"""

import logging

def configure(verbosity: int, timestamps: bool = False) -> None:
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