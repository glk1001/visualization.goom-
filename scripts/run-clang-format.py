#!/usr/bin/env python

"""A wrapper script around clang-format, suitable for formatting multiple files
and to use for continuous integration.

This is an alternative API for the clang-format command line. It runs over
multiple files and directories in parallel and uses more sensible patterns
in ".clang-format-ignore".

"""

import argparse
import errno
import fnmatch
import io
import multiprocessing
import os
import subprocess
import sys
import traceback
from functools import partial
from subprocess import DEVNULL
from typing import List


CLANG_FORMAT_IGNORE_FILE = ".clang-format-runner-ignore"
CLANG_FORMAT_ENV_VAR = "CLANG_FORMAT"
DEFAULT_EXTENSIONS = "c,h,C,H,cpp,cppm,hpp,cc,hh,c++,h++,cxx,hxx"


def safe_print(msg: str) -> None:
    print(msg, file=sys.stdout)
    sys.stdout.flush()


class ExitStatus:
    SUCCESS = 0
    ERROR = 1


class UnexpectedError(Exception):
    def __init__(self, message: str):
        super(UnexpectedError, self).__init__(message)
        self.formatted_traceback = traceback.format_exc()


def get_excludes_from_ignore_file(ignore_file: str, verbose: bool) -> List[str]:
    ignore_file = os.path.realpath(ignore_file)

    excludes = []
    try:
        with io.open(ignore_file, "r", encoding="utf-8") as f:
            if verbose:
                safe_print(f'Checking ignore file "{ignore_file}".')
            ignore_file_dir = os.path.dirname(ignore_file)
            for line in f:
                if line.startswith("#"):
                    # ignore comments
                    continue
                pattern = line.rstrip()
                if not pattern:
                    # allow empty lines
                    continue
                pattern = os.path.join(ignore_file_dir, pattern)
                if verbose:
                    safe_print(f'Using exclude pattern "{pattern}".')
                excludes.append(pattern)
    except EnvironmentError as e:
        if e.errno != errno.ENOENT:
            raise

    return excludes


def get_all_files_to_format(
    files: List[str],
    recursive: bool = False,
    extensions: List[str] = None,
    verbose: bool = False,
) -> List[str]:
    if extensions is None:
        extensions = []

    if verbose:
        if recursive:
            safe_print("Recursing into directories to get list of files to format.")
        else:
            safe_print("NOT recursing into directories to get list of files to format.")

    excludes = get_excludes_from_ignore_file(CLANG_FORMAT_IGNORE_FILE, verbose)

    out = []
    for file in files:
        if os.path.isfile(file):
            out.append(file)
        elif recursive:
            for dir_path, dir_names, filenames in os.walk(file):
                excludes.extend(
                    get_excludes_from_ignore_file(
                        os.path.join(dir_path, CLANG_FORMAT_IGNORE_FILE), verbose
                    )
                )

                file_paths = [os.path.join(dir_path, f) for f in filenames]

                for pattern in excludes:
                    # os.walk() supports trimming down the dir_names list by
                    # modifying it in-place, to avoid unnecessary directory listings.
                    dir_names[:] = [
                        d
                        for d in dir_names
                        if not fnmatch.fnmatch(os.path.join(dir_path, d), pattern)
                    ]
                    file_paths = [
                        f for f in file_paths if not fnmatch.fnmatch(f, pattern)
                    ]

                for f in file_paths:
                    ext = os.path.splitext(f)[1][1:]
                    if ext in extensions:
                        out.append(f)

    return out


def run_clang_format_wrapper(args, file: str) -> List[str]:
    try:
        return run_clang_format(args, file)
    except Exception as e:
        raise UnexpectedError(f"{file}: {e.__class__.__name__}: {e}")


def run_clang_format(args, file: str) -> List[str]:
    invocation = [args.clang_format_exe, "-i", file]

    if args.style:
        invocation.extend(["--style", args.style])

    if args.dry_run:
        safe_print(" ".join(invocation))
        return []

    encoding_py3 = {"encoding": "utf-8"}

    try:
        proc = subprocess.Popen(
            invocation,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            universal_newlines=True,
            **encoding_py3,
        )
    except OSError as e:
        raise UnexpectedError(
            f"Command '{subprocess.list2cmdline(invocation)}' failed to start: {e}"
        )

    proc_stderr = proc.stderr
    # Hopefully the stderr pipe won't get full and block the process.
    std_errs = list(proc_stderr.readlines())
    proc.wait()

    if proc.returncode:
        raise UnexpectedError(
            f"Command '{subprocess.list2cmdline(invocation)}'"
            f" returned non-zero exit status {proc.returncode}"
        )

    return std_errs


