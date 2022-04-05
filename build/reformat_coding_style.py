#!/usr/bin/python3
import os
import subprocess

# Reformats files in src and tests folder
# Use to change coding style

BASE_DIR = os.path.dirname(os.path.abspath(__file__)) + '/'
SRC_DIR = BASE_DIR + "../src"
TEST_DIR = BASE_DIR + "../tests"
STYLE_FILE = BASE_DIR + "../.clang-format"

list_of_files = []

def get_files_and_append_to_list(dir, list):
    for root, dirs, files in os.walk(dir):
        for file in files:
            if file.endswith(".hpp") or file.endswith(".cpp"):
                list_of_files.append(os.path.join(root,file))

get_files_and_append_to_list(SRC_DIR, list_of_files)
get_files_and_append_to_list(TEST_DIR, list_of_files)


args = ["clang-format", "-i", "--style=file"]
args.extend(list_of_files)
print
subprocess.call(args, cwd=BASE_DIR)
