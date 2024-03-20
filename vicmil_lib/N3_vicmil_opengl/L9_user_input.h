#include "L8_drawing.h"

namespace vicmil {
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
    int _x;
    int _y;
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
 * Creates a button on the screen that you can check if it is pressed or not
 * @param start_x: x position on screen, goes from -1 to 1
 * @param start_y: y position on screen, goes from -1 to 1
 * @param width: width on screen, goes from 0 to 2
 * @param height: height on screen, goes from 0 to 2
*/
class Button {
public:
    double start_x;
    double start_y;
    double width;
    double height;
    unsigned int screen_width = 1000;
    unsigned int screen_height = 1000;
    bool is_pressed(MouseState mouse_state) {
        double mouse_x = x_pixel_to_opengl(mouse_state.x(), screen_width);
        double mouse_y = y_pixel_to_opengl(mouse_state.y(), screen_height);
        if(mouse_state.left_button_is_pressed() == false) {
            return false;
        }
        if(mouse_x < start_x) {
            return false;
        }
        if(mouse_y < start_y) {
            return false;
        }
        if(mouse_x > start_x + width) {
            return false;
        }
        if(mouse_y > start_y + height) {
            return false;
        }
        return true;
    }
};

/**
 * Create a way to move the camera around as you press left click and drag
*/
class CameraMover {
public:
    Camera* camera = nullptr;
    double rotation_speed = 0.01;
    double movement_speed = 0.1;
    int last_mouse_pos[2];
    bool enabled = false;
    CameraMover() {}
    CameraMover(Camera* camera_) {
        camera = camera_;
    }
    void update_rotation() {
        MouseState mouse_state = MouseState();
        if(enabled) {
            int mouse_diff_x = last_mouse_pos[0] - mouse_state.x();
            int mouse_diff_y = last_mouse_pos[1] - mouse_state.y();
            camera->rotate(mouse_diff_y*rotation_speed, mouse_diff_x*rotation_speed);
        }
        enabled = mouse_state.left_button_is_pressed();
        last_mouse_pos[0] = mouse_state.x();
        last_mouse_pos[1] = mouse_state.y();
    }
    void update_movement() {
        KeyboardState keyboard_state = KeyboardState();
        if (keyboard_state.key_is_pressed(SDL_SCANCODE_A)) {
            glm::vec4 movement_vec = camera->get_left_right_rotation_matrix_inv() * glm::vec4(1.0, 0.0, 0.0, 1.0);
            camera->position[0] += movement_speed * movement_vec[0];
            camera->position[2] += movement_speed * movement_vec[2];
        }
        if (keyboard_state.key_is_pressed(SDL_SCANCODE_D)) {
            glm::vec4 movement_vec = camera->get_left_right_rotation_matrix_inv() * glm::vec4(-1.0, 0.0, 0.0, 1.0);
            camera->position[0] += movement_speed * movement_vec[0];
            camera->position[2] += movement_speed * movement_vec[2];
        }
        if (keyboard_state.key_is_pressed(SDL_SCANCODE_W)) {
            glm::vec4 movement_vec = camera->get_left_right_rotation_matrix_inv() * glm::vec4(0.0, 0.0, 1.0, 1.0);
            camera->position[0] += movement_speed * movement_vec[0];
            camera->position[2] += movement_speed * movement_vec[2];
        }
        if (keyboard_state.key_is_pressed(SDL_SCANCODE_S)) {
            glm::vec4 movement_vec = camera->get_left_right_rotation_matrix_inv() * glm::vec4(0.0f, 0.0, -1.0, 1.0);
            camera->position[0] += movement_speed * movement_vec[0];
            camera->position[2] += movement_speed * movement_vec[2];
        }
        if (keyboard_state.key_is_pressed(SDL_SCANCODE_SPACE)) {
            camera->position[1] += movement_speed; // This means we should move up, so y value should increase
        }
        if (keyboard_state.key_is_pressed(SDL_SCANCODE_LSHIFT)) {
            camera->position[1] -= movement_speed; // This means we want to move down, so y value should decrease
        }
    }
    void update() {
        update_rotation();
        update_movement();
    }
};
}