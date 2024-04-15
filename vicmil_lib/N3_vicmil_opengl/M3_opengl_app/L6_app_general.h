#include "L5_widget.h"

namespace vicmil {
vicmil::WidgetManager::MouseInput get_mouse_input(vicmil::SDLUserInputRef user_input) {
    vicmil::WidgetManager::MouseInput mouse_input = vicmil::WidgetManager::MouseInput();
    mouse_input.was_clicked = mouse_left_clicked(user_input.get_recent_events());
    mouse_input.x = user_input.get_mouse_state().x();
    mouse_input.y = user_input.get_mouse_state().y();
    return mouse_input;
}

namespace general_app_setup {
class App {
public:
    vicmil::GPUSetup setup;
    vicmil::SDLUserInputManager user_input_manager;
    vicmil::SDLUserInputRef user_input;
    vicmil::WindowSize window_size;
    vicmil::WidgetManager widget_manager = vicmil::WidgetManager();
    vicmil::LayoutRectManager window_layout = vicmil::LayoutRectManager();

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
    // One some systems, the viewport may not always be the entire screen. This function can account for that
    void translate_position_to_viewport(int& mouse_x, int& mouse_y) {
        vicmil::translate_position_to_viewport(
            mouse_x, 
            mouse_y, 
            window_layout.entire_screen_rect.get_position(), 
            RectT<int>(0, 0, 
                window_size.w, 
                window_size.h
            )
        );
    }
    void update() {
        user_input_manager.fetch_events_and_update();
        window_size.update();
        auto mouse_ = vicmil::get_mouse_input(user_input);
        #ifndef __EMSCRIPTEN__
        const std::vector<SDL_Event>& events = user_input.get_recent_events();
        if(vicmil::window_resized(events)) {
            glViewport(
                0,
                0,
                (GLsizei)window_size.w,
                (GLsizei)window_size.h
            );
            window_layout.set_screen_size(
                window_size.w, 
                window_size.h
            );
        }
        #else
            translate_position_to_viewport(mouse_.x, mouse_.y);
        #endif
        widget_manager.update(mouse_);
    }
    void init() {
        setup = vicmil::general_gpu_setup::create_gpu_setup();
        user_input_manager = vicmil::SDLUserInputManager();
        user_input = user_input_manager.get_reference();
        widget_manager = vicmil::WidgetManager();
        window_layout.set_screen_size(1024, 1024);
        setup.window_renderer.set_to_current_window();
        window_size = vicmil::WindowSize(setup.window_renderer.window);
    }
    vicmil::LayoutRect get_layout_reference() {
        return window_layout.entire_screen_rect;
    }
    vicmil::SDLUserInputRef get_user_input_reference() {
        return user_input_manager.get_reference();
    }
    vicmil::Rect get_opengl_position(vicmil::RectT<int> pixel_position) {
        return vicmil::get_opengl_position(pixel_position, get_layout_reference().get_position());
    }
    vicmil::Widget create_widget(vicmil::LayoutRect layout_element) {
        vicmil::__layout__::WidgetRect widget_rect = vicmil::__layout__::WidgetRect(widget_manager, layout_element);
        return widget_rect.widget();
    }
    void visualize_widgets() {
        auto it = widget_manager.widgets.begin();
        int color_index = 0;
        while(it != widget_manager.widgets.end()) {
            vicmil::ColorRGB888 color_ = vicmil::ColorRGB888(color_index);
            glm::vec4 color = glm::vec4({color_.r/255.0, color_.g/255.0, color_.b/255.0, 1.0});
            color_index += 1;
            vicmil::Rect position = get_opengl_position(it.lock().position);
            double depth = it.lock().depth;
            draw_add(vicmil::general_gpu_setup::triangles_from_2d_color_rect(position, color, depth));
            it.operator++();
        }
    }
};
}
}