#define USE_DEBUG
#define DEBUG_KEYWORDS "." 
#include "../../source/quantum_computer_include.h"

using namespace vicmil;

const int FPS = 30;
bool init_called = false;

gpu_setup_barebones::BareBonesGPUSetup* gpu_setup;


void init() {
    START_TRACE_FUNCTION();
    create_vertex_array_object();
    vicmil::init_SDL();
    gpu_setup = new gpu_setup_barebones::BareBonesGPUSetup();
    Debug("gpu_setup->init()");
    gpu_setup->init();
    END_TRACE_FUNCTION();
}
void update() {
    if(init_called == false) {
        init_called = true;
        init();
    }
    vicmil::update_SDL();
    gpu_setup->gpu_setup.window_renderer.set_to_current_window();
    gpu_setup->draw();
    vicmil::clear_screen(glm::vec4(0.0, 0.0, 1.0, 1.0));
    gpu_setup->show_on_screen();
}

// Handle emscripten
void native_app() {
    while(true) {
        update();
    }
}
int main(int argc, char *argv[]) {
    Debug("Main!");
    #ifdef __EMSCRIPTEN__
        std::cout << "Emscripten!" << std::endl;
        emscripten_set_main_loop(update, 0, 1);
    #else
        native_app();
    #endif

    return 0;
};

