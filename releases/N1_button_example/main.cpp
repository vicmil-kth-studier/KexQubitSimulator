#define USE_DEBUG
#define DEBUG_KEYWORDS ".,!main.cpp" 
#include "../../vicmil_lib/vicmil_lib.h"

bool init_called = false;
vicmil::general_app_setup::App app = vicmil::general_app_setup::App();
vicmil::Widget button_widget;

void render() {
    vicmil::clear_screen();

    // Draw button
    glm::vec4 color = glm::vec4(0.0, 1.0, 0.0, 1.0);
    if(button_widget.lock().is_last_clicked_widget) {
        color = glm::vec4(1.0, 0.0, 0.0, 1.0);
    }
    vicmil::Rect position = app.get_opengl_position(button_widget.get_position());
    app.draw_add(vicmil::general_gpu_setup::triangles_from_2d_color_rect(position, color, 0));

    // Display on screen
    app.draw();
    app.show_on_screen();
}

void init() {
    Print("Init!");
    // Init app
    app.init();

    // Create button
    auto split = vicmil::__layout__::PropSplit(app.get_layout_reference());
    split.set_split_horizontal();
    split.push_back(0.4);
    button_widget = app.create_widget(split.push_back(0.2));
}

void update(){
    if(!init_called) {
        init_called = true;
        init();
    }
    update_window_layout_size(app.window_layout); // Handle window layout size update from emscripten if enabled
    //PrintExpr(app.window_layout.entire_screen_rect.get_position().to_string());
    //Print("x: " << app.get_user_input_reference().get_mouse_state().x() << 
    //    "  y: " << app.get_user_input_reference().get_mouse_state().y());
    app.update();

    render();
};


// Handle emscripten
void native_app() {
    while(true) {
        update();
    }
}
void emscripten_update() {
    update();
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