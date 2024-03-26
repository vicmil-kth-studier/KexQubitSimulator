import sys
from pathlib import Path
sys.path.append(str(Path(__file__).resolve().parents[3])) 
sys.path.append(str(Path(__file__).resolve().parents[0])) 

import vicmil_lib.N3_vicmil_opengl as build

builder = build.CppBuilder()

current_path = build.path_traverse_up(__file__, 0)

builder.N1_add_compiler_path_arg("g++")
builder.N2_add_cpp_file_arg(current_path + "/main.cpp")
build.N5_gcc_add_opengl_compiler_settings(builder=builder)
builder.N9_add_output_file_arg("run.out")

build.change_active_directory(current_path)
build.delete_file("run.out")
builder.build()

if(build.file_exist("run.out")):
    build.run_command("./run.out")

