import sys
from pathlib import Path
sys.path.append(str(Path(__file__).resolve().parents[2])) 
sys.path.append(str(Path(__file__).resolve().parents[0])) 

import vicmil_lib.N4_vicmil_emscripten as build

builder = build.CppBuilder()

current_path = build.path_traverse_up(__file__, 0)

builder.N1_add_compiler_path_arg(build.emscripten_compiler_path)
builder.N2_add_cpp_file_arg(current_path + "/main.cpp")
build.N5_emscripten_add_opengl_compiler_settings(builder=builder, exported_functions=["main", "set_screen_size"])
builder.N9_add_output_file_arg("run.html")

build.change_active_directory(current_path)
build.delete_file("run.html")
builder.build()

if(build.file_exist("run.html")):
    build.launch_html_page("fullscreen.html")

