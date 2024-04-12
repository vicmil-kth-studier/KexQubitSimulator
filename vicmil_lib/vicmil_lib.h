#pragma once
#ifdef __EMSCRIPTEN__
#include "N4_vicmil_emscripten/vicmil_emscripten.h"
#else
#include "N3_vicmil_opengl/vicmil_opengl.h"
#endif

void update_window_layout_size(vicmil::__layout__::WindowLayout& layout) {
    #ifdef __EMSCRIPTEN__
    layout.set_size(vicmil::browser::window_width, vicmil::browser::window_height);
    #endif
}