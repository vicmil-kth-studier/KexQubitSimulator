#define USE_DEBUG
#define DEBUG_KEYWORDS "!vicmil_lib,/source,main(),init()" 
#include "../../source/quantum_computer_include.h"

using namespace vicmil;

const int FPS = 30;

class QuantumCircuitInterface {
    vicmil::WindowLayout window_layout = vicmil::WindowLayout();
    int output_console_window_id;
    int quantum_circuit_window_id;

};

class ApplicationInterace {
public:
    FPSCounter fps_counter = FPSCounter();

    vicmil::WindowLayout window_layout = vicmil::WindowLayout();
    int top_bar_window_id;
    int circuit_interface_window_id;

    vicmil::app::TextButton top_bar = vicmil::app::TextButton();
    ApplicationInterace() {
        // Split window horizontal
        window_layout.set_split_window_horizontal(window_layout.get_instance_id(), false);
        circuit_interface_window_id = window_layout.add_subwindow(window_layout.get_instance_id(), 30, true);
        top_bar_window_id = window_layout.add_subwindow(window_layout.get_instance_id(), 1, true);
    }
    void draw() {
        DisableLogging;
        fps_counter.record_frame();
        Rect window_pos;
        window_layout.get_window_position(top_bar_window_id, &window_pos);
        DebugExpr(window_pos.to_string_min_max());
        top_bar.rect = window_pos;
        top_bar.text = "fps: " + std::to_string(fps_counter.get_fps());
        top_bar.text += "   x: " + std::to_string(vicmil::app::get_mouse_pos_x());
        top_bar.text += "   y: " + std::to_string(vicmil::app::get_mouse_pos_y());
        top_bar.draw();
    }
};

ApplicationInterace app_interface = ApplicationInterace();

void render() {
    clear_screen();
    app_interface.draw();
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
    app_interface = ApplicationInterace();
    app_interface.window_layout.set_window_position(-1, -1, 1, 1);
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

