#!/usr/bin/env python3
# make_compile_commands.py
# Usage: python3 make_compile_commands.py --root /abs/path/to/firmware \
#       --src applications_user/my_game --includes build/f7-firmware-D/sdk-headers applications_user/my_game \
#       --cflags "-std=c99 -DFURI_LOG_LEVEL=FURI_LOG_LEVEL_DEBUG"

import os
import json
import argparse
import shlex

def collect_c_files(src_dirs, root):
    files = []
    for d in src_dirs:
        path = os.path.join(root, d)
        for dirpath, _, filenames in os.walk(path):
            for fn in filenames:
                if fn.endswith(".c"):
                    rel = os.path.relpath(os.path.join(dirpath, fn), root)
                    files.append(rel)
    return sorted(files)

def main():
    p = argparse.ArgumentParser()
    p.add_argument("--root", required=True, help="Absolute path to project root")
    p.add_argument("--src", nargs="+", required=True, help="Source directories relative to root")
    p.add_argument("--includes", nargs="*", default=[], help="Include dirs relative to root")
    p.add_argument("--cflags", default="-std=c99", help="Additional compiler flags")
    p.add_argument("--compiler", default="clang", help="Compiler to use in commands")
    p.add_argument("--out", default="compile_commands.json", help="Output file")
    args = p.parse_args()

    root = os.path.abspath(args.root)
    c_files = collect_c_files(args.src, root)
    if not c_files:
        print("No .c files found. Check --src paths.")
        return

    include_flags = []
    for inc in args.includes:
        include_flags.append("-I" + inc)
    base_flags = shlex.split(args.cflags)
    entries = []
    for f in c_files:
        # command should be executed from 'root'
        out_obj = f.replace(".c", ".o")
        cmd_parts = [args.compiler] + include_flags + base_flags + ["-c", f, "-o", out_obj]
        entry = {
            "directory": root,
            "command": " ".join(shlex.quote(x) for x in cmd_parts),
            "file": f
        }
        entries.append(entry)

    out_path = os.path.join(root, args.out)
    with open(out_path, "w") as of:
        json.dump(entries, of, indent=2)
    print("Wrote", out_path, "with", len(entries), "entries")

if __name__ == "__main__":
    main()

