import sys; from pathlib import Path; 
sys.path.append(str(Path(__file__).resolve().parents[1])) 

from N1_vicmil_std_lib import *

emscripten_compiler_path = path_traverse_up(__file__, 1) + "/dependencies/linux_browser_/emsdk/upstream/emscripten/emcc"

def N5_emscripten_add_opengl_compiler_settings(builder: CppBuilder, exported_functions: List[str] = ["main"], use_assets: bool = False):
    builder.N5_add_compiler_setting("USE_SDL=2")
    builder.N5_add_compiler_setting("USE_SDL_IMAGE=2")

    exported_functions_str = '"'
    for i in range(0, len(exported_functions)):
        if i != 0:
            exported_functions_str += ","
        exported_functions_str += "_" + exported_functions[i]  # Emscripten requires that each function has a underscore in-front
    exported_functions_str += '"'
    builder.N5_add_compiler_setting("EXPORTED_FUNCTIONS=" + exported_functions_str)

    builder.N5_add_compiler_setting("EXTRA_EXPORTED_RUNTIME_METHODS=ccall,cwrap")
    builder.N5_add_compiler_setting("""SDL2_IMAGE_FORMATS='["png"]'""")
    builder.N5_add_compiler_setting("FULL_ES3=1")
    if use_assets:
        builder.add_argument("--preload-file assets")
