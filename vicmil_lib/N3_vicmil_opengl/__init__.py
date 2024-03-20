import sys; from pathlib import Path; 
sys.path.append(str(Path(__file__).resolve().parents[1])) 

from N1_vicmil_std_lib import *

"""
Add compiler settings when compiling with gcc(native platform)
"""
def N5_gcc_add_opengl_compiler_settings(builder: CppBuilder):
    builder.add_argument("-w -lSDL2")
    builder.add_argument("-w -lGL")  #(Used for OpenGL on desktops)

    # builder.add_argument("-w -lGLESv2")  # (OpenGL ES, subet of OPENGL that can also run in browser)
    # builder.add_argument("-lEGL")
