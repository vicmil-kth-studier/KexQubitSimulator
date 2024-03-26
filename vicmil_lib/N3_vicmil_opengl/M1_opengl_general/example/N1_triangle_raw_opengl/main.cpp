#define USE_DEBUG
#define DEBUG_KEYWORDS "." 
#include "../../L1_gl_error.h"

static void clear_screen() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

class Setup {
public:
    SDL_Renderer *renderer = NULL;
    SDL_Window *window = NULL;
    SDL_GLContext gl_context;

    Setup() {}
    static Setup create_setup() {
        Setup setup = Setup();
        setup.create_window_and_renderer();
        //setup.setup_frag_shader_blending();
        return setup;
    }
    static void setup_frag_shader_blending() {
        // Enable blending, which makes textures look less blocky
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
    }
    void create_window_and_renderer() {
        /* Create a windowed mode window and its OpenGL context */
        SDL_CreateWindowAndRenderer(1024, 1024, SDL_WINDOW_OPENGL, &window, &renderer);

        Debug("OpenGL version " << glGetString(GL_VERSION));
    }
    ~Setup() {
    }
};



static bool quitting = false;
static Setup setup;

// Shader sources
const GLchar* vertexSource =
    "#version 130\n"
    "attribute vec4 position;    \n"
    "void main()                  \n"
    "{                            \n"
    "   gl_Position = vec4(position.xyz, 1.0);  \n"
    "}                            \n";

const GLchar* fragmentSource =
    "#version 130\n"
    "precision mediump float;\n"
    "void main()                                  \n"
    "{                                            \n"
    "  gl_FragColor = vec4 (1.0, 1.0, 1.0, 1.0 ); \n"
    "}                                            \n";

void render() {
    START_TRACE_FUNCTION();
    SDL_GL_MakeCurrent(setup.window, setup.gl_context);
    clear_screen();

    // Draw a triangle from the 3 vertices
    glDrawArrays(GL_TRIANGLES, 0, 3);

    SDL_GL_SwapWindow(setup.window);
    END_TRACE_FUNCTION();
}

void update(){
    SDL_Event event;
    while( SDL_PollEvent(&event) ) {
        if(event.type == SDL_QUIT) {
            throw;
        }
    }

    render();
};

int main(int argc, char *argv[]) {
    Debug("Start!");
    setup = Setup::create_setup();

    // Create Vertex Array Object
    GLuint vao;
    glGenVertexArraysOES(1, &vao);
    glBindVertexArrayOES(vao);

    // Create a Vertex Buffer Object and copy the vertex data to it
    GLuint vbo;
    glGenBuffers(1, &vbo);

    GLfloat vertices[] = {0.0f, 0.5f, 0.5f, -0.5f, -0.5f, -0.5f};

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Create and compile the vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    // Create and compile the fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    // Link the vertex and fragment shader into a shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    // glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    // Specify the layout of the vertex data
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // register update as callback
    while(true) {
        update();
    }
}