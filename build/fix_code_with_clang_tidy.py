#!/usr/bin/python3
import os
import subprocess
import sys
import shutil

# Use clang tidy to automatically fix files

BASE_DIR = os.path.dirname(os.path.abspath(__file__)) + '/'
SRC_DIR = BASE_DIR + "../src"
COMPILE_JSON_DIR = BASE_DIR + "out/soldat_default-clang"
EXPORT_FIXES_DIR = BASE_DIR + "out/clang-tidy-fixes"
if not os.path.exists(os.path.join(COMPILE_JSON_DIR, "compile_commands.json")):
    print(f'Missing {COMPILE_JSON_DIR}/compile_commands.json file')
    sys.exit(-1)

if os.path.exists(EXPORT_FIXES_DIR):
    shutil.rmtree(EXPORT_FIXES_DIR)

os.makedirs(EXPORT_FIXES_DIR)

list_of_files = []

def get_files_and_append_to_list(dir, list):
    for root, dirs, files in os.walk(dir):
        for file in files:
            if file.endswith(".cpp"):
                list_of_files.append(os.path.abspath(os.path.join(root,file)))

get_files_and_append_to_list(SRC_DIR, list_of_files)

args = ['run-clang-tidy', '-p', COMPILE_JSON_DIR, '-j', '8', '-format', '-style', 'file', '-fix', '-export-fixes', EXPORT_FIXES_DIR]
cmd = args + list_of_files
subprocess.call(cmd, cwd=BASE_DIR)
