"""Parse ThrowTheSwitch Unity test summary lines from a log (stdin).

Usage:
  python3 -m estd.unity.decode < log.txt
"""

# Authored by Grok OSS Bot on behalf of Malachi Burke
from __future__ import annotations

import sys
from typing import Optional, TextIO, TypedDict


class UnitySummary(TypedDict):
    tests: int
    failures: int
    ignored: int


def parse_test_summary(lines) -> Optional[UnitySummary]:
    """Find Unity's '<N> Tests <F> Failures <I> Ignored' line after a ---- separator."""
    saw_separator = False

    for line in lines:
        line = line.strip()

        if line.startswith("----"):
            saw_separator = True
            continue

        if saw_separator:
            parts = line.split()

            # Expect: "<tests> Tests <failures> Failures <ignored> Ignored"
            if (
                len(parts) == 6
                and parts[1] == "Tests"
                and parts[3] == "Failures"
                and parts[5] == "Ignored"
            ):
                try:
                    return {
                        "tests": int(parts[0]),
                        "failures": int(parts[2]),
                        "ignored": int(parts[4]),
                    }
                except ValueError:
                    pass

            # Next line after ---- wasn't the summary; keep scanning
            saw_separator = False

    return None


def report(summary: Optional[UnitySummary], out: TextIO = sys.stdout) -> int:
    """Print one-line status; return process exit code (0 pass, 1 fail)."""
    if summary is None:
        print("Unit tests FAIL: no Unity summary found in input", file=out)
        return 1

    if summary["failures"] > 0:
        print(
            f"Unit tests FAIL: {summary['tests']} ran, {summary['failures']} failed",
            file=out,
        )
        return 1

    print(f"Unit tests OK: {summary['tests']} passed", file=out)
    return 0


def main(argv: Optional[list[str]] = None) -> int:
    # stdin-only for now (matches current CI). argv reserved for future flags.
    _ = argv
    summary = parse_test_summary(sys.stdin)
    return report(summary)


if __name__ == "__main__":
    raise SystemExit(main())
