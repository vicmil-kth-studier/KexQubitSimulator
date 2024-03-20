import sys
from pathlib import Path
sys.path.append(str(Path(__file__).resolve().parents[0])) 

import vicmil_lib.N4_vicmil_emscripten as build

builder = build.CppBuilder()

current_path = build.path_traverse_up(__file__, 0)
build.change_active_directory(build.path_traverse_up(__file__, 0))


html_file = current_path + "/index.html"

build.launch_html_page(html_file)