def get_cmd_line_args():
    clang_format_exe = os.environ.get(CLANG_FORMAT_ENV_VAR, "")
    if not clang_format_exe:
        clang_format_exe = "clang-format"

    parser = argparse.ArgumentParser(description=__doc__)

    parser.add_argument("files", metavar="file", nargs="+")

    parser.add_argument(
        "--clang-format-exe",
        help=f"path to the clang-format executable (default: {clang_format_exe})",
        default=clang_format_exe,
    )
    parser.add_argument(
        "--extensions",
        help=f"comma separated list of file extensions (default: {DEFAULT_EXTENSIONS})",
        default=DEFAULT_EXTENSIONS,
    )
    parser.add_argument(
        "-r",
        "--recursive",
        action="store_true",
        help="run recursively over directories",
    )
    parser.add_argument(
        "-d", "--dry-run", action="store_true", help="just print the list of files"
    )
    parser.add_argument(
        "-q",
        "--quiet",
        action="store_true",
        help="disable output, useful when only checking exit code",
    )
    parser.add_argument(
        "--verbose", action="store_true", help="enable additional helpful output"
    )
    parser.add_argument(
        "-j",
        "--num-jobs",
        metavar="N",
        type=int,
        default=0,
        help="run N clang-format jobs in parallel (default: number of cpus + 1)",
    )
    parser.add_argument(
        "--style",
        help="formatting style to apply (LLVM, Google, Chromium, Mozilla, WebKit)",
    )

    return parser.parse_args(), parser.prog


def get_num_jobs(args, num_files: int) -> int:
    num_jobs = args.num_jobs

    if num_jobs == 0:
        num_jobs = multiprocessing.cpu_count() + 1

    return min(num_files, num_jobs)


def check_clang_format_ok(args, prog) -> bool:
    version_invocation = [args.clang_format_exe, str("--version")]
    try:
        subprocess.check_call(version_invocation, stdout=DEVNULL)
    except subprocess.CalledProcessError as e:
        print_error_msg(prog, str(e))
        return False
    except OSError as e:
        print_error_msg(
            prog,
            f"Command '{subprocess.list2cmdline(version_invocation)}' failed to start: {e}",
        )
        return False

    return True


def run_jobs(args, prog, num_jobs: int, files: List[str]) -> int:
    if num_jobs == 1:
        # Execute directly instead of in a pool: less overhead, simpler stacktraces,
        it = (run_clang_format_wrapper(args, file) for file in files)
        pool = None
    else:
        pool = multiprocessing.Pool(num_jobs)
        it = pool.imap_unordered(partial(run_clang_format_wrapper, args), files)
        pool.close()

    while True:
        try:
            errs = next(it)
        except StopIteration:
            break
        except UnexpectedError as e:
            print_error_msg(prog, str(e))
            sys.stderr.write(e.formatted_traceback)
            # Stop at the first unexpected error, since something could be
            # very wrong - don't process all files unnecessarily.
            if pool:
                pool.terminate()
            return ExitStatus.ERROR
        else:
            if errs:
                sys.stderr.writelines(errs)
                return ExitStatus.ERROR

    if pool:
        pool.join()

    return ExitStatus.SUCCESS


def print_error_msg(prog: str, message: str):
    error_text = "error:"
    print(f"{prog}: {error_text} {message}", file=sys.stderr)


if __name__ == "__main__":
    cmd_args, prog_name = get_cmd_line_args()

    if not check_clang_format_ok(cmd_args, prog_name):
        sys.exit(ExitStatus.ERROR)

    files_to_format = get_all_files_to_format(
        cmd_args.files,
        recursive=cmd_args.recursive,
        extensions=cmd_args.extensions.split(","),
        verbose=cmd_args.verbose,
    )
    if not files_to_format:
        if cmd_args.verbose:
            safe_print(f"Nothing to do - exiting.")
        sys.exit(ExitStatus.SUCCESS)

    num_processes = get_num_jobs(cmd_args, len(files_to_format))
    if cmd_args.verbose:
        safe_print(f"Using {num_processes} threads.")

    exit_code = run_jobs(cmd_args, prog_name, num_processes, files_to_format)

    sys.exit(exit_code)
