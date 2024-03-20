import sys; from pathlib import Path; 
sys.path.append(str(Path(__file__).resolve().parents[2])) 

import N1_vicmil_std_lib as build

builder = build.CppBuilder()

builder.N1_add_compiler_path_arg("g++")
builder.N2_add_cpp_file_arg(build.path_traverse_up(__file__, 0) + "/main.cpp")
exe_file_path = build.path_traverse_up(__file__, 0) + "/a.out"
builder.N9_add_output_file_arg(exe_file_path)

build.delete_file(exe_file_path)
builder.build()

build.change_active_directory(build.path_traverse_up(__file__, 0))
build.run_command("./a.out") 