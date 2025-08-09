#!/bin/python3
import venv
import os
import subprocess
import argparse
import json
import logging
from datetime import datetime

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
)

logger = logging.getLogger(__name__)

current_dir = os.path.abspath(os.path.dirname(__file__))
def create_virtual_env(env_dir):
    venv.create(env_dir, with_pip=True)
    print(f"Virtual environment created at {env_dir}")

def install_packages(env_dir, packages):
    pip_executable = os.path.join(env_dir, 'bin', 'pip')
    subprocess.check_call([pip_executable, 'install'] + packages)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--install", action='store_true')
    parser.add_argument("--compile_commands", type=str, help="path to compile commands")
    parser.add_argument("--file", type=str, help="use with compile_commands, extracts only this file from compile_commands")
    args = parser.parse_args()
    if args.install:
        env_dir = os.path.join(current_dir, 'out', 'codechecker', 'venv')
        create_virtual_env(env_dir)
        install_packages(env_dir, ['codechecker'])
    elif args.compile_commands:
        new_cc = []
        with open(args.compile_commands, 'r') as f:
            cc = json.load(f)
        for entry in cc:
            if entry["file"].endswith(args.file):
                new_cc.append(entry)
        cc_json = f'{current_dir}/out/codechecker/compile_commands.json'
        with open(f'{current_dir}/out/codechecker/compile_commands.json', 'w') as file:
            json.dump(new_cc, file)
            logger.info(f'Saved files to {cc_json}:')
            for entry in new_cc:
                logger.info(f'  {entry["file"]}')

