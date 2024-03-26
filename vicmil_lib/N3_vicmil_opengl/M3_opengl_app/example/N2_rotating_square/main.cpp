#define USE_DEBUG
#define DEBUG_KEYWORDS "." 
#include "../../vicmil_opengl_app.h"

static bool quitting = false;
static vicmil::GPUSetup setup;
float rotation = 0.0;

void render() {
    DisableLogging;
    START_TRACE_FUNCTION();
    setup.window_renderer.set_to_current_window();
    vicmil::clear_screen();

    rotation += 0.001;
    vicmil::Rotation rot = vicmil::Rotation::from_axis_rotation(rotation, glm::dvec3(0, 0, 1));
    vicmil::set_uniform_mat4f(rot.to_matrix4x4(), setup.program.program, "u_MVP");

    // Draw a triangle from the 3 vertices
    setup.draw();

    setup.show_on_screen();
    END_TRACE_FUNCTION();
}

void update(){
    vicmil::update_SDL();
    render();
};

int main(int argc, char *argv[]) {
    Debug("Start!");
    setup = vicmil::general_gpu_setup::create_gpu_setup();

    std::vector<vicmil::general_gpu_setup::Triangle> triangles = 
        vicmil::general_gpu_setup::triangles_from_2d_color_rect(vicmil::Rect(-0.2, -0.2, 0.4, 0.4), glm::dvec4(0, 1, 0, 1));

    Print(triangles[0].to_string());
    Print(triangles[1].to_string());

    vicmil::general_gpu_setup::set_triangles(setup, triangles);

    // register update as callback
    while(true) {
        update();
    }
}