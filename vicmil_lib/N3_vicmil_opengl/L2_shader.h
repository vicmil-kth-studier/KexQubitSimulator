// Load shader from file and store it in a class
#include "L1_util.h"

namespace vicmil {
std::string get_shader_type_name(unsigned int shader_type) {
    if(shader_type == GL_FRAGMENT_SHADER) {
        return "fragment shader";
    }
    if(shader_type == GL_VERTEX_SHADER) {
        return "vertex shader";
    }
    return "unknown shader";
}

class Shader {
public:
    std::string raw_content;
    unsigned int id;
    unsigned int type;
    Shader() {}
    static Shader from_str(const std::string& str_, unsigned int type_) {
        Shader new_shader = Shader();
        new_shader.raw_content = str_;
        new_shader.compile_shader(type_);
        return new_shader;
    }
    static Shader from_file(const std::string& filename, unsigned int type_) {
        Shader new_shader = Shader();
        new_shader.raw_content = read_file_contents(filename);
        new_shader.compile_shader(type_);
        return new_shader;
    }
    void compile_shader(unsigned int shader_type) {
        type = shader_type;
        id = glCreateShader(shader_type);
        const char* ptr = raw_content.c_str();
        glShaderSource(id, 1, &ptr, nullptr);
        glCompileShader(id);
        print_errors("Failed to compile");
    }
    void print_errors(std::string context_message) {
        int result;
        GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
        if(result == GL_FALSE) {
            int length;
            GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
            char* err_message = (char*) alloca(length * sizeof(char));
            GLCall(glGetShaderInfoLog(id, length, &length, err_message));
            std::cout << context_message << ": " << get_shader_type_name(type) << std::endl;
            std::cout << err_message << std::endl;
            GLCall(glDeleteShader(id));
        }
    }

};

class VertexShader {
public:
    Shader shader;
    static VertexShader from_str(const std::string& str_) {
        VertexShader new_shader = VertexShader();
        new_shader.shader = Shader::from_str(str_, GL_VERTEX_SHADER);
        return new_shader;
    }
    static VertexShader from_file(const std::string& filename) {
        VertexShader new_shader = VertexShader();
        new_shader.shader = Shader::from_file(filename, GL_VERTEX_SHADER);
        return new_shader;
    }
    VertexShader() {}
};

class FragmentShader {
public:
    Shader shader;
    static FragmentShader from_str(const std::string& str_) {
        FragmentShader new_shader = FragmentShader();
        new_shader.shader = Shader::from_str(str_, GL_FRAGMENT_SHADER);
        return new_shader;
    }
    static FragmentShader from_file(const std::string& filename) {
        FragmentShader new_shader = FragmentShader();
        new_shader.shader = Shader::from_file(filename, GL_FRAGMENT_SHADER);
        return new_shader;
    }
    FragmentShader() {}
};

class GPUProgram {
public:
    unsigned int id;
    static GPUProgram from_strings(const std::string& vert_str, const std::string& frag_str) {
        GPUProgram new_program = GPUProgram();
        new_program.id = glCreateProgram();
        VertexShader vert_shader = VertexShader::from_str(vert_str);
        FragmentShader frag_shader = FragmentShader::from_str(frag_str);
        new_program._attach_shaders(vert_shader, frag_shader);
        new_program._delete_shaders(vert_shader, frag_shader); // The shaders are already linked and are therefore not needed
        return new_program;
    }
    static GPUProgram from_files(const std::string& vert_filename, const std::string& frag_filename) {
        GPUProgram new_program = GPUProgram();
        new_program.id = glCreateProgram();
        VertexShader  vert_shader = VertexShader::from_file(vert_filename);
        FragmentShader frag_shader = FragmentShader::from_file(frag_filename);
        new_program._attach_shaders(vert_shader, frag_shader);
        new_program._delete_shaders(vert_shader, frag_shader); // The shaders are already linked and are therefore not needed
        return new_program;
    }
    void bind_program() {
        START_TRACE_FUNCTION();
        // You need to bind the program to use it, only one program can be bound at the same time
        GLCall(glUseProgram(id));
        END_TRACE_FUNCTION();
    }
    void _delete_shaders(VertexShader& vert_shader, FragmentShader& frag_shader) {
        GLCall(glDeleteShader(vert_shader.shader.id));
        GLCall(glDeleteShader(frag_shader.shader.id));
    }
    void _attach_shaders(VertexShader& vert_shader, FragmentShader& frag_shader) {
        GLCall(glAttachShader(id, vert_shader.shader.id));
        GLCall(glAttachShader(id, frag_shader.shader.id));
        GLCall(glLinkProgram(id));
        GLCall(glValidateProgram(id));
    }
    void delete_program() {
        glDeleteProgram(id);
    }
    ~GPUProgram() {}
};

/**
 * Some example shaders
 *  - drawing models
 *  - drawing textures/text
*/
namespace shader_example {
#ifdef __EMSCRIPTEN__
const std::string version_str = ""; // No need to specify version in emscripten!
#else
const std::string version_str = "#version 130\n";
#endif
const std::string vert_shader =
version_str +
"uniform mat4 u_MVP;\n"
"attribute vec3 position;\n"
"attribute vec3 color;\n"
"\n"
"varying vec4 v_Color;\n"
"\n"
"void main() {\n"
"    gl_Position = u_MVP * vec4(position.x, position.y, position.z, 1.0);\n"
"    v_Color = vec4(color.x, color.y, color.z, 1.0);\n"
"}\n";

const std::string frag_shader =
version_str +
"precision mediump float;\n"
"varying vec4 v_Color;\n"
"\n"
"void main() {\n"
"    gl_FragColor = v_Color;\n"
"}\n";

const std::string texture_vert_shader =
version_str +
"uniform mat4 u_MVP;\n"
"attribute vec3 position;\n"
"attribute vec3 color;\n"
"attribute vec2 aTexCoord;\n"
"\n"
"varying vec4 v_Color;\n"
"varying vec2 TexCoord;\n"
"\n"
"void main() {\n"
"    gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
"    v_Color = vec4(color.x, color.y, color.z, 1.0);\n"
"    TexCoord = aTexCoord;\n"
"}\n";

const std::string texture_frag_shader =
version_str +
"precision mediump float;\n"
"varying vec4 v_Color;\n"
"varying vec2 TexCoord;\n"
"\n"
"uniform sampler2D ourTexture;\n"
"\n"
"void main() {\n"
"    // gl_FragColor = v_Color;\n"
"    gl_FragColor = texture2D(ourTexture, TexCoord);\n"
"}\n";
}
}