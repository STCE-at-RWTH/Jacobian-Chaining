#!/usr/bin/env python3

"""Code formatter.

Frontend for clang-format which applies some pre- and post-processing
to handle OpenMP pragmas correctly.
"""

from __future__ import annotations

import os
import re
import sys
import argparse
import glob
from pathlib import Path
import warnings
from subprocess import run, PIPE
from concurrent.futures import ProcessPoolExecutor, as_completed
import signal

from typing import Optional
from types import FrameType

description = """\
Uses clang-format to format a source file. Applies some pre- and
post-processing to handle OpenMP pragmas correctly."""

help = """\
----------------------------------------------------------------------
Example uses:

  format_code.py zgemm.c
    Formats the file in-place.

  format_code.py -o zgemm.c
    Performs out-of-place formatting (creates new file zgemm_formatted.c).

  format_code.py -a
    Formats the entire codebase.

  format_code.py -a -j16
    Formats the entire codebase with 16 parallel jobs.

  format_code.py -a -s
    Formats the entire codebase silently.

"""

# -------------------------------------------------------------------- #
# Command line options
parser = argparse.ArgumentParser(
    formatter_class=argparse.RawDescriptionHelpFormatter,
    description=description,
    epilog=help,
)

parser.add_argument(
    "-o",
    "--outplace",
    action="store_true",
    help="Create new files",
)
parser.add_argument(
    "-a",
    "--all",
    action="store_true",
    help="Format the entire codebase",
)
parser.add_argument(
    "-s",
    "--silent",
    action="store_true",
    help="Suppress verbose output",
)
parser.add_argument(
    "-j",
    "--jobs",
    action="store",
    help="Amount of parallel jobs",
    default="1",
)
parser.add_argument(
    "-n",
    "--dry-run",
    action="store_true",
    help="Just check if files need formatting",
)
parser.add_argument(
    "-Werror",
    "--Werror",
    action="store_true",
    help="Treat warnings as errors.",
)
parser.add_argument("args", nargs="*", action="store", help="Files to process")
opts = parser.parse_args()


# -------------------------------------------------------------------- #


def _format_file(filename: str) -> int:
    """Format a given file and returns the exit code of clang-format.

    Applies pre-processing, calls clang-format and applies some
    post-processing afterwards.

    ### Arguments

    filename : str
        Filename of the input file.

    ### Returns

    The exit code of the clang-format call.
    """
    output_file = filename
    if opts.outplace:
        _path = Path(filename)
        output_file = "{0}_formatted{1}".format(_path.stem, _path.suffix)

    # Preprocess ...
    with open(filename, "r") as source:
        lines = source.readlines()
    with open(output_file, "w") as source:
        for line in lines:
            # ... comment out OpenMP pragmas
            line = re.sub(r"#pragma omp", "//#pragma omp", line)
            # ... comment out multiple case statements
            line = re.sub(r"case (.*?):( +)(.*):", "case \\1: /*\\3:*/", line)
            source.write(line)

    clang_format_call = "clang-format -i --Wno-error=unknown "
    if not opts.silent:
        clang_format_call += "--verbose "
    if opts.dry_run:
        clang_format_call += "--dry-run "
    if opts.Werror:
        clang_format_call += "--Werror "

    # Call clang-format (ignore interupt signals)
    result = run(
        clang_format_call + output_file, shell=True, stdout=PIPE, stderr=PIPE
    )

    # Postprocess ...
    lines = []
    with open(output_file, "r") as source:
        line = source.read().strip("\n")
        # global rexex
        if not opts.dry_run:
            a = re.findall(r"^( *),\n( *)(.*)", line, flags=re.MULTILINE)
            if len(a):
                line = re.sub(
                    r"^( *),\n( *)(.*)", "\\1, \\3", line, flags=re.MULTILINE
                )
        lines = [s + "\n" for s in line.split("\n")]  # split and readd '\n'

    with open(output_file, "w") as source:
        # single line regEx
        for line in lines:
            # ... comment in OpenMP pragmas
            line = re.sub(r"\/\/ *#pragma omp", "#pragma omp", line)
            # ... comment in multiple case statements
            line = re.sub(
                r"case (.*?):( +)\/\*case (.*?):\*\/",
                "case \\1: case \\3:",
                line,
            )
            source.write(line)

    # Atomic write
    sys.stdout.write(result.stderr.decode("utf-8"))

    # Return the exit code of clang-format
    return result.returncode


def init_worker() -> None:
    """Initialize worker thread.

    Disables interrupt signals for workers.
    """
    signal.signal(signal.SIGINT, signal.SIG_IGN)


def sigint_handler(
    executer: ProcessPoolExecutor, signum: int, frame: Optional[FrameType]
) -> None:
    """Handle interrupt signals for worker threads.

    Gracefully shut down (wait for current jobs to finish).

    ### Arguments

    executer : ProcessPoolExecutor
        The process pool.

    signum : int
        Signal number.

    frame : Optional[FrameType]
        Current stack frame.
    """
    executer.shutdown(wait=True, cancel_futures=True)
    sys.exit(signal.SIGINT)


# -------------------------------------------------------------------- #
def main() -> None:
    """Run code formatter."""
    if opts.all:
        repository_root = Path(__file__).parent.parent.parent.resolve()
        src_files = os.path.join(repository_root, "src", "*.cpp")
        headers_1 = os.path.join(repository_root, "include/jcdp", "*.hpp")
        headers_2 = os.path.join(repository_root, "include/jcdp", "*.inl")
        headers_3 = os.path.join(repository_root, "include/jcdp", "*", "*.hpp")
        headers_4 = os.path.join(repository_root, "include/jcdp", "*", "*.inl")
        opts.args = [src_files, headers_1, headers_2, headers_3, headers_4]

    # Check amount of jobs
    cpus = os.cpu_count()
    if cpus and int(opts.jobs) > cpus:
        warnings.warn("The amount of jobs is higher than the cpu core count.")

    # Create process pool (with custom signal handling for python >= 3.7)
    if sys.version_info[1] >= 7:
        executer = ProcessPoolExecutor(int(opts.jobs), initializer=init_worker)
    else:
        executer = ProcessPoolExecutor(int(opts.jobs))
        warnings.warn(
            "Python < 3.7 -> Workers won't ignore interrupt signals! If you "
            "use CTRL+C to cancel the formatting you might end up with "
            "scrambled files."
        )

    if sys.version_info[1] >= 9:
        signal.signal(
            signal.SIGINT, lambda s, f: sigint_handler(executer, s, f)
        )

    # Submit formatting jobs
    future_results = []
    for arg in opts.args:
        for file in glob.glob(arg):
            future_results.append(executer.submit(_format_file, file))

    # Wait for all processes to finish and return exit code
    exit_code = 0
    for future in as_completed(future_results):
        exit_code = future.result() if exit_code == 0 else exit_code

    sys.exit(exit_code)


if __name__ == "__main__":
    main()
