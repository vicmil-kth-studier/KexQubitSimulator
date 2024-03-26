#pragma once
#include "../../N2_vicmil_glm/vicmil_glm.h"

// Include all graphing library headers (SDL & OpenGL)
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#define GL_GLEXT_PROTOTYPES 1
#include <SDL2/SDL_opengles2.h>

namespace vicmil {
/**
 * Clear out any OpenGL errors and print out the errors if there was any
 *  Returns -1 if any errors were detected
*/
static int GLClearErrors() {
    bool error_detected = false;
    while(GLenum error = glGetError()) {
        Print("[OpenGl Error] \"" << error << "\"");
        error_detected = true;
    }
    if(error_detected) {
        return -1;
    }
    return 0;
}

/**
 * Detect OpenGL errors both before and after the expression was executed
*/
#define GLCall(x) \
if(GLClearErrors() == -1) {ThrowError("Unhandled opengl error before call!");} \
x; \
if(GLClearErrors() == -1) {ThrowError("Opengl call caused error!");}
}