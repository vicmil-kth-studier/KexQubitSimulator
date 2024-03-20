import sys; from pathlib import Path; 
sys.path.append(str(Path(__file__).resolve().parents[2])) 

import N4_vicmil_emscripten as build

builder = build.CppBuilder()

builder.N1_add_compiler_path_arg(build.emscripten_compiler_path)
builder.N2_add_cpp_file_arg(build.path_traverse_up(__file__, 0) + "/main.cpp")
builder.N3_add_optimization_level(optimization_number=2)
build.N5_emscripten_add_opengl_compiler_settings(builder=builder)
builder.N9_add_output_file_arg("run.html")

build.change_active_directory(build.path_traverse_up(__file__, 0))
build.delete_file("run.html")
builder.build()

build.launch_html_page("run.html")