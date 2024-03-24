// Load shader from file and store it in a class
#include "L1_util.h"

namespace vicmil {

/**
 * Convert the shader type as int to a string
*/
std::string get_shader_type_name(unsigned int shader_type) {
    if(shader_type == GL_FRAGMENT_SHADER) {
        return "fragment shader";
    }
    if(shader_type == GL_VERTEX_SHADER) {
        return "vertex shader";
    }
    return "unknown shader";
}

/**
 * Wrapper class to create and manipulate a shader on the GPU
 *   A shader is basically a program that runs on the GPU
 *   The vertex shader is executed once for every triangle corner
 *   The fragment shader is executed once for every pixel in every triangle, 
 *      it will interpolate between the triangle corners as input data
 * 
 *   You will need both a fragment shader AND a vertex shader to see anything on the screen
*/   
class Shader {
public:
    std::string raw_content;  // The source code of the shader
    unsigned int id;  // The id of the shader, used to reference it on the GPU
    unsigned int type; // The type of the shader, eg fragment shader or vertex shader
    Shader() {}

    /**
     * Create a new shader on the GPU
     * @arg str_: the shader program
     * @arg type_; The type of shader as an int, eg GL_FRAGMENT_SHADER or GL_VERTEX_SHADER
    */
    static Shader from_str(const std::string& str_, unsigned int type_) {
        Shader new_shader = Shader();
        new_shader.raw_content = str_;
        new_shader.compile_shader(type_);
        return new_shader;
    }
    /**
     * Read a file and create a shader from it
     * @arg filename: the name or path to the file with the shader source code.
     * @arg type_: The type of shader as an int, eg GL_FRAGMENT_SHADER or GL_VERTEX_SHADER
    */
    static Shader from_file(const std::string& filename, unsigned int type_) {
        Shader new_shader = Shader();
        new_shader.raw_content = read_file_contents(filename);
        new_shader.compile_shader(type_);
        return new_shader;
    }
    /**
     * Compile the shader using the source code, this will generate a new shader instance on the GPU
    */
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

/**
 * Wrapper class for vertex shader
*/
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

/**
 * Wrapper class for fragment shader
*/
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

/**
 * Wrapper class for a GPU program
 * A GPU program consist of two sub-programs, the vertex shader and the fragment shader
*/
class GPUProgram {
public:
    unsigned int id; // A reference to the program on the GPU
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
    /**
     * This tells OpenGL that we will use this program going forward, until we bind another program
     *   Only one program can be bound at the same time
    */
    void bind_program() {
        START_TRACE_FUNCTION();
        GLCall(glUseProgram(id));
        END_TRACE_FUNCTION();
    }
    /**
     * Delete both the fragment shader and the vertex shader from the GPU
    */
    void _delete_shaders(VertexShader& vert_shader, FragmentShader& frag_shader) {
        GLCall(glDeleteShader(vert_shader.shader.id));
        GLCall(glDeleteShader(frag_shader.shader.id));
    }
    /**
     * Attach the shaders to the current program
     *  this tells the GPU that these shaders are the ones that make up the program
     *  The vert_shader and frag_shader are copied to the program, so we don't need 
     *  to keep the old shaders for the program to work
    */
    void _attach_shaders(VertexShader& vert_shader, FragmentShader& frag_shader) {
        GLCall(glAttachShader(id, vert_shader.shader.id));
        GLCall(glAttachShader(id, frag_shader.shader.id));
        GLCall(glLinkProgram(id));
        GLCall(glValidateProgram(id));
    }
    /**
     * Delete the program frome the GPU
    */
    void delete_program() {
        glDeleteProgram(id);
    }
    ~GPUProgram() {}
};

namespace shader_example {

/**
 * For the shader programs to work we need to specify which version of shader we should use
 *   the OPENGL_ES shader version seems to be very universal, and can be run both in
 *   the browser and locally in an application program, so I would recommend using that
*/
#ifdef __EMSCRIPTEN__
const std::string OPENGL_ES_VERSON = ""; // Emscripten runs OpenGL ES by default
#else
const std::string OPENGL_ES_VERSON = "#version 130\n";
#endif

/** OpenGL ES terminology:
 *    uniform: input variable, it is shared among all instances of the program(eg all vertexes)
 *    attribute: input variable, it is specific for every vertex(triangle corner)
 *    varying: output variable that will be passed to the fragment shader
*/

/**
 * This is an example of a vertex-fragment shader pair that can be used for many things, 
 *   more specialized shaders can use less memory, be faster, or do more advanced things
 *   but this is a one size fits all solution that can do most basic things in both 2D and 3D such as
 *   - Drawing textures
 *   - Drawing triangles
 *   - Drawing text
 *   - Drawing models
 *   - Handle camera perspecitives and rotations
 * 
 *  NOTE: No fancy shading or lighting
 *  NOTE: You need to configure the GPU correctly for everything to work! (see examples in later files)
*/
const std::string vert_shader_example =
OPENGL_ES_VERSON +
"uniform mat4 u_MVP;\n"       // model view matrix (MVP) is a matrix that will be applied to all vertecies, 
                              // can contain things like rotations or translations(moving objects in space)
"attribute vec3 position;\n"  // The position of the vertex(triangle corner)
"attribute vec4 color;\n"     // The color of the vertex(triangle corner)
"attribute vec2 aTexCoord;\n" // The coordinate on the texture the vertex maps to
                              // (if x corordinate is negative I treat it as it doesn't map to any texture)
"\n"
"varying vec4 v_Color;\n"    
"varying vec2 TexCoord;\n"   
"\n"
"void main() {\n"            
"    gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
"    v_Color = vec4(color.x, color.y, color.z, 1.0);\n"
"    TexCoord = aTexCoord;\n"
"}\n";

const std::string frag_shader_example =
OPENGL_ES_VERSON +
"precision mediump float;\n"
"varying vec4 v_Color;\n"
"varying vec2 TexCoord;\n"
"\n"
"uniform sampler2D ourTexture;\n"
"\n"
"void main() {\n"
"    if(TexCoord.x < 0.0) {\n"
"        gl_FragColor = v_Color;\n"
"    }\n"
"    else {\n"
"       gl_FragColor = texture2D(ourTexture, TexCoord);\n"
"    }\n"
"}\n";
}
}