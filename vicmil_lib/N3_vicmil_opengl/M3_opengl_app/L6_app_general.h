#include "L5_widget.h"

namespace vicmil {
namespace general_app_setup {
class App {
public:
    vicmil::GPUSetup setup;
    vicmil::SDLUserInputManager user_input_manager;
    vicmil::__layout__::WidgetManager widget_manager;
    vicmil::__layout__::WindowLayout window_layout = vicmil::__layout__::WindowLayout();

    std::vector<vicmil::general_gpu_setup::Triangle> triangles = {};
    void draw_add(std::vector<vicmil::general_gpu_setup::Triangle> new_triangles) {
        vicmil::vec_extend(triangles, new_triangles);
    }
    void draw() {
        vicmil::general_gpu_setup::set_triangles(setup, triangles);
        setup.draw();
        triangles = {};
    }
    void show_on_screen() {
        setup.show_on_screen();
    }
    void update() {
        user_input_manager.fetch_events_and_update();
        widget_manager.update(vicmil::get_mouse_input(user_input_manager.get_reference()));
        const std::vector<SDL_Event>& events = user_input_manager.get_reference().get_recent_events();
        if(vicmil::window_resized(events)) {
            int w;
            int h;
            vicmil::get_window_size(setup.window_renderer.window, &w, &h);
            glViewport(0,0,(GLsizei)w,(GLsizei)h);
            window_layout.set_size(w, h);
        }
        #ifdef __EMSCRIPTEN__
        window_layout.set_size(vicmil::browser::window_width, vicmil::browser::window_height);
        #endif
    }
    void init() {
        setup = vicmil::general_gpu_setup::create_gpu_setup();
        user_input_manager = vicmil::SDLUserInputManager::create_user_input_manager();
        widget_manager = vicmil::__layout__::WidgetManager();
        window_layout.set_size(1000, 1000);
        setup.window_renderer.set_to_current_window();
    }
    vicmil::__layout__::LayoutRectReference get_layout_reference() {
        return window_layout.get_entire_window_reference();
    }
    vicmil::SDLUserInputRef get_user_input_reference() {
        return user_input_manager.get_reference();
    }
    vicmil::Rect get_opengl_position(vicmil::RectT<int> pixel_position) {
        return vicmil::get_opengl_position(pixel_position, get_layout_reference().get_position());
    }
    vicmil::__layout__::Widget create_widget(vicmil::__layout__::LayoutRectReference layout_element) {
        vicmil::__layout__::Widget new_widget = vicmil::__layout__::Widget();
        new_widget.set_layout_element(layout_element);
        widget_manager.add_widget(new_widget);
        return new_widget;
    }
};
}
}