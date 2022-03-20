#!/bin/python3
import os
import subprocess
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--run', help='run game', action='store_true')
args = parser.parse_args()

JOBS = str(os.cpu_count())

BASE_DIR = os.path.dirname(os.path.abspath(__file__)) + '/'

ROOT_DIR = BASE_DIR + "../"
OUT_DIR = BASE_DIR + 'out/linux_x86/debug/soldat'
os.makedirs(OUT_DIR, exist_ok = True)

subprocess.check_call(['cmake', ROOT_DIR, "-DCMAKE_BUILD_TYPE=Debug", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"], cwd=OUT_DIR)
subprocess.check_call(['cmake', "--build", ".", '--parallel', JOBS, "--target", "SoldatGame"], cwd=OUT_DIR)

cc_json = ROOT_DIR + "compile_commands.json"
if not os.path.exists(cc_json):
    print("Create", flush = True)

    os.symlink(OUT_DIR + "/compile_commands.json", cc_json)

if args.run is True:
    try:
        subprocess.check_call([OUT_DIR + '/src/client/SoldatClient'])
    except subprocess.CalledProcessError as ex:
        print("")
        print("Run failed with code: %d and output is %s" % (ex.returncode, ex.output))
