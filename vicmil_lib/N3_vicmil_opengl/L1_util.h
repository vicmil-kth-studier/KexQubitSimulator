#pragma once
#include "../N2_vicmil_glm/vicmil_glm.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#define GL_GLEXT_PROTOTYPES 1
#include <SDL2/SDL_opengles2.h>

namespace vicmil {
static void GLClearError() {
    //START_TRACE_FUNCTION();
    bool error_detected = false;
    while(GLenum error = glGetError()) {
        std::cout << "unhandled [OpenGl Error] (" << error << ")" << std::endl;
        error_detected = true;
    }
    if(error_detected) {
        Debug("Throw exception");
        throw std::invalid_argument("opengl threw exception!");
    }
    //END_TRACE_FUNCTION();
    return;
}

static void GLCheckError() {
    //START_TRACE_FUNCTION();
    bool error_detected = false;
    while(GLenum error = glGetError()) {
        std::cout << "[OpenGl Error] (" << error << ")" << std::endl;
        error_detected = true;
    }
    if(error_detected) {
        Debug("Throw exception");
        throw std::invalid_argument("opengl threw exception!");
    }
    //END_TRACE_FUNCTION();
}

#define GLCall(x) try{GLClearError(); x; GLCheckError();} catch (const std::exception& e) {ThrowError("Opengl threw error!: " << e.what());}
}