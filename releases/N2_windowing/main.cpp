#define USE_DEBUG
#define DEBUG_KEYWORDS "!vicmil_lib,/source,main(),init()" 
#include "../../source/quantum_computer_include.h"

using namespace vicmil;

FPSCounter fps_counter;
const int FPS = 30;

vicmil::WindowLayout window_layout = vicmil::WindowLayout();

void render() {
    clear_screen();

    // Update camera
    vicmil::app::globals::main_app->camera.screen_aspect_ratio = 
        vicmil::app::globals::main_app->graphics_setup.get_window_aspect_ratio();

    fps_counter.record_frame();
    double fps = fps_counter.get_fps();
    std::string info_str = "fps: " + std::to_string(fps);

    MouseState mouse_state = MouseState();
    info_str += "   x: " + std::to_string(vicmil::x_pixel_to_opengl(mouse_state.x(), vicmil::app::globals::main_app->graphics_setup.width));
    info_str += "   y: " + std::to_string(vicmil::y_pixel_to_opengl(mouse_state.y(), vicmil::app::globals::main_app->graphics_setup.height));

    vicmil::app::draw2d_text(info_str, 0.0, 1.0, 0.02);
}

// Runs at a fixed framerate
void game_loop() {
    // Do nothing yet!
}

void init() {
    Debug("C++ init!");
    vicmil::app::set_render_func(VoidFuncRef(render));
    vicmil::app::set_game_update_func(VoidFuncRef(game_loop));
    vicmil::app::set_game_updates_per_second(FPS);
    fps_counter = FPSCounter();

    std::cout << vicmil::type_to_str<MouseState>() << std::endl;
    AnyTypeInt my_int = AnyTypeInt();
    my_int.int_ = 123;
    AnyType* my_int_ptr = &my_int;
    

    AnyTypeInt* ptr1 = my_int_ptr->try_cast<AnyTypeInt>();
    if(ptr1 == nullptr) {
        std::cout << "ptr1 was null" << std::endl;
    }
    else {
        std::cout << "ptr1 was not null" << std::endl;
        std::cout << ptr1->int_ << std::endl;
    }
    AnyTypeString* ptr2 = my_int_ptr->try_cast<AnyTypeString>();
    if(ptr2 == nullptr) {
        std::cout << "ptr2 was null" << std::endl;
    }
    else {
        std::cout << "ptr2 was not null" << std::endl;
    }
}


// Handle emscripten
void native_app() {
    while(true) {
        vicmil::app::app_loop_handler(vicmil::VoidFuncRef(init));
    }
}
void emscripten_update() {
    vicmil::app::app_loop_handler(vicmil::VoidFuncRef(init));
}
int main(int argc, char *argv[]) {
    Debug("Main!");
    #ifdef __EMSCRIPTEN__
        std::cout << "Emscripten!" << std::endl;
        emscripten_set_main_loop(emscripten_update, 0, 1);
    #else
        native_app();
    #endif

    return 0;
};

