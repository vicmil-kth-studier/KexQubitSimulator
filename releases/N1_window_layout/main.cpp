#define USE_DEBUG
#define DEBUG_KEYWORDS "!vicmil_lib,/source,main(),init()" 
#include "../../source/quantum_computer_include.h"

static vicmil::GPUSetup setup;
bool init_called = false;
vicmil::SDLUserInputManager user_input_manager;
vicmil::WidgetUpdater tick_updater;

class DropDownButton: public vicmil::Button {
public:
    vicmil::__layout__::Anchor dropdown_anchor = vicmil::__layout__::Anchor();
    vicmil::__layout__::WindowLayoutElement dropdown;
    void button_pressed_event() override {
        dropdown_anchor.set_size(100, 100);
        dropdown_anchor.update();
    }
    void something_else_pressed_event() override {
        dropdown_anchor.set_size(0, 0);
        dropdown_anchor.update();
    }
};

class TopBar {
public:
    vicmil::__layout__::WindowLayoutElement layout_element;
    vicmil::__layout__::WindowLayoutElement file_settings;
    vicmil::__layout__::WindowLayoutElement edit_settings;
    DropDownButton dropdown_button;
    
    void setup() {
        // Setup layout element
        layout_element.erase_children();
        layout_element.set_height(0, 35 + 31);
        layout_element.set_width(0, -1);
        layout_element.set_vertical_split();

        // Add all window elements
        vicmil::__layout__::WindowLayoutElement window_name = layout_element.create_child_element();
        window_name.set_height(35, 35);

        vicmil::__layout__::WindowLayoutElement settings_bar = layout_element.create_child_element();
        settings_bar.set_height(31, 31);
        settings_bar.set_horizontal_split();

        settings_bar.create_child_element().set_width(5, 5); // Add space element

        file_settings = settings_bar.create_child_element();
        file_settings.set_width(38, 38);

        edit_settings = settings_bar.create_child_element();
        edit_settings.set_width(40, 40);

        dropdown_button.dropdown_anchor = vicmil::__layout__::Anchor(edit_settings);
        dropdown_button.dropdown_anchor.set_attach_left();
        dropdown_button.dropdown_anchor.set_attach_bottom();
        dropdown_button.dropdown = dropdown_button.dropdown_anchor.get_window_layout_element();
        dropdown_button.dropdown.set_width(80, 80);
        dropdown_button.dropdown.set_height(66, 66);

        dropdown_button.set_layout_element(edit_settings.create_child_element());
        dropdown_button.set_user_input_reference(user_input_manager.get_reference());

        dropdown_button.dropdown.set_vertical_split();
        dropdown_button.dropdown.create_child_element().set_height(22, 22);
        dropdown_button.dropdown.create_child_element().set_height(22, 22);
        dropdown_button.dropdown.create_child_element().set_height(22, 22);
    }
};

class Application {
public:
    vicmil::__layout__::WindowLayout layout = vicmil::__layout__::WindowLayout();
    TopBar top_bar = TopBar();
    vicmil::__layout__::WindowLayoutElement entire_window;
    vicmil::__layout__::WindowLayoutElement bottom_bar;

    // Temp variable, resets every draw call
    std::vector<vicmil::general_gpu_setup::Triangle> triangles;
    
    Application() {
        layout.set_size(1000, 1000);
        entire_window = layout.get_window_element();
        entire_window.set_vertical_split();
        top_bar.layout_element = entire_window.create_child_element();
        top_bar.setup();
        bottom_bar = entire_window.create_child_element();
        entire_window.update();
    }
    vicmil::Rect get_opengl_coordinates(vicmil::RectT<int> rect) {
        vicmil::Rect return_rect;
        vicmil::RectT<int> full_window = entire_window.get_position();
        return_rect.w = (2*(double)rect.w) / full_window.w;
        return_rect.h = (2*(double)rect.h) / full_window.h;
        return_rect.x = (2*((double)rect.x) / full_window.w) - 1;
        return_rect.y = (2*((double)rect.y) / full_window.h) - 1;
        return_rect.y = -return_rect.y - return_rect.h;
        return return_rect;
    }
    void draw() {
        user_input_manager.fetch_events_and_update();
        triangles = {};
        triangles = vicmil::visualize_layout_element(entire_window, 0.1, 10);
        vicmil::vec_extend(
            triangles, vicmil::visualize_layout_element(top_bar.dropdown_button.dropdown, 0, 5)
        );
        //PrintExpr(top_bar.dropdown.get_position().to_string());

        // Draw all the triangles!
        vicmil::general_gpu_setup::set_triangles(setup, triangles);
        setup.draw();
    }
};

Application main_app = Application();

void render() {
    DisableLogging;
    START_TRACE_FUNCTION();
    setup.window_renderer.set_to_current_window();
    vicmil::clear_screen();

    main_app.draw();

    setup.show_on_screen();
    END_TRACE_FUNCTION();
}

void init() {
    Debug("Start!");
    setup = vicmil::general_gpu_setup::create_gpu_setup();
    user_input_manager = vicmil::SDLUserInputManager::create_user_input_manager();
}

void update(){
    if(!init_called) {
        init_called = true;
        init();
    }
    user_input_manager.fetch_events_and_update();
    const std::vector<SDL_Event>& events = user_input_manager.get_reference().get_recent_events();
    if(vicmil::window_resized(events)) {
        int w;
        int h;
        vicmil::get_window_size(setup.window_renderer.window, &w, &h);
        main_app.layout.set_size(w, h);
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

