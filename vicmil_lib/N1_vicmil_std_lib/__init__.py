"""
This is a file with some help functions to build a cpp project,
Like a make file but with the power of python!
"""

# Add directory to path
"""
import sys; from pathlib import Path; 
sys.path.append(str(Path(__file__).resolve().parents[0])) 
"""

import sys
import subprocess
import pathlib
from pathlib import Path; 
from typing import List, Dict
import os
import webbrowser

class CppBuilder:
    """
    A class to help build c++ code projects

    When builing c++ projects, you generally want to add the arguments in the following order,
        this is not set in stone how    git_url = "https://github.com/emscripten-core/emsdk"
    parents = pathlib.Path(__file__).parents
    path_raw = str(parents[0].resolve())
    dir_path = path_raw.replace("\\", "/")
    repo_dir = dir_path + "/emsdk"
    Repo.clone_from(git_url, repo_dir)ever

    1: compiler path
        this could for example be gcc
    2: cpp files
        List all cpp files you plan to use in the project

    3: [OPTIONAL] optimization level
        this could for example be "-O2" or "-O3"
    4: [OPTIONAL] macros
        this could for example be flags or other macros you cant to define
    5: [OPTIONAL] include paths
        These are additional paths where your code will look for .h files
    6: [OPTIONAL] lib paths
        These are additional paths where your code will look for .lib files
    7: [OPTIONAL] lib files
        These are the .lib files that will be included in the project
    8: [OPTIONAL] output file:
        This could for example be run.exe, this will be the compiled result

    ...

    Attributes
    ----------
    args: List[str]
        All arguments to use in the compilation

    Methods
    -------
    build()
        builds a c++ project based on the arguments stored in the class
    
    """

    def __init__(self):
        self.args: List[str] = list()


    def build(self) -> None:
        """Build the cpp files with the arguments provided to the class

        Parameters
        ----------

        Returns
        -------
            None
        """
        command = ""
        for arg_ in self.args:
            command += " " + arg_

        run_command(command=command)

    def add_argument(self, arg: str):
        self.args.append(arg)

    def N1_add_compiler_path_arg(self, file_path: str):
        self.args.append(file_path)

    def N2_add_cpp_file_arg(self, file_path: str):
        self.args.append(file_path)

    def N3_add_optimization_level(self, optimization_number: int): # Typically 2 or 3
        self.args.append("-O" + str(optimization_number))

    def N4_add_macro(self, macro: str):
        self.args.append("-D " + macro)


    def N5_add_compiler_setting(self, filename: str):
        self.args.append("-s " + filename) # Supported by some compilers, e.g. emscripten

    def N6_add_include_path(self, path: str):
        self.args.append("-I " + path)

    def N7_add_lib_path(self, path: str):
        self.args.append("-L " + path)

    def N8_add_library_file(self, filename: str):
        self.args.append("-l " + filename)

    def N9_add_output_file_arg(self, file_path: str):
        self.args.append("-o " + file_path)


def N4_enable_debug_on_keyword(builder: CppBuilder, keywords: List[str] = ["."]):
    """
    Only enable debug messages that contain one of the keywords, the keyword can refer to a file or a function
    """
    debug_keyword_str = ""
    debug_keyword_str += "'"
    debug_keyword_str += '"'
    for i in range(0, len(keywords)):
        if i != 0:
            debug_keyword_str += ","
        debug_keyword_str += keywords[i]
    debug_keyword_str += '"'
    debug_keyword_str += "'"

    builder.N4_add_macro("USE_DEBUG")
    builder.N4_add_macro("DEBUG_KEYWORDS=" + debug_keyword_str)


def path_traverse_up(path: str, count: int) -> str:
    """Traverse the provided path upwards

    Parameters
    ----------
        path (str): The path to start from, tips: use __file__ to get path of the current file
        count (int): The number of directories to go upwards

    Returns
    -------
        str: The path after the traversal, eg "/some/file/path"
    """

    parents = pathlib.Path(path).parents
    path_raw = str(parents[count].resolve())
    return path_raw.replace("\\", "/")


def move_file(src_path: str, dst_path: str) -> None:
    """Move a file from source to destination

    Parameters
    ----------
        src_path (str): Where to copy the file from
        dst_path (str): Where the file should be copied to

    Returns
    -------
        None
    """
    pass


def delete_file(file_path: str) -> None:
    """Delete a file from the computer

    Parameters
    ----------
        file_path (str): The path to the file to delete

    Returns
    -------
        None
    """
    try:
        run_command("rm " + file_path)
    except Exception as e:
        pass


def file_exist(file_path: str) -> bool:
    """Determine if a file exists on the computer

    Parameters
    ----------
        file_path (str): The path to the file

    Returns
    -------
        bool: returns true if the file exist, otherwise false
    """
    return os.path.exists(file_path)


def change_active_directory(path: str) -> None:
    """Change the current active directory

    Parameters
    ----------
        path (str): The path to the new active directory

    Returns
    -------
        None
    """
    os.chdir(path)


def run_command(command: str, get_result_as_str: bool = False) -> None:
    """Run a command in the terminal

    Parameters
    ----------
        command (str): The command to run
    get_result_as_str:
        if false: the output of the command is printed into the terminal in real time but is not returned
        if true: the output of the command is returned as a string when the command completes

    Returns
    -------
        None
    """
    if get_result_as_str:
        command_split: List[str] = command.split(" ")
        command_split = filter(lambda a: a != "", command_split)

        print(command)
        result = subprocess.check_output(command_split)
        result = str(result)
        result = result.replace("\\n", "\n")
        print(result)
        return result
    else:
        print(command)
        subprocess.call(command, shell=True)


def get_platform() -> str:
    """Gets what platform the system is running on

    Parameters
    ----------

    Returns
    -------
        str: The platform. The return values are "windows", "linux", "mac" or "unknown"
    """

    if sys.platform == "win32":
        return "windows"
    
    if sys.platform == "cygwin":
        return "windows"
    
    if sys.platform == "darwin":
        return "mac"
    
    if sys.platform == "linux":
        return "linux"
    
    return "unknown"


def launch_html_page(html_file_path: str):
    """ Start the webbrowser if not already open and launch the html page

    Parameters
    ----------
        html_file_path (str): The path to the html file that should be shown in the browser

    Returns
    -------
        None
    """
    change_active_directory(path_traverse_up(html_file_path, count=0))
    if not (file_exist(html_file_path)):
        print("html file does not exist!")
        return
    
    file_name: str = html_file_path.replace("\\", "/").rsplit("/", maxsplit=1)[-1]
    webbrowser.open("http://localhost:8000/" + file_name, new=0, autoraise=True)
    run_command("python3 -m http.server")

