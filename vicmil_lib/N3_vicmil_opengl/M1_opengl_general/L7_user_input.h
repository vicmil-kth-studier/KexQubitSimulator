#include "L6_data_layout.h"

namespace vicmil {
/**
 * Create a screenshot of the current keyboard state, with what keys are pressed down
*/
class KeyboardState {
    const Uint8* state = SDL_GetKeyboardState(nullptr);
public:
    bool key_is_pressed(unsigned int sdl_key) const {
        return (bool)state[sdl_key];
    }
    bool escape_key_is_pressed() const {
        return key_is_pressed(SDL_SCANCODE_ESCAPE);
    }
};

/**
 * Create a screenshot of the current mouse state, with mouse position and button presses
*/
class MouseState {
public:
    int _x; // The x position in pixels
    int _y; // The y position in pixels
    Uint32 _button_state;
    MouseState() { // May not work if update_SDL has not been called for a while
        _button_state = SDL_GetMouseState(&_x, &_y);
    }
    int x() const {
        return _x;
    }
    int y() const {
        return _y;
    }
    bool left_button_is_pressed() const {
        return (bool)(_button_state & SDL_BUTTON(1));
    }
    bool middle_button_is_pressed() const {
        return (bool)(_button_state & SDL_BUTTON(2));
    }
    bool right_button_is_pressed() const {
        return (bool)(_button_state & SDL_BUTTON(3));
    }
};

/**
 * converts pixels to opengl format
 * x = -1 is the left edge in opengl
 * x = 1 is the right edge in opengl
 * @param x_pixel The x pixel coordinate(0 is the left)
 * @param screen_width The screen width in pixels
*/
double x_pixel_to_opengl(int x_pixel, int screen_width) {
    return (2 * ((double)x_pixel) / screen_width) - 1;
}
/**
 * converts pixels to opengl format
 * y = -1 is the bottom in opengl
 * y = 1 is the top in opengl
 * @param y_pixel The y pixel coordinate(0 is the top)
 * @param screen_height The screen height in pixels
*/
double y_pixel_to_opengl(int y_pixel, int screen_height) {
    return -((2 * ((double)y_pixel) / screen_height) - 1);
}
/**
 * converts pixels to opengl format
 * x = -1 is the left edge in opengl
 * x = 1 is the right edge in opengl
 * @param w_pixel The width in pixels
 * @param screen_width The screen width in pixels
*/
double w_pixel_to_opengl(int w_pixel, int screen_width) {
    return (2 * ((double)w_pixel) / screen_width);
}
/**
 * converts pixels to opengl format
 * y = -1 is the bottom in opengl
 * y = 1 is the top in opengl
 * @param h_pixel The height in pixels
 * @param screen_height The screen height in pixels
*/
double h_pixel_to_opengl(int h_pixel, int screen_height) {
    return (2 * ((double)h_pixel) / screen_height);
}


/**
 * Class for storing the current user input state(mouse state, keyboard state, events since last update)
*/
struct _SDLUserInput {
    KeyboardState last_captured_keyboard_state;
    MouseState last_captured_mouse_state;
    std::vector<SDL_Event> last_captured_events = {};
};
/**
 * Class for referencing the _SDLUserInput 
 * makes it easy to have multiple references to the same object
 * Updating and reference creation is managed by SDLUserInputManager
*/
class SDLUserInputRef {
public:
    std::weak_ptr<_SDLUserInput> _user_input;
    const KeyboardState& get_keyboard_state() {
        return _user_input.lock()->last_captured_keyboard_state;
    }
    const MouseState& get_mouse_state() {
        return _user_input.lock()->last_captured_mouse_state;
    }
    const std::vector<SDL_Event>& get_recent_events() {
        return _user_input.lock()->last_captured_events;
    }
};
/**
 * Manages the _SDLUserInput object. 
 * + Create references to the object using create_reference
 * + Update the user input state using fetch_events_and_update
*/
class SDLUserInputManager {
public:
    std::shared_ptr<_SDLUserInput> user_input;
    SDLUserInputManager() {}
    static SDLUserInputManager create_user_input_manager() {
        SDLUserInputManager new_user_input_manager = SDLUserInputManager();
        new_user_input_manager.user_input = std::make_shared<_SDLUserInput>();
        return new_user_input_manager;
    }
    void _update_keyboard_and_mouse_state() {
        user_input->last_captured_keyboard_state = KeyboardState();
        user_input->last_captured_mouse_state = MouseState();
    }
    void _fetch_sdl_recent_events() {
        user_input->last_captured_events = vicmil::update_SDL();
    }
    void fetch_events_and_update() {
        _update_keyboard_and_mouse_state();
        _fetch_sdl_recent_events();
    }
    SDLUserInputRef get_reference() {
        SDLUserInputRef new_user_input_ref;
        new_user_input_ref._user_input = user_input;
        return new_user_input_ref;
    }
};
}