"""
You need to have GitPython installed:
pip install GitPython

https://github.com/gitpython-developers/GitPython
"""
import pathlib

from git import Repo  # pip install gitpython

import sys; from pathlib import Path; 
sys.path.append(str(Path(__file__).resolve().parents[2])) 

import N1_vicmil_std_lib as build

def download_emscripten():
    git_url = "https://github.com/emscripten-core/emsdk"
    repo_dir = build.path_traverse_up(__file__, 0) + "/emsdk"
    if(not build.file_exist(repo_dir + "/emsdk")):
        print("Downloading emscripten...")
        Repo.clone_from(git_url, repo_dir)
    else:
        print("emscripten directory already exists!")


def setup_emscripten():
    repo_dir = build.path_traverse_up(__file__, 0) + "/emsdk"
    build.change_active_directory(path=repo_dir)
    build.run_command("./emsdk install latest")
    build.run_command("./emsdk activate latest")


download_emscripten()
setup_emscripten()



