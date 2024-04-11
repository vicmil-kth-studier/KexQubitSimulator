#include "layout.h"

static MainWindow main_window_layout;
static vicmil::GPUSetup setup;
bool init_called = false;
vicmil::SDLUserInputManager user_input_manager;
vicmil::__layout__::WidgetManager widget_manager;
std::shared_ptr<vicmil::MouseInputWrapper> mouse_input;

class MyButton: public vicmil::__layout__::Widget {
public:
    void update() override {
        if(is_clicked()) {
            Print("Button clicked!");
        }
    }
};
std::shared_ptr<MyButton> my_button;

void create_button() {
    my_button = std::make_shared<MyButton>();
    my_button->set_layout_element(main_window_layout.button_element);
    widget_manager.add_widget(my_button);
}


void render() {
    DisableLogging;
    START_TRACE_FUNCTION();
    setup.window_renderer.set_to_current_window();
    vicmil::clear_screen();

    std::vector<vicmil::general_gpu_setup::Triangle> triangles = {};
    if(my_button->is_last_clicked_widget()) {
        vicmil::Rect position = vicmil::get_opengl_position(main_window_layout.button_element);
        auto triangles2 = vicmil::general_gpu_setup::triangles_from_2d_color_rect(position,
        glm::vec4(1.0, 0.0, 0.0, 1.0), 0);
        vicmil::vec_extend(triangles, triangles2);
    }
    else {
        vicmil::Rect position = vicmil::get_opengl_position(main_window_layout.button_element);
        auto triangles2 = vicmil::general_gpu_setup::triangles_from_2d_color_rect(position,
        glm::vec4(0.0, 1.0, 0.0, 1.0), 0);
        vicmil::vec_extend(triangles, triangles2);
    }
    
    vicmil::general_gpu_setup::set_triangles(setup, triangles);
    setup.draw();

    setup.show_on_screen();
    END_TRACE_FUNCTION();
}

void init() {
    Debug("Start!");
    setup = vicmil::general_gpu_setup::create_gpu_setup();
    user_input_manager = vicmil::SDLUserInputManager::create_user_input_manager();
    main_window_layout = MainWindow();
    mouse_input = std::make_shared<vicmil::MouseInputWrapper>(user_input_manager.get_reference());
    widget_manager = vicmil::__layout__::WidgetManager(mouse_input);
    create_button();
}

void update(){
    if(!init_called) {
        init_called = true;
        init();
    }
    user_input_manager.fetch_events_and_update();
    widget_manager.update();
    const std::vector<SDL_Event>& events = user_input_manager.get_reference().get_recent_events();
    if(vicmil::window_resized(events)) {
        int w;
        int h;
        vicmil::get_window_size(setup.window_renderer.window, &w, &h);
        main_window_layout.layout.set_size(w, h);
        glViewport(0,0,(GLsizei)w,(GLsizei)h);
    }

    #ifdef __EMSCRIPTEN__
    main_app.layout.set_size(vicmil::browser::window_width, vicmil::browser::window_height);
    #endif

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

