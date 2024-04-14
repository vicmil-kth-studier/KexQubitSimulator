#define USE_DEBUG
#define DEBUG_KEYWORDS ".,!_update_side_buffers,!main.cpp,!assign_space" 

#include "../../vicmil_lib/vicmil_lib.h"

bool init_called = false;
vicmil::general_app_setup::App app = vicmil::general_app_setup::App();

vicmil::LayoutRect top_button;
vicmil::LayoutRect top_button_copy;
vicmil::Widget top_button_widget;

vicmil::LayoutRect dropdown;

void setup_top_button() {
    vicmil::__layout__::SizeSplit size_split(app.get_layout_reference());
    size_split.set_split_horizontal();
    size_split.push_back(10, 10); // Add some spacing
    top_button = size_split.push_back(50, 30);
    vicmil::__layout__::Copy copy_ = vicmil::__layout__::Copy(top_button);
    top_button_copy = copy_.create_copy();

    top_button_widget = app.create_widget(copy_.create_copy());
}
void setup_dropdown() {
    vicmil::__layout__::AlignRect dropdown_align = vicmil::__layout__::AlignRect(
        top_button_copy,
        vicmil::RectAlignment::BOTTOM_align_LEFT
    );
    dropdown_align.set_size(200, 400);
    dropdown = dropdown_align.get_aligned_layout();
}

void render() {
    vicmil::clear_screen();

    top_button.lock().update_element->update();

    PrintExpr(app.get_layout_reference().get_position().to_string());
    PrintExpr(top_button_widget.get_position().to_string());
    PrintExpr(top_button_widget.expired());
    // Draw button
    glm::vec4 color = glm::vec4(0.0, 1.0, 0.0, 1.0);
    if(top_button_widget.is_selected()) {
        color = glm::vec4(1.0, 0.6, 0, 1.0);
    }
    if(top_button_widget.is_last_clicked_widget()) {
        color = glm::vec4(1.0, 0.0, 0.0, 1.0);
    }
    vicmil::Rect position = app.get_opengl_position(top_button.get_position());
    app.draw_add(vicmil::general_gpu_setup::triangles_from_2d_color_rect(position, color, 0.01));

    if(!top_button_widget.is_last_clicked_widget()) {
        dropdown = vicmil::LayoutRect();
    }
    if(top_button_widget.was_clicked()) {
        setup_dropdown();
    }

    if(!dropdown.expired()) {
        position = app.get_opengl_position(dropdown.get_position());
        app.draw_add(vicmil::general_gpu_setup::triangles_from_2d_color_rect(position, glm::vec4(0.1, 0.2, 0.9, 1.0), 0.02));
    }

    // Display on screen
    app.draw();
    app.show_on_screen();
}

void init() {
    Print("Init!");
    // Init app
    app.init();

    // Create button
    setup_top_button();
}

void update(){
    if(!init_called) {
        init_called = true;
        init();
    }
    update_window_layout_size(app.window_layout); // Handle window layout size update from emscripten if enabled
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