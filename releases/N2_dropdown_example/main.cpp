#define USE_DEBUG
#define DEBUG_KEYWORDS ".,!_update_side_buffers,!main.cpp,!assign_space" 

#include "../../vicmil_lib/vicmil_lib.h"

bool init_called = false;
vicmil::general_app_setup::App app = vicmil::general_app_setup::App();

vicmil::__layout__::Widget dropdown_widget = vicmil::__layout__::Widget();
vicmil::__layout__::Anchor dropdown_anchor = vicmil::__layout__::Anchor();

void setup_anchor() {
    dropdown_anchor.set_layout_element(dropdown_widget.get_layout_element());
    dropdown_anchor.set_anchor_alignment(vicmil::RectAlignment::BOTTOM_align_LEFT);
    dropdown_anchor.get_anchor_element().get_properties().height.max_size = 100;
    dropdown_anchor.get_anchor_element().get_properties().width.max_size = 100;
    dropdown_anchor.update();
}

void render() {
    vicmil::clear_screen();

    // Draw button
    glm::vec4 color = glm::vec4(0.0, 1.0, 0.0, 1.0);
    if(dropdown_widget.is_selected()) {
        color = glm::vec4(1.0, 0.6, 0, 1.0);
    }
    if(dropdown_widget.is_last_clicked_widget()) {
        color = glm::vec4(1.0, 0.0, 0.0, 1.0);
    }
    vicmil::Rect position = app.get_opengl_position(dropdown_widget.get_position());
    app.draw_add(vicmil::general_gpu_setup::triangles_from_2d_color_rect(position, color, 0.01));

    if(!dropdown_widget.is_last_clicked_widget()) {
        // Remove anchor
        dropdown_anchor = vicmil::__layout__::Anchor();
    }
    if(dropdown_widget.was_clicked()) {
        setup_anchor();
    }

    position = app.get_opengl_position(dropdown_anchor.get_position());
    app.draw_add(vicmil::general_gpu_setup::triangles_from_2d_color_rect(position, glm::vec4(0.1, 0.2, 0.9, 1.0), 0.02));

    // Display on screen
    app.draw();
    app.show_on_screen();
}

void init() {
    Print("Init!");
    // Init app
    app.init();

    // Create button
    Print("Create Top bar");
    app.get_layout_reference().get_properties().split_horizontal = false;
    vicmil::__layout__::LayoutRectReference top_bar = app.get_layout_reference().create_child();
    top_bar.get_properties().height.max_size = 30;
    top_bar.get_properties().split_horizontal = true;

    // Create space element
    top_bar.create_child().get_properties().width.max_size = 30;

    Print("Create Button");
    vicmil::__layout__::LayoutRectReference dropdown_button = top_bar.create_child();
    dropdown_button.get_properties().width.max_size = 60;
    
    Print("Create Widget");
    dropdown_widget = app.create_widget(dropdown_button);
    setup_anchor();

    Print("Update");
    app.get_layout_reference().update();
}

void update(){
    if(!init_called) {
        init_called = true;
        init();
    }
    update_window_layout_size(app.window_layout); // Handle window layout size update from emscripten if enabled
    //PrintExpr(app.window_layout.get_entire_window_reference().get_position().to_string());
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