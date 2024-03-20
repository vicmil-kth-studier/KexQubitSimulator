#define USE_DEBUG
#define DEBUG_KEYWORDS "!vicmil_lib,init(),main(),handle_cube_cube_collision" 
#include "../../source/quantum_computer_include.h"

using namespace vicmil;

FPSCounter fps_counter;
vicmil::app::TextConsole text_console;

const int FPS = 30;
int update_count = 0;

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

    vicmil::app::draw2d_text(info_str, -1.0, 1.0, 0.02);

    text_console.draw();
}

// Runs at a fixed framerate
void game_loop() {
    update_count += 1;
    // Do nothing yet!
}

void init() {
    Debug("C++ init!");
    vicmil::app::set_render_func(VoidFuncRef(render));
    vicmil::app::set_game_update_func(VoidFuncRef(game_loop));
    vicmil::app::set_game_updates_per_second(FPS);
    fps_counter = FPSCounter();

    text_console.log("init");


    // lets create a quantum system and do some calculations
    QubitSystem qubit_system = QubitSystem(3);
    qubit_system.hadamar(1);
    qubit_system.phase_shift_pi_over_4(1);
    qubit_system.phase_shift_pi_over_4(1);
    qubit_system.phase_shift_pi_over_4(1);
    qubit_system.phase_shift_pi_over_4(1);
    qubit_system.hadamar(1);

    text_console.log(qubit_system.state_vector_to_str());
    text_console.log(qubit_system.state_vector_to_str_complex());
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

