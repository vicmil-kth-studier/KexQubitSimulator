Here are all the external libraries, compilers etc that are necessary to build and run the project

All the extra dependencies in the project can be found below. For each dependency, there exists a link to the original website from where you can download the material

Just put the files in the appropriate folder



This project has the following dependencies

    emscripten:
        For Compiling c++ so it can run in the web
        Note that the emscripten compiler requires linux to run propperly. There 
        is a windows version as well but I have not been able to get it working due 
        to a number of compiler errors.

        Path:
            dependencies/linux_/emsdk/

        Links:
            https://emscripten.org/docs/getting_started/downloads.html


    gcc:
        For compiling c++ so it can run locally on the computer.

    OpenGL:
        Graphics library to access low level graphics. 

        Version:
            Requires OpenGL 1.0 (a very old version). Emscripten does not support 
                higher versions that I have found

        Links:
            Should already work if you have emscripten.
            To run locally, just download the necessary files and link into the project

    SDL2:
        Needed to host a window where opengl grahics can be put, also contains 
            functionality for mouse input, keyboard input, loading images etc.

        Links:
            Should already work if you have emscripten.
            To run locally, just download the necessary .lib file and link into the project

