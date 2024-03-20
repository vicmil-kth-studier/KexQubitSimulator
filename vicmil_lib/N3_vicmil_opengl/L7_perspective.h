#include "L6_model_loading.h"

namespace vicmil {

class ModelOrientation {
public:
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f); // Default to no position change
    glm::mat4 rotation = glm::mat4(1.0f); // Default to no rotation(unit matrix)
};

class Camera {
public:
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f); // Default to no position change
    float zoom_degrees = 60.0;

    float radians_up_down; // between -0.5pi and 0.5pi
    float radians_left_right; // between 0 and 2pi // Can be seen as hip rotation

    float screen_aspect_ratio = 4.0f / 3.0f; // Aspect Ratio. Depends on the size of your window. Notice that 4/3 == 800/600 == 1280/960, sounds familiar ?

    glm::mat4 get_rotation_matrix() {
        glm::mat4 up_down_rotation = glm::rotate( radians_up_down, glm::vec3(1, 0, 0) );
        glm::mat4 left_right_rotation = glm::rotate( radians_left_right, glm::vec3(0, 1, 0) );
        return up_down_rotation * left_right_rotation;
    }
    glm::vec3 get_camera_view_vector() {
        glm::vec4 vec = glm::vec4(0, 0, -1, 0); // Default is looking in negative z direction
        glm::vec4 transformed_vec = glm::inverse(get_rotation_matrix()) * vec;
        return glm::vec3(transformed_vec.x, transformed_vec.y, transformed_vec.z);
    }
    glm::mat4 get_left_right_rotation_matrix_inv() {
        glm::mat4 left_right_rotation = glm::rotate( -radians_left_right, glm::vec3(0, 1, 0) );
        return left_right_rotation;
    }
    glm::mat4 get_projection_matrix() {
        // Generates a really hard-to-read matrix, but a normal, standard 4x4 matrix nonetheless
        glm::mat4 projectionMatrix = glm::perspective(
            glm::radians(zoom_degrees), // The vertical Field of View, in radians: the amount of "zoom". Think "camera lens". Usually between 90° (extra wide) and 30° (quite zoomed in)
            screen_aspect_ratio,
            0.1f,              // Near clipping plane. Keep as big as possible, or you'll get precision issues.
            100.0f             // Far clipping plane. Keep as little as possible.
        );
        return projectionMatrix;
    }
    glm::mat4 get_orthographic_projection_matrix() {
        glm::mat4 projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates
        return projection;
    }
    void rotate(float radians_up_down_, float radians_left_right_) {
        this->radians_up_down += radians_up_down_;
        this->radians_left_right += radians_left_right_;
        if(this->radians_up_down > 3.14 / 2) {
            this->radians_up_down = 3.14 / 2;
        }
        if(this->radians_up_down < -3.14 / 2) {
            this->radians_up_down = -3.14 / 2;
        }
        this->radians_left_right = modulo(this->radians_left_right, 2 * PI);
    }
};

class PerspectiveMatrixGen {
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
    void load_object_orientation(ModelOrientation& obj_orientation) {
        obj_rotation = obj_orientation.rotation;
        obj_position = obj_orientation.position;
    }
    void load_camera_state(Camera& camera) {
        camera_rotation = camera.get_rotation_matrix();
        camera_position = camera.position;
        projection_matrix = camera.get_projection_matrix();
    }
};

struct CameraViewLine {
    glm::vec3 start_pos;
    glm::vec3 view_vector;
};
glm::vec4 get_camera_pos_on_screen(Camera& camera) {
    PerspectiveMatrixGen perspective_matrix_gen = PerspectiveMatrixGen();
    perspective_matrix_gen.load_camera_state(camera);

    glm::mat4 mvp = perspective_matrix_gen.get_perspective_matrix_MVP();
    glm::vec4 camera_pos = glm::vec4(camera.position.x, camera.position.y, camera.position.z, 1);
    glm::vec4 camera_pos_on_screen = mvp * camera_pos;
    return camera_pos_on_screen;
}
// Get the line in the real world the camera is looking in
// The screen values are between -1 and 1: (-1, -1) being lower left corner, (0, 0) being in the middle
CameraViewLine get_camera_viewline(double screen_x, double screen_y, Camera& camera) {
    glm::vec4 mouse_position = glm::vec4(screen_x, screen_y, get_camera_pos_on_screen(camera).z, 0);
    glm::vec4 mouse_position2 = glm::vec4(screen_x, screen_y, get_camera_pos_on_screen(camera).z-1.0, -1.0); // A little further into the screen

    PerspectiveMatrixGen perspective_matrix_gen = PerspectiveMatrixGen();
    perspective_matrix_gen.load_camera_state(camera);

    glm::mat4 mvp = perspective_matrix_gen.get_perspective_matrix_MVP();
    //std::cout << "mvp: " << glm::to_string(mvp) << std::endl;
    glm::mat4 mvp_inverse = glm::inverse(mvp);
    //std::cout << "mvp_inverse: " << glm::to_string(mvp_inverse) << std::endl;

    /*{ // For debugging
        glm::vec4 camera_pos = glm::vec4(camera.position.x, camera.position.y, camera.position.z, 1);
        glm::vec4 camera_pos_on_screen = mvp * camera_pos;
        glm::vec4 camera_view_vector = camera.get_rotation_matrix() * glm::vec4(0, 0, 1, 1);
        camera_view_vector += camera_pos;
        glm::vec4 camera_view_vector_screen = mvp * camera_view_vector;
        std::cout << "camera_pos_on_screen: " << glm::to_string(camera_pos_on_screen) << std::endl;
        std::cout << "camera_view_vector_screen: " << glm::to_string(camera_view_vector_screen) << std::endl;
    }*/

    CameraViewLine viewline;
    viewline.start_pos = mvp_inverse * mouse_position;
    viewline.view_vector = mvp_inverse * mouse_position2;
    viewline.view_vector -= viewline.start_pos;
    viewline.view_vector = -glm::normalize(viewline.view_vector);
    //viewline.view_vector.y = -viewline.view_vector.y; // The y values are flipped!
    //viewline.view_vector.x = -viewline.view_vector.x; // The x values are flipped!
    return viewline;
}
TestWrapper(TEST_get_camera_viewline,
    void test() {
        Camera camera = Camera();
        CameraViewLine viewline = get_camera_viewline(0.0, 0.0, camera);
        //std::cout << "start_pos: " << glm::to_string(viewline.start_pos) << std::endl;
        //std::cout << "view_vector: " << glm::to_string(viewline.view_vector) << std::endl;
        Assert(abs(viewline.start_pos.x) < 0.0001);
        Assert(abs(viewline.start_pos.y) < 0.0001);
        Assert(abs(viewline.view_vector.x) < 0.0001);
        Assert(abs(viewline.view_vector.y) < 0.0001);
        Assert(glm::length(viewline.view_vector - glm::vec3(0, 0, -1)) < 0.0001); // Default is looking in negative z direction

        // Lets try rotating the camera a bit and see that we get the right viewline
        camera.rotate(0.2, 0.1);
        CameraViewLine viewline2 = get_camera_viewline(0.0, 0.0, camera);
        glm::vec3 camera_view_vector = camera.get_camera_view_vector();
        DebugExpr(glm::to_string(viewline2.view_vector));
        DebugExpr(glm::to_string(camera_view_vector));
        DebugExpr(glm::to_string(viewline2.start_pos));
        Assert(glm::length(viewline2.start_pos) < 0.0001);
        Assert(glm::length(camera_view_vector - viewline2.view_vector) < 0.0001);
    }
);
}