#define USE_DEBUG
#define DEBUG_KEYWORDS "!vicmil_lib,/source,main(),init()" 
#include "../../source/quantum_computer_include.h"

static vicmil::GPUSetup setup;
bool init_called = false;

class Application {
public:
    vicmil::WindowLayout layout = vicmil::WindowLayout();
    vicmil::WindowLayoutElement entire_window;
        vicmil::WindowLayoutElement top_bar;
        vicmil::WindowLayoutElement bottom_bar;
            //vicmil::WindowLayoutElement right_scroll; // TODO
            //vicmil::WindowLayoutElement bottom_scroll; // TODO

    // Temp variable, resets every draw call
    std::vector<vicmil::general_gpu_setup::Triangle> triangles;
    
    Application() {
        layout.set_size(1000, 1000);
        entire_window = layout.get_window_element();
        entire_window.set_vertical_split();
        top_bar = entire_window.create_child_element();
        top_bar.set_size(100, 100);
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
    void draw_top_bar() {
        vicmil::Rect rect = get_opengl_coordinates(top_bar.get_position());
        vicmil::vec_extend(
            triangles,
            vicmil::general_gpu_setup::triangles_from_2d_color_rect(rect, glm::dvec4(0, 0.5, 0.5, 1.0))
        );
    }
    void draw_bottom_bar() {
        vicmil::Rect rect = get_opengl_coordinates(bottom_bar.get_position());
        vicmil::vec_extend(
            triangles,
            vicmil::general_gpu_setup::triangles_from_2d_color_rect(rect, glm::dvec4(0.3, 0, 0.7, 1.0))
        );
    }
    void draw() {
        triangles = {};
        draw_top_bar();
        draw_bottom_bar();

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

    //std::vector<vicmil::general_gpu_setup::Triangle> triangles = 
    //    vicmil::general_gpu_setup::triangles_from_2d_texture_rect(vicmil::Rect(-0.2, -0.2, 0.4, 0.4), vicmil::Rect(0, 0, 1, 1));

    //Print(triangles[0].to_string());
    //Print(triangles[1].to_string());

    //vicmil::general_gpu_setup::set_triangles(setup, triangles);
}

void update(){
    if(!init_called) {
        init_called = true;
        init();
    }
    std::vector<SDL_Event> events = vicmil::update_SDL();
    for(int i = 0; i < events.size(); i++) {
        SDL_Event event = events[i];
        if(event.type == SDL_WINDOWEVENT &&
            event.window.event == SDL_WINDOWEVENT_RESIZED) {
            Print("Window Resize!");
            int w;
            int h;
            vicmil::get_window_size(setup.window_renderer.window, &w, &h);
            main_app.layout.set_size(w, h);
            PrintExpr(w);
            PrintExpr(h);
            PrintExpr(main_app.top_bar.get_position().w);
            PrintExpr(main_app.entire_window.get_position().w);
            glViewport(0,0,(GLsizei)w,(GLsizei)h);
        }
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

