#define USE_DEBUG
#define DEBUG_KEYWORDS "!vicmil_lib,/source,main(),init()" 
#include "../../source/quantum_computer_include.h"

static vicmil::GPUSetup setup;
bool init_called = false;

vicmil::Rect get_opengl_position(vicmil::__layout__::WindowLayoutElement element_) {
    vicmil::RectT<int> full_window = element_._layout_ref.lock()->_layout_elements[0].pixel_position;
    vicmil::RectT<int> rect = element_.get_position();
    vicmil::Rect return_rect;
    return_rect.w = (2*(double)rect.w) / full_window.w;
    return_rect.h = (2*(double)rect.h) / full_window.h;
    return_rect.x = (2*((double)rect.x) / full_window.w) - 1;
    return_rect.y = (2*((double)rect.y) / full_window.h) - 1;
    return_rect.y = -return_rect.y - return_rect.h;
    return return_rect;
}

std::vector<vicmil::general_gpu_setup::Triangle> visualize_layout_element(
    vicmil::__layout__::WindowLayoutElement layout_element,
    double depth = 0,
    int seed = 123) {
    std::vector<vicmil::__layout__::WindowLayoutElement> unparsed_elements = {layout_element};

    vicmil::RandomNumberGenerator rand_gen = vicmil::RandomNumberGenerator();
    rand_gen.set_seed(seed);
    std::vector<vicmil::general_gpu_setup::Triangle> triangles = {};

    while(unparsed_elements.size() > 0) {
        vicmil::__layout__::WindowLayoutElement element_ = unparsed_elements.back();
        unparsed_elements.pop_back();
        std::vector<vicmil::__layout__::WindowLayoutElement> children = element_.get_children();
        if(children.size() > 0) {
            vicmil::vec_extend(unparsed_elements, children);
        }
        else {
            vicmil::Rect rect = get_opengl_position(element_);
            glm::dvec4 color = glm::dvec4(
                rand_gen.rand_between_0_and_1(), 
                rand_gen.rand_between_0_and_1(), 
                rand_gen.rand_between_0_and_1(), 1.0);
            rand_gen.rand();
            rand_gen.rand();
            rand_gen.rand();
            rand_gen.rand();
            vicmil::vec_extend(
                triangles,
                vicmil::general_gpu_setup::triangles_from_2d_color_rect(rect, color, depth)
            );
        }
    }
    return triangles;
}

class TopBar {
public:
    vicmil::__layout__::WindowLayoutElement layout_element;
    vicmil::__layout__::WindowLayoutElement file_settings;
    vicmil::__layout__::WindowLayoutElement edit_settings;
    vicmil::__layout__::Anchor dropdown_anchor = vicmil::__layout__::Anchor();
    vicmil::__layout__::WindowLayoutElement dropdown;

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

        dropdown_anchor = vicmil::__layout__::Anchor(edit_settings);
        dropdown_anchor.set_attach_left();
        dropdown_anchor.set_attach_bottom();
        dropdown = dropdown_anchor.get_window_layout_element();
        dropdown.set_width(80, 80);
        dropdown.set_height(66, 66);

        dropdown.set_vertical_split();
        dropdown.create_child_element().set_height(22, 22);
        dropdown.create_child_element().set_height(22, 22);
        dropdown.create_child_element().set_height(22, 22);
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
        triangles = {};
        triangles = visualize_layout_element(entire_window, 0.1, 10);
        vicmil::vec_extend(
            triangles, visualize_layout_element(top_bar.dropdown, 0, 5)
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
            PrintExpr(main_app.top_bar.layout_element.get_position().w);
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

