#include "L7_perspective.h"

namespace vicmil {
static void clear_screen() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

static void set_depth_testing_enabled(bool is_enabled) {
    // This makes sure that things closer block things further away
    if(is_enabled) {
        // Enable depth buffer
        glEnable(GL_DEPTH_TEST);
    }
    else {
        glDisable(GL_DEPTH_TEST);
    }
}

struct ColorTriangleVertex {
    float vertex[3];
    float color[3];
};

struct TextureTriangleVertex {
    float vertex[3];
    float color[3];
    float tex_coord[2];
    TextureTriangleVertex() {}
    TextureTriangleVertex(float v1, float v2, float v3, float tex_x, float tex_y) {
        vertex[0] = v1;
        vertex[1] = v2;
        vertex[2] = v3;
        tex_coord[0] = tex_x;
        tex_coord[1] = tex_y;

        // Just set default values for color
        color[0] = 1.0f;
        color[1] = 1.0f;
        color[2] = 1.0f;
    }
};

struct TraingleIndecies {
    unsigned int index[3];
    TraingleIndecies() {}
    TraingleIndecies(unsigned int i1, unsigned int i2, unsigned int i3) {
        index[0] = i1;
        index[1] = i2;
        index[2] = i3;
    }
};

class Drawable3DModel {
public:
    std::vector<ColorTriangleVertex> _color_triangle_vertecies;
    std::vector<TraingleIndecies> _triangle_indecies;
    static Drawable3DModel from_models_info(ModelsInfo* models_info) {
        Drawable3DModel new_drawable_object = Drawable3DModel();
        for (auto i = models_info->obj_file_contents.surfaces.begin(); i != models_info->obj_file_contents.surfaces.end(); ++i) {
            std::string material_name = i->first;
            std::vector<Surface>& surfaces = i->second;

            Material material = models_info->get_material(material_name);

            for(int i2 = 0; i2 < surfaces.size(); i2++) {
                TraingleIndecies new_surface;
                new_surface.index[0] = new_drawable_object._triangle_indecies.size() * 3;
                new_surface.index[1] = new_surface.index[0] + 1;
                new_surface.index[2] = new_surface.index[0] + 2;

                new_drawable_object._triangle_indecies.push_back(new_surface);
                
                Surface& surface = surfaces[i2];
                Color& color = material.color;
                for(int i3 = 0; i3 < 3; i3++) {
                    ColorTriangleVertex new_vertex;
                    new_vertex.color[0] = color.v.v[0];
                    new_vertex.color[1] = color.v.v[1];
                    new_vertex.color[2] = color.v.v[2];

                    Vertex& vertex = models_info->obj_file_contents.verticies[surface.vertex_index.v[i3]-1];
                    new_vertex.vertex[0] = vertex.v.v[0];
                    new_vertex.vertex[1] = vertex.v.v[1];
                    new_vertex.vertex[2] = vertex.v.v[2];
                    new_drawable_object._color_triangle_vertecies.push_back(new_vertex);
                }
            }
        }
        return new_drawable_object;
    }
    IndexVertexBufferPair create_IndexVertexBufferPair() {
        IndexVertexBufferPair new_buffer_pair;
        new_buffer_pair.vertex_buffer = GLBuffer::generate_buffer(sizeof(ColorTriangleVertex) * this->_color_triangle_vertecies.size(), &this->_color_triangle_vertecies[0], GL_ARRAY_BUFFER);
        new_buffer_pair.index_buffer = GLBuffer::generate_buffer(sizeof(TraingleIndecies) * this->_triangle_indecies.size(), &this->_triangle_indecies[0], GL_ELEMENT_ARRAY_BUFFER);
        return new_buffer_pair;
    }
    void print_content() {
        for(int i = 0; i < _triangle_indecies.size(); i++) {
            for(int i2 = 0; i2 < 3; i2++) {
                auto vertex = _color_triangle_vertecies[_triangle_indecies[i].index[i2]];
                std::cout << "vert " << vertex.vertex[0] << " " << vertex.vertex[1] << " " << vertex.vertex[2] << std::endl;
            }
            std::cout << std::endl;
        }
    }
};

/**
 * Merge multiple objects into one vertex and index buffer
*/
class Shared3DModelsBuffer {
public:
    IndexVertexBufferPair buffers;
    std::vector<unsigned int> index_buffer_obj_offset;
    std::vector<unsigned int> index_buffer_obj_size;
    Shared3DModelsBuffer() {}
    Shared3DModelsBuffer(std::vector<Drawable3DModel> objects) {
        std::vector<ColorTriangleVertex> vertecies = std::vector<ColorTriangleVertex>();
        std::vector<TraingleIndecies> indecies = std::vector<TraingleIndecies>();
        unsigned int vertex_offset = 0;
        unsigned int index_offset = 0;
        for(unsigned int i = 0; i < objects.size(); i++) {
            // Push new objects vertecies and indecies
            vertecies.insert(vertecies.end(), objects[i]._color_triangle_vertecies.begin(), objects[i]._color_triangle_vertecies.end());
            indecies.insert(indecies.end(), objects[i]._triangle_indecies.begin(), objects[i]._triangle_indecies.end());

            // Update index positions
            for(unsigned int i2 = 0; i2 < objects[i]._triangle_indecies.size(); i2++) {
                indecies[i2 + index_offset].index[0] += vertex_offset;
                indecies[i2 + index_offset].index[1] += vertex_offset;
                indecies[i2 + index_offset].index[2] += vertex_offset;
            }

            index_buffer_obj_offset.push_back(index_offset);
            index_buffer_obj_size.push_back(objects[i]._triangle_indecies.size());
            vertex_offset += objects[i]._color_triangle_vertecies.size();
            index_offset += objects[i]._triangle_indecies.size();
        };

        buffers = IndexVertexBufferPair::from_raw_data(
            &indecies[0], 
            sizeof(TraingleIndecies) * indecies.size(), 
            &vertecies[0],
            sizeof(ColorTriangleVertex) * vertecies.size()
        );
    }
    static Shared3DModelsBuffer from_models(std::vector<Drawable3DModel> objects) {
        Shared3DModelsBuffer new_obj_drawer = Shared3DModelsBuffer(objects);
        return new_obj_drawer;
    }
    static Shared3DModelsBuffer from_files(std::vector<std::string> filenames) {
        std::vector<Drawable3DModel> objs;
        for(int i = 0; i < filenames.size(); i++) {
            ModelsInfo new_model = ModelsInfo::from_obj_file(filenames[i]);
            auto new_drawable_object = Drawable3DModel::from_models_info(&new_model);
            objs.push_back(new_drawable_object);
        }
        return Shared3DModelsBuffer::from_models(objs);
    }
    void draw_object(unsigned int model_index, glm::mat4 matrix_mvp, GPUProgram* program) {
        buffers.bind();
        buffers.set_vertex_buffer_layout();
        UniformBuffer::set_mat4f(matrix_mvp, *program, "u_MVP");
        unsigned int offset = index_buffer_obj_offset[model_index] * sizeof(TraingleIndecies);
        buffers.draw(index_buffer_obj_size[model_index], offset);
    }
    void bind() {
        buffers.bind();
        buffers.set_vertex_buffer_layout();
    }
};






struct TextureTraingles {
    std::vector<TextureTriangleVertex> vertices = {};
    std::vector<TraingleIndecies> indicies = {};
    TextureTraingles(const std::vector<TextureTriangleVertex>& vertices_, const std::vector<TraingleIndecies>& indicies_) {
        vertices = vertices_;
        indicies = indicies_;
    }
    inline static TextureTraingles merge(const std::vector<TextureTraingles> texture_triangles) {
        TextureTraingles new_texture_triangles = TextureTraingles(texture_triangles[0].vertices, texture_triangles[0].indicies);
        int vertex_count = texture_triangles[0].vertices.size();
        int index_count = texture_triangles[0].indicies.size();
        for(int t = 1; t < texture_triangles.size(); t++) {
            new_texture_triangles.vertices.insert(new_texture_triangles.vertices.end(), texture_triangles[t].vertices.begin(), texture_triangles[t].vertices.end());
            new_texture_triangles.indicies.insert(new_texture_triangles.indicies.end(), texture_triangles[t].indicies.begin(), texture_triangles[t].indicies.end());
            for(int i = index_count; i < new_texture_triangles.indicies.size(); i++) {
                new_texture_triangles.indicies[i].index[0] += vertex_count;
                new_texture_triangles.indicies[i].index[1] += vertex_count;
                new_texture_triangles.indicies[i].index[2] += vertex_count;
            }
            vertex_count += texture_triangles[t].vertices.size();
            index_count += texture_triangles[t].indicies.size();
        }
        return new_texture_triangles;
    }
    void overwrite_index_vertex_buffer_pair(IndexVertexBufferPair& buffer_pair) {
        buffer_pair.overwrite_data(
            (void*)(&indicies[0]), 
            indicies.size() * sizeof(TraingleIndecies), 
            (void*)(&vertices[0]), 
            vertices.size() * sizeof(TextureTriangleVertex));
    }
    IndexVertexBufferPair create_index_vertex_buffer_pair() {
        IndexVertexBufferPair new_buffer_pair = IndexVertexBufferPair::from_raw_data(
            (void*)(&indicies[0]), 
            indicies.size() * sizeof(TraingleIndecies), 
            (void*)(&vertices[0]), 
            vertices.size() * sizeof(TextureTriangleVertex));

        return new_buffer_pair;
    }
};
}