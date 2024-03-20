#include "L10_obj_mtl_text.h"

namespace vicmil {
    /**
    A wrapper to make it easier to write applications
    */
   namespace app {
        namespace globals {
            static VoidFuncRef game_update_func = VoidFuncRef(); // A reference to the function that will handle the game updates
            static VoidFuncRef render_func = VoidFuncRef(); // A reference to the function that will handle the rendering
            static bool init_called = false;
        }
        
        class App {
        public:
            // Add graphics setup, programs and other stuff here
            vicmil::FrameStabilizer frame_stabilizer;
            GraphicsSetup graphics_setup;
            GPUProgram program;
            GPUProgram texture_program;
            IndexVertexBufferPair index_vertex_buffer;
            Texture text_texture;
            Shared3DModelsBuffer shared_models_buffer;
            Camera camera;
            App() : 
            graphics_setup(GraphicsSetup::create_setup()) {
                frame_stabilizer = FrameStabilizer(30);
                
                // Create Vertex Array Object
                create_vertex_array_object();

                Debug(glGetString (GL_SHADING_LANGUAGE_VERSION));

                // Load gpu program for how to interpret the data
                texture_program = GPUProgram::from_strings(vicmil::shader_example::texture_vert_shader, vicmil::shader_example::texture_frag_shader);
                program = GPUProgram::from_strings(vicmil::shader_example::vert_shader, vicmil::shader_example::frag_shader);
                std::vector<float> vertices = {0.0f, 0.5f, 3.0f,
                         0.5f, -0.5f, 3.0f,
                         -0.5f, -0.5f, 3.0f};
                std::vector<unsigned int> faces = {1, 2, 3};

                index_vertex_buffer = vicmil::IndexVertexBufferPair::from_raw_data(
                    &faces[0], 
                    3*sizeof(unsigned int), 
                    &vertices[0],
                    9 * sizeof(float));

                RawImageRGB raw_image_alphabet = graphics_help::get_raw_image_of_alphabet();
                text_texture = Texture::from_raw_image_rgb(raw_image_alphabet);

                // Load models
                shared_models_buffer = Shared3DModelsBuffer::from_models(graphics_help::get_models_vector());
            }
        };
        namespace globals {
            App* main_app = nullptr;
        }
        
        void app_loop_handler(VoidFuncRef init_func) {
            DisableLogging;
            if(globals::init_called == false) {
                init_SDL();
                if(globals::main_app == nullptr) {
                    globals::main_app = new App();
                }   
                init_func.call();
                globals::init_called = true;
            }
            update_SDL();
            Debug("emscripten_loop_handler");
            if(globals::main_app != nullptr) {
                if(globals::main_app->frame_stabilizer.get_time_to_next_frame_s() < 0) {
                    if(globals::game_update_func.try_call() != 0) {
                        Debug("Game update func not set!");
                    }
                    globals::main_app->frame_stabilizer.record_frame();
                }
            }
 
            // Render
            SDL_GL_MakeCurrent(vicmil::app::globals::main_app->graphics_setup.window, vicmil::app::globals::main_app->graphics_setup.gl_context);
            if(globals::render_func.try_call() != 0) {
                Debug("Render func not set!");
            }
            SDL_GL_SwapWindow(vicmil::app::globals::main_app->graphics_setup.window);
        }
        void set_game_update_func(VoidFuncRef func) {
            globals::game_update_func = func;
        }
        void set_render_func(VoidFuncRef func) {
            globals::render_func = func;
        }
        void set_game_updates_per_second(unsigned int updates_per_s) {
            if(globals::main_app != nullptr) {
                globals::main_app->frame_stabilizer = FrameStabilizer(updates_per_s);
            }
        }
        void draw2d_rect() {
            ThrowNotImplemented();
            // Make sure the right shader is loaded

            // Add rectangle to vertex and index buffer

            // Create a mvp matrix that does nothing

            // Ensure that depth testing is disabled

            // Perform the drawing

            // glm::mat4 mvp = glm::mat4(1.0);
            
            // Ensure that depth testing is enabled: TODO

            // Perform the drawing
            // vicmil::app_help::app->shared_models_buffer.draw_object(2, mvp, &vicmil::app_help::app->program);
        }
        void draw2d_text(std::string text, double x = -1.0, double y = 1.0, double letter_width = 0.1) {
            double screen_aspect_ratio = globals::main_app->graphics_setup.get_window_aspect_ratio();

            // Make sure the right shader is loaded
            globals::main_app->texture_program.bind_program();
            
            // Add character rectangles to vertex and index buffer
            TextureTraingles text_trig = graphics_help::get_texture_triangles_from_text(text, x, y, letter_width, screen_aspect_ratio);

            // Load the vertex buffer
            text_trig.overwrite_index_vertex_buffer_pair(globals::main_app->index_vertex_buffer);
            globals::main_app->index_vertex_buffer.bind();
            globals::main_app->index_vertex_buffer.set_texture_vertex_buffer_layout();

            // Make sure the right texture is loaded
            globals::main_app->text_texture.bind();

            // Ensure that depth testing is disabled
            set_depth_testing_enabled(false);

            // Create a mvp matrix that does nothing
            glm::mat4 matrix_mvp = glm::mat4(1.0);
            //UniformBuffer::set_mat4f(matrix_mvp, app->texture_program, "u_MVP");

            // Perform the drawing
            globals::main_app->index_vertex_buffer.draw();
        }
        void draw3d_text() {
            ThrowNotImplemented();
            // Make sure the right shader is loaded

            // Add rectangles to vertex and index buffer

            // Create a mvp matrix that transforms the position according to camera

            // Ensure that depth testing is enabled

            // Perform the drawing
        }
        
