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

    //rotation += 0.001;
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

    // Create texture of all ascii
    Debug("get_ascii_images");
    std::map<std::string, vicmil::RawImageRGBA> ascii_images = vicmil::get_ascii_images();
    Debug("RawImageRGBAPacking(ascii_images)");
    vicmil::RawImageRGBAPacking image_packing = vicmil::RawImageRGBAPacking(ascii_images);
    Debug("to_png");
    image_packing.packed_image.to_png("ascii_image.png");

    vicmil::TexturePacking packed_texture = vicmil::TexturePacking(image_packing);
    packed_texture.packed_texture.bind();

    std::vector<vicmil::general_gpu_setup::Triangle> triangles = 
        vicmil::general_gpu_setup::triangles_from_2d_texture_rect(vicmil::Rect(-0.2, -0.2, 0.4, 0.4), vicmil::Rect(0, 0, 1, 1));

    vicmil::Rect text_rect = vicmil::Rect(-0.8, 0.8, 0.1, 0.2);
    std::string text = "hello world!";
    for(int i = 0; i < text.size(); i++) {
        std::string char_ = std::to_string(text[i]);
        vicmil::vec_extend(triangles, vicmil::general_gpu_setup::triangles_from_texture_mapping_key(char_, text_rect, packed_texture.image_pos));
        text_rect.x += text_rect.w;
    }

    Print(triangles[0].to_string());
    Print(triangles[1].to_string());

    vicmil::general_gpu_setup::set_triangles(setup, triangles);

    // register update as callback
    while(true) {
        update();
    }
}