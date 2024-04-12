#define USE_DEBUG
#define DEBUG_KEYWORDS "!vicmil_lib,/source,main(),init()" 
#include "../../vicmil_lib/vicmil_lib.h"

bool init_called = false;
vicmil::general_app_setup::App app = vicmil::general_app_setup::App();
vicmil::__layout__::Widget button_widget;

void render() {
    vicmil::clear_screen();

    // Draw button
    glm::vec4 color = glm::vec4(0.0, 1.0, 0.0, 1.0);
    if(button_widget.is_last_clicked_widget()) {
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
    vicmil::__layout__::LayoutRectReference button_element = 
        vicmil::__layout__::make_element_in_middle_from_weight(app.get_layout_reference(), 1, 1, 1, 1);
    button_widget = app.create_widget(button_element);
}

void update(){
    if(!init_called) {
        init_called = true;
        init();
    }
    update_window_layout_size(app.window_layout); // Handle window layout size update from emscripten if enabled
    PrintExpr(app.window_layout.get_entire_window_reference().get_position().to_string());
    Print("x: " << app.get_user_input_reference().get_mouse_state().x() << 
        "  y: " << app.get_user_input_reference().get_mouse_state().y());
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