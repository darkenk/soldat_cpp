#!/usr/bin/python3
import os
import subprocess
import sys

# Use clang tidy to automatically fix files

BASE_DIR = os.path.dirname(os.path.abspath(__file__)) + '/'
SRC_DIR = BASE_DIR + "../src"
COMPILE_JSON_DIR = BASE_DIR + "out/soldat_default-clang"
if not os.path.exists(os.path.join(COMPILE_JSON_DIR, "compile_commands.json")):
    print(f'Missing {COMPILE_JSON_DIR}/compile_commands.json file')
    sys.exit(-1)

list_of_files = []

def get_files_and_append_to_list(dir, list):
    for root, dirs, files in os.walk(dir):
        for file in files:
            if file.endswith(".cpp"):
                list_of_files.append(os.path.join(root,file))

get_files_and_append_to_list(SRC_DIR, list_of_files)


args = ["clang-tidy", "-p", COMPILE_JSON_DIR, "--format-style=file", "--fix"]

for file in list_of_files:
    cmd = args + [file]
    subprocess.call(cmd, cwd=BASE_DIR)