        void draw_3d_model(unsigned int model_index, vicmil::ModelOrientation obj_orientation, double scale = 1.0) {
            // Make sure the right shader is loaded
            globals::main_app->program.bind_program();
            
            // Load the correct model:
            globals::main_app->shared_models_buffer.bind();

            // Create a mvp matrix that transforms the position according to camera
            PerspectiveMatrixGen mvp_gen;
            mvp_gen.obj_scale = scale;
            mvp_gen.load_camera_state(vicmil::app::globals::main_app->camera);
            mvp_gen.load_object_orientation(obj_orientation);
            glm::mat4 mvp = mvp_gen.get_perspective_matrix_MVP();
            
            // Ensure that depth testing is enabled:
            set_depth_testing_enabled(true);

            // Perform the drawing
            vicmil::app::globals::main_app->shared_models_buffer.draw_object(model_index, mvp, &vicmil::app::globals::main_app->program);
        }

        class TextButton {
        public:
            std::string text;
            double letter_width = 0.04;
            double center_x;
            double center_y;
            bool is_pressed(vicmil::MouseState mouse_state) {
                double width = vicmil::graphics_help::get_letter_width_with_spacing(letter_width) * text.size();;
                double height = vicmil::graphics_help::get_letter_height_with_spacing(letter_width, vicmil::app::globals::main_app->graphics_setup.get_window_aspect_ratio());
                double x = center_x - width / 2;
                double y = center_y + height / 2 - height;

                Button button = Button();
                button.screen_width = vicmil::app::globals::main_app->graphics_setup.width;
                button.screen_height = vicmil::app::globals::main_app->graphics_setup.height;
                button.width = width;
                button.height = height;
                button.start_x = x;
                button.start_y = y;
                return button.is_pressed(mouse_state);
            }
            void draw() {
                double width = vicmil::graphics_help::get_letter_width_with_spacing(letter_width) * text.size();
                double height = vicmil::graphics_help::get_letter_height_with_spacing(
                    letter_width, 
                    vicmil::app::globals::main_app->graphics_setup.get_window_aspect_ratio()
                );
                double x = center_x - width / 2;
                double y = center_y + height / 2;
                draw2d_text(text, x, y, letter_width);
            }
        };

        class TextConsole {
        public:
            std::vector<std::string> text_buffer;
            int total_line_count = 0;
            double min_x = -1;
            double min_y = -1;
            double max_x = 0;
            double max_y = 0;

            double letter_width = 0.01;
        
            TextConsole() {
                text_buffer.resize(100);
            }

            int get_buffer_index(int line_num) {
                int buffer_size = text_buffer.size();
                
                // Can also handle negative line numbers
                return ((line_num%buffer_size) + buffer_size) % buffer_size;
            }
            void log(std::string log_message) {
                std::vector<std::string> log_lines = split_string(log_message, '\n');
                for(int i = 0; i < log_lines.size(); i++) {
                    text_buffer[get_buffer_index(total_line_count)] = log_lines[i];
                    total_line_count += 1;
                }
            }

            void clear() {
                total_line_count = 0;
                for(int i = 0; i < text_buffer.size(); i++) {
                    text_buffer[i] = "";
                }
            }
            void draw(double screen_aspect_ratio = -1) {
                std::string draw_string;
                double letter_width_with_spacing = vicmil::graphics_help::get_letter_width_with_spacing(letter_width);
                double letter_height_with_spacing = vicmil::graphics_help::get_letter_height_with_spacing(letter_width, globals::main_app->graphics_setup.get_window_aspect_ratio());
                int max_characters_per_line = (int)((max_x - min_x) / letter_width_with_spacing);
                int max_line_count =          (int)((max_y - min_y) / letter_height_with_spacing);

                int draw_line_count = std::min((int)text_buffer.size(), max_line_count);

                int first_line = std::max(total_line_count - draw_line_count, 0);
                int last_line = total_line_count;
                for(int i = first_line; i < last_line; i++) {
                    int line_index = get_buffer_index(i);
                    std::string line_str = text_buffer[line_index];
                    if(line_str.size() > max_characters_per_line) {
                        line_str.resize(max_characters_per_line);
                    }
                    draw_string += line_str + "\n";
                }
                draw2d_text(draw_string, min_x, max_y, letter_width);
            }

        };
    }
}