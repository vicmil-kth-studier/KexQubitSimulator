#include "L4_gpu_setup_general.h"
#include <list>

namespace vicmil {

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

class MouseInputWrapper: public vicmil::__layout__::MouseInputInterface {
public:
    vicmil::SDLUserInputRef user_input;
    MouseInputWrapper(vicmil::SDLUserInputRef _user_input) {
        user_input = _user_input;
    }
    int x() override {
        return user_input.get_mouse_state().x();
    }
    int y() override {
        return user_input.get_mouse_state().y();
    }
    bool has_clicked() override {
        return vicmil::mouse_left_clicked(user_input.get_recent_events());
    }
};

/*class _Widget: public vicmil::__layout__::AttachedElement {
public:
    double depth = 0; // To differentiate which widget is on top in case of conflict(lower depth is the top one)
    bool on_top = false;
    RectT<int> position = RectT<int>(0, 0, 0, 0);
    RectT<int> entire_window_position = RectT<int>(0, 0, 0, 0);
    vicmil::__layout__::WindowLayoutElement _layout_element;
    void update() override { // Called each time _layout_element updates position
        position = _layout_element.get_position();
        entire_window_position = _layout_element._get_window_layout_element(0).get_position();
    }
};

class Widget {
public:
    std::shared_ptr<_Widget> widget;
    vicmil::SDLUserInputRef user_input;
    Widget() {
        widget = std::make_shared<_Widget>();
    }
    void set_layout_element(vicmil::__layout__::WindowLayoutElement layout_element_) {
        widget->_layout_element = layout_element_;
        layout_element_.set_attached_element(widget);
    }
    void set_depth(double depth) {
        widget->depth = depth;
    }
    void set_user_input_reference(vicmil::SDLUserInputRef user_input_) {
        user_input = user_input_;
    }
    bool is_top_widget() {
        return widget->on_top;
    }
    RectT<int> get_position() {
        return widget->position;
    }
    // The widgets tick update is called every tick
    virtual void tick_update() = 0;
};

class WidgetUpdater {
public:
    vicmil::SDLUserInputRef user_input;

    WidgetUpdater() {}
    WidgetUpdater(vicmil::SDLUserInputRef user_input_) {
        user_input = user_input_;
    }

    // Keeps track of widgets and updates them every tick
    // Keeps track of which widget is on top
    std::list<std::weak_ptr<Widget>> widgets = {};
    void _remove_expired_widgets() {
        auto it = widgets.begin();
        while(it != widgets.end()) {
            if((*it).expired()) {
                Print("Removed expired widget!");
                auto it_copy = it;
                it++;
                widgets.erase(it_copy);
                continue;
            }
            it++;
        }
    }
    void _update_top_widget() {
        int mouse_position_x = user_input.get_mouse_state().x();
        int mouse_position_y = user_input.get_mouse_state().y();
        bool found_top = false;
        auto top_widget = widgets.begin();
        auto it = widgets.begin();
        //Print("Fetch top widget");
        while(it != widgets.end()) {
            _Widget& widget = *(*it).lock()->widget;
            widget.on_top = false;
            if(widget.position.is_inside_rect(mouse_position_x, mouse_position_y)) {
                if(!found_top) {
                    top_widget = it;
                    found_top = true;
                }
                else if(widget.depth < (*top_widget).lock()->widget->depth) {
                    top_widget = it;
                    found_top = true;
                }
            }
            it++;
        }
        if(found_top) {
            //Print("Found top widget!");
            // Set the top widget to be ontop
            (*top_widget).lock()->widget->on_top = true;
        }
    }
    void _update_all_widgets() {
        auto it = widgets.begin();
        while(it != widgets.end()) {
            (*it).lock()->tick_update();
            it++;
        }
    }
    void update() {
        _remove_expired_widgets();
        _update_top_widget();
        _update_all_widgets();
    }
    void add_widget(std::weak_ptr<Widget> new_widget) {
        widgets.push_back(new_widget);
    }
};*/

/*class Button: public Widget {
public:
    bool button_pressed = false;
    bool last_widget_pressed = false;
    void tick_update() override {
        bool left_button_clicked = mouse_left_clicked(user_input.get_recent_events());
        PrintExpr(is_top_widget());
        PrintExpr(last_widget_pressed);
        if(is_top_widget() && left_button_clicked) {
            button_pressed = true;
            last_widget_pressed = true;
            button_pressed_event();
        }
        else if(!is_top_widget() && left_button_clicked) {
            button_pressed = false;
            last_widget_pressed = false;
            something_else_pressed_event();
        }
        if(!is_top_widget() || !user_input.get_mouse_state().left_button_is_pressed()) {
            button_pressed = false;
        }
    }
    virtual void button_pressed_event() {} // Called each time the button is pressed(mouse left click)
    virtual void something_else_pressed_event() {} // Called each time the button was pressed, but then something else got pressed
};
/*class Slider: Widget {

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