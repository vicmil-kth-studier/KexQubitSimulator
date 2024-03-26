#include "L6_data_layout.h"

namespace vicmil {
/**
 * Create a screenshot of the current keyboard state, with what keys are pressed down
*/
class KeyboardState {
    const Uint8* state = SDL_GetKeyboardState(nullptr);
public:
    bool key_is_pressed(unsigned int sdl_key) {
        return (bool)state[sdl_key];
    }
    bool escape_key_is_pressed() {
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
    int x() {
        return _x;
    }
    int y() {
        return _y;
    }
    bool left_button_is_pressed() {
        return (bool)(_button_state & SDL_BUTTON(1));
    }
    bool middle_button_is_pressed() {
        return (bool)(_button_state & SDL_BUTTON(2));
    }
    bool right_button_is_pressed() {
        return (bool)(_button_state & SDL_BUTTON(3));
    }
};

/**
 * Mouse tracker, track things such as mouse clicks
*/
class MouseTracker {
    MouseState _last_mouse_state = MouseState();
public:
    MouseState state = MouseState();
    void update(MouseState new_mouse_state = MouseState()) {
        _last_mouse_state = state;
        state = new_mouse_state;
    }
    bool mouse_left_clicked() {
        if(!_last_mouse_state.left_button_is_pressed() && state.left_button_is_pressed()) {
            return true;
        }
        return false;
    }
    bool mouse_right_clicked() {
        if(!_last_mouse_state.right_button_is_pressed() && state.right_button_is_pressed()) {
            return true;
        }
        return false;
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
}