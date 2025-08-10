#!/usr/bin/python3
import os
import subprocess
import sys
import shutil
import argparse

# Use clang tidy to automatically fix files

BASE_DIR = os.path.dirname(os.path.abspath(__file__)) + '/'
SRC_DIR = BASE_DIR + "../src"
COMPILE_JSON_DIR = BASE_DIR + "out/soldat_default-clang"
EXPORT_FIXES_DIR = BASE_DIR + "out/clang-tidy-fixes"

def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description='Run clang-tidy to automatically fix files')
    parser.add_argument('--plugin', help='run clang-tidy')
    parser.add_argument('--export-fixes', help='export fixes', action='store_true')
    parser.add_argument('--only-src', help='fixes only code in src directory', action='store_true')
    parser.add_argument('--checks', help='checks to apply')
    args = parser.parse_args(argv)

    if not os.path.exists(os.path.join(COMPILE_JSON_DIR, "compile_commands.json")):
        print(f'Missing {COMPILE_JSON_DIR}/compile_commands.json file')
        sys.exit(-1)

    clang_tidy_args = ['run-clang-tidy', '-p', COMPILE_JSON_DIR, '-header-filter=.*', '-format', '-style', 'file']
    if args.export_fixes:
        if os.path.exists(EXPORT_FIXES_DIR):
            shutil.rmtree(EXPORT_FIXES_DIR)
        os.makedirs(EXPORT_FIXES_DIR)
        clang_tidy_args.append('-export-fixes')
        clang_tidy_args.append(EXPORT_FIXES_DIR)
    else:
        clang_tidy_args.append('-fix')

    if args.only_src:
        list_of_files = []

        def get_files_and_append_to_list(dir, list):
            for root, dirs, files in os.walk(dir):
                for file in files:
                    if file.endswith(".cpp"):
                        list_of_files.append(os.path.abspath(os.path.join(root,file)))

        get_files_and_append_to_list(SRC_DIR, list_of_files)
        clang_tidy_args = clang_tidy_args + list_of_files

    if args.checks:
        clang_tidy_args.append('-checks=-*,' + args.checks)
    if args.plugin:
        clang_tidy_args.append('-load')
        clang_tidy_args.append(os.path.abspath(args.plugin))
    print(clang_tidy_args)
    #clang
    subprocess.call(clang_tidy_args, cwd=BASE_DIR)
    if args.export_fixes:
        print("clang-apply-replacements")
        subprocess.call(['clang-apply-replacements', EXPORT_FIXES_DIR])
    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
