#define USE_DEBUG
#define DEBUG_KEYWORDS "!vicmil_lib,/source,main(),init()" 
#include "../../source/quantum_computer_include.h"

static vicmil::GPUSetup setup;
bool init_called = false;

class Application {
public:
    vicmil::WindowLayout layout = vicmil::WindowLayout();
    int top_bar_id;
    int bottom_id;
    int left_bottom_id;
    int right_bottom_id;

    // Temp variable, resets every draw call
    std::vector<vicmil::general_gpu_setup::Triangle> triangles;
    
    Application() {
        layout.set_window_position(-1, -1, 1, 1);
        layout.set_split_window_horizontal(false);
        bottom_id = layout.add_subwindow(30, layout.get_instance_id(), true);
            right_bottom_id = layout.add_subwindow(1, bottom_id);
            left_bottom_id = layout.add_subwindow(1, bottom_id);
        top_bar_id = layout.add_subwindow();
    }
    void draw_top_bar() {
        vicmil::Rect rect;
        layout.get_window_position(top_bar_id, &rect);
        vicmil::vec_extend(
            triangles,
            vicmil::general_gpu_setup::triangles_from_2d_color_rect(rect, glm::dvec4(0, 0.5, 0.5, 1.0))
        );
    }
    void draw_bottom_left() {
        vicmil::Rect rect;
        layout.get_window_position(left_bottom_id, &rect);
        vicmil::vec_extend(
            triangles,
            vicmil::general_gpu_setup::triangles_from_2d_color_rect(rect, glm::dvec4(0.3, 0, 0.7, 1.0))
        );
    }
    void draw(double screen_width, double screen_height) {
        triangles = {};
        if(screen_width / screen_height > 0.7) {
            layout.set_split_window_horizontal(true, bottom_id);
        }
        else {
            layout.set_split_window_horizontal(false, bottom_id);
        }

        draw_top_bar();
        draw_bottom_left();

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

    #ifdef __EMSCRIPTEN__
    main_app.draw(vicmil::browser::window_width, vicmil::browser::window_height);
    #else
    main_app.draw(1000, 1000);
    #endif

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
    vicmil::update_SDL();
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

