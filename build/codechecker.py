#!/bin/python3
import venv
import os
import subprocess

current_dir = os.path.abspath(os.path.dirname(__file__))
def create_virtual_env(env_dir):
    venv.create(env_dir, with_pip=True)
    print(f"Virtual environment created at {env_dir}")

def install_packages(env_dir, packages):
    pip_executable = os.path.join(env_dir, 'bin', 'pip')
    subprocess.check_call([pip_executable, 'install'] + packages)


if __name__ == "__main__":
    env_dir = os.path.join(current_dir, 'out', 'codechecker', 'venv')
    create_virtual_env(env_dir)
    install_packages(env_dir, ['codechecker'])

