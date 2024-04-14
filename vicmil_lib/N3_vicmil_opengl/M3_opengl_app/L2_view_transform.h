#include "L1_model_loading.h"
/**
 * Helper functions to create a view transform
 *  a view transform is a 4x4 matrix that is applied to all triangle corners in a scene to change triangle position
 *  The view tranform can for example be used to move objects relative to the camera, or to set a certain camera angle
 * 
 * It can also be used in a 2D environment to zoom into a particular part of the scene for example
*/

namespace vicmil {
/**
 * How to transform the verticies(triangle corners)
*/
struct CameraViewTransform {
    glm::mat4 mat;
    CameraViewTransform(glm::mat4 mat_) {
        mat = mat_;
    }
};

class CameraViewTransformGen {
public:
    glm::mat4 camera_rotation = glm::mat4(1.0);
    glm::vec3 camera_position = glm::vec3(0.0);
    glm::mat4 obj_rotation = glm::mat4(1.0);
    glm::vec3 obj_position = glm::vec3(0.0);
    float obj_scale = 1.0;
    glm::mat4 projection_matrix;
    static glm::mat4 get_translation_matrix(glm::vec3 pos) {
        return glm::translate(glm::mat4(1.0f), pos);
    }
    static glm::mat4 get_scaling_matrix(float scale) {
        glm::mat4 scaling_matrix = glm::mat4(1.0f);
        scaling_matrix[0][0] = scale;
        scaling_matrix[1][1] = scale;
        scaling_matrix[2][2] = scale;
        return scaling_matrix;
    }
    glm::mat4 get_perspective_matrix_MVP() {
        glm::mat4 model = get_translation_matrix(obj_position) * obj_rotation * get_scaling_matrix(obj_scale);
        glm::mat4 view = camera_rotation * get_translation_matrix(-camera_position);
        return projection_matrix * view * model;
    }
};

/**
 * Represent a camera view in 2d
*/
class CameraView2d {
    public:
    Rect rect = Rect(-1, -1, 2, 2); // Where on the screen to look
    bool flip_vertical = false;
    double rotation_rad = 0; // Add rotation to window
    CameraViewTransform get_transform() {
        ThrowNotImplemented();
    }
};

/**
 * Represent a camera view in 3D
*/
class CameraView3D {
    glm::dvec3 pos;
    double zoom;
    double aspect_ratio; // screen width / height
    Rotation rotation;
    CameraViewTransform get_transform() {
        ThrowNotImplemented();
    }
};

struct WindowInfo {
    // How many pixels the GPU thinks the screen is
    int gpu_mapping_width;
    int gpu_mapping_height;

    // How big the window actually is on screen
    double width;
    double height;
    double get_aspect_ratio() {
        return width / height;
    }
};

/**
 * Get mouse position on the screen in openGL coordinates, eg from -1 to 1
*/
double get_mouse_pos_x(WindowInfo& window_info, MouseState mouse_state = MouseState()) {
    return vicmil::x_pixel_to_opengl(mouse_state.x(), window_info.width);
}
double get_mouse_pos_y(WindowInfo& window_info, MouseState mouse_state = MouseState()) {
    return vicmil::y_pixel_to_opengl(mouse_state.y(), window_info.height);
}

/**
 * Determines if mouse is inside a ractangle in screen coordinates, eg from -1 to 1 in openGL fashion
*/
bool mouse_inside_rect(Rect& rect, WindowInfo& window_info, MouseState mouse_state = MouseState()) {
    double mouse_x = get_mouse_pos_x(window_info, mouse_state);
    double mouse_y = get_mouse_pos_y(window_info, mouse_state);
    return rect.is_inside_rect(mouse_x, mouse_y);
}

vicmil::Rect get_opengl_position(vicmil::RectT<int> rect, vicmil::RectT<int> full_window) {
    vicmil::Rect return_rect;
    return_rect.w = vicmil::w_pixel_to_opengl(rect.w, full_window.w);
    return_rect.h = vicmil::h_pixel_to_opengl(rect.h, full_window.h);
    return_rect.x = vicmil::x_pixel_to_opengl(rect.x, full_window.w);
    return_rect.y = vicmil::y_pixel_to_opengl(rect.y, full_window.h);
    return_rect.y -= return_rect.h;
    return return_rect;
}

vicmil::Rect get_opengl_position(vicmil::LayoutRect element_) {
    vicmil::RectT<int> full_window = element_.entire_window().get_position();
    vicmil::RectT<int> rect = element_.get_position();
    return get_opengl_position(rect, full_window);
}
}