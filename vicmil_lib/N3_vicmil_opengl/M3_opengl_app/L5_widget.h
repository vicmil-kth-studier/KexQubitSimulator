#include "L4_gpu_setup_general.h"
#include <list>

namespace vicmil {

std::vector<vicmil::general_gpu_setup::Triangle> visualize_layout_element(
    vicmil::__layout__::LayoutRectReference layout_element,
    double depth = 0,
    int seed = 123) {
    std::vector<int> unparsed_elements = {layout_element.index};

    vicmil::RandomNumberGenerator rand_gen = vicmil::RandomNumberGenerator();
    rand_gen.set_seed(seed);
    std::vector<vicmil::general_gpu_setup::Triangle> triangles = {};

    while(unparsed_elements.size() > 0) {
        vicmil::__layout__::LayoutRectReference element_ = layout_element.get_reference(unparsed_elements.back());
        unparsed_elements.pop_back();
        const std::vector<int>& children = element_.get_properties().children_indecies;
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

/**
 * Determine if the mouse left button has been pressed:
 * Read here for more documentation: 
 * https://www.libsdl.org/release/SDL-1.2.15/docs/html/sdlmousebuttonevent.html
*/
bool mouse_left_clicked(const std::vector<SDL_Event>& events) {
    auto it = events.begin();
    while(it != events.end()) {
        const SDL_Event& event = *it;
        if(event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            return true;
        }
        it++;
    }
    return false;
} 

bool window_resized(const std::vector<SDL_Event>& events) {
    for(int i = 0; i < events.size(); i++) {
        SDL_Event event = events[i];
        if(event.type == SDL_WINDOWEVENT &&
            event.window.event == SDL_WINDOWEVENT_RESIZED) {
            return true;
        }
    }
    return false;
} 

vicmil::__layout__::MouseInput get_mouse_input(vicmil::SDLUserInputRef user_input) {
    vicmil::__layout__::MouseInput mouse_input;
    mouse_input.was_clicked = mouse_left_clicked(user_input.get_recent_events());
    mouse_input.x = user_input.get_mouse_state().x();
    mouse_input.y = user_input.get_mouse_state().y();
    return mouse_input;
}

/*class Button: public Widget {
};
class Slider: Widget {

};
class Switch: Widget {

};
class TextBox {

};
class DropDown {

};
class Draggable { // For things you can drag on screen 

};
class ScatterPlot { // Plot where you can select data points

};*/
}