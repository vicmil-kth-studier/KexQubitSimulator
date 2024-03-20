/*
Load .obj and .mtl files
*/ 
#include "L5_texture.h"
#include <map>

namespace vicmil {
struct FVec3 {
public:
    float v[3]; // x, y, z
    static FVec3 from_str(std::string v1, std::string v2, std::string v3) {
        FVec3 new_vec;
        new_vec.v[0] = std::stof(v1);
        new_vec.v[1] = std::stof(v2);
        new_vec.v[2] = std::stof(v3);
        return new_vec;
    }
};

struct FVec2 {
    float v[2]; // x, y, z
    static FVec2 from_str(std::string v1, std::string v2) {
        FVec2 new_vec;
        new_vec.v[0] = std::stof(v1);
        new_vec.v[1] = std::stof(v2);
        return new_vec;
    }
};

struct IVec3 {
    int v[3];
    static IVec3 from_str(std::string v1, std::string v2, std::string v3) {
        IVec3 new_vec;
        new_vec.v[0] = std::stoi(v1);
        new_vec.v[1] = std::stoi(v2);
        new_vec.v[2] = std::stoi(v3);
        return new_vec;
    }
};

struct Vertex {
    FVec3 v;
    static Vertex from_obj_file_line(std::vector<std::string> args) {
        expect_vec_length(args, 4, "[Parse Error] [Wrong number of args]");
        Vertex new_vertex;
        new_vertex.v = FVec3::from_str(args[1], args[2], args[3]);
        return new_vertex;
    }
};

struct TextureCoordinate {
    FVec2 v;
    static TextureCoordinate from_obj_file_line(std::vector<std::string> args) {
        expect_vec_length(args, 3, "[Parse Error] [Wrong number of args]");
        TextureCoordinate new_tex_coord;
        new_tex_coord.v = FVec2::from_str(args[1], args[2]);
        return new_tex_coord;
    }
};

struct Normal {
    FVec3 v;
    static Normal from_obj_file_line(std::vector<std::string> args) {
        expect_vec_length(args, 4, "[Parse Error] [Wrong number of args]");
        Normal new_normal;
        new_normal.v = FVec3::from_str(args[1], args[2], args[3]);
        return new_normal;
    }
};

struct Surface {
    IVec3 vertex_index;
    IVec3 tex_coordinate_index; // Defaults to 0
    IVec3 normal_index;
    static Surface from_obj_file_line(std::vector<std::string> args) {
        expect_vec_length(args, 16, "[Parse Error] [Wrong number of args]");
        Surface new_surface;
        new_surface.vertex_index =         IVec3::from_str(args[1], args[1+5], args[1+5*2]);
        new_surface.tex_coordinate_index = IVec3::from_str(args[3], args[3+5], args[3+5*2]);
        new_surface.normal_index =         IVec3::from_str(args[5], args[5+5], args[5+5*2]);
        return new_surface;
    }
};

struct Color {
    FVec3 v; // [r, g, b], values between 0.0 - 1.0
    static Color from_obj_file_line(std::vector<std::string> args) {
        expect_vec_length(args, 4, "[Parse Error] [Wrong number of args]");
        Color new_color;
        new_color.v = FVec3::from_str(args[1], args[2], args[3]);
        return new_color;
    }
};

struct Material {
    Color color;
    std::string texture; // Provide with texture...
};

static void remove_hashtag_comments(std::vector<std::string>& file_lines) {
    for(int i = 0; i < file_lines.size(); i++) {
        file_lines[i] = cut_off_after_find(file_lines[i], "#");
    }
}

static std::vector<std::string> split_obj_file_line_into_args(std::string line_str) {
    return regex_find_all(line_str, "[a-zA-Z0-9_.\\-]+|[/]");
}

class ObjFileContents {
public:
    std::vector<Vertex> verticies;
    std::vector<Normal> normals;
    std::vector<TextureCoordinate> texture_coordinates;
    std::map<std::string, std::vector<Surface>> surfaces; // [material name, surface]
    std::vector<std::string> mtl_files;
    std::string _active_material = "Undefined"; // Used when loading file to store state

    void use_material(std::string material_name) {
        this->_active_material = material_name;
        if(this->surfaces.count(this->_active_material) == 0) {
            this->surfaces[this->_active_material] = std::vector<Surface>();
        }
    }

    void parse_obj_line(std::vector<std::string> args) {
        if(args[0] == "mtllib") {
            expect_vec_length(args, 2, "[Parse Error] [Wrong number of args]");
            mtl_files.push_back(args[1]);
            return;
        }   

        if(args[0] == "v") { // Load vertex
            Vertex new_vertex = Vertex::from_obj_file_line(args);
            this->verticies.push_back(new_vertex);
            return;
        }   

        if(args[0] == "vn") { // Load normal
            Normal new_normal = Normal::from_obj_file_line(args);
            this->normals.push_back(new_normal);
            return;
        }   

        if(args[0] == "usemtl") { // Set active material to use
            expect_vec_length(args, 2, "[Parse Error] [Wrong number of args]");
            use_material(args[1]);
            return;
        }   

        if(args[0] == "vt") { // Load texture coordinate
            TextureCoordinate new_tex_coord = TextureCoordinate::from_obj_file_line(args);
            this->texture_coordinates.push_back(new_tex_coord);
            return;
        }   

        if(args[0] == "f") { // Load face
            Surface new_surface = Surface::from_obj_file_line(args);
            this->surfaces[this->_active_material].push_back(new_surface); // Associate each surface with material
            return;
        }   

        // Ignore the following arguments
        std::vector<std::string> ignore_args = {"o", "g", "s", "l"};
        if (in_vector(args[0], ignore_args)) {
            return;
        }   

        ThrowError("[Parse Error] Unknown Argument: " << args[0]);
        throw;
    }
    static ObjFileContents from_file_contents(std::vector<std::string>& file_contents_lines) {
        DisableLogging;
        ObjFileContents file_contents = ObjFileContents();
        remove_hashtag_comments(file_contents_lines);

        // Handle contents
        for(int i = 0; i < file_contents_lines.size(); i++) {
            // Split line into its elements
            std::vector<std::string> line_tokens = split_obj_file_line_into_args(file_contents_lines[i]);
            if(line_tokens.size() == 0) {
                continue;
            }
            Debug(file_contents_lines[i]);
            // Parse the line
            file_contents.parse_obj_line(line_tokens);
        }
        return file_contents;
    }
    static ObjFileContents from_file(std::string filepath) {
        std::vector<std::string> file_contents_lines = read_file_contents_line_by_line(filepath);
        return from_file_contents(file_contents_lines);
    }
};

class MtlFileContents {
public:
    std::map<std::string, Material> materials; // [material name, material info]
    std::string _active_material = "Undefined"; // Used to store state when loading file

    void new_material(std::string material_name) {
        this->_active_material = material_name;
        if(this->materials.count(this->_active_material) != 0) {
            ThrowError("material already defined: " << material_name);
        }
        this->materials[this->_active_material] = Material();
    }

    void parse_mtl_line(std::vector<std::string> args) {
        if(args[0] == "newmtl") { // Set active material to use
            expect_vec_length(args, 2, "[Parse Error] [Wrong number of args]");
            this->new_material(args[1]);
            return;
        }   

        if(args[0] == "Kd") { // Set material color
            this->materials[this->_active_material].color = Color::from_obj_file_line(args);
            return;
        }   

        // Ignore the following arguments
        std::vector<std::string> ignore_args = {"Ns","Ka","Ks","Ke","Ks","Ni","d","illum"};
        if (in_vector(args[0], ignore_args)) {
            return;
        }   

        ThrowError("[Parse Error] Unknown Argument: " << args[0]);
        throw;
    }

    static MtlFileContents from_file_contents(std::vector<std::string>& file_contents_lines) {
        MtlFileContents new_content = MtlFileContents();
        remove_hashtag_comments(file_contents_lines);

        // Handle contents
        for(int i = 0; i < file_contents_lines.size(); i++) {
            // Split line into its elements
            std::vector<std::string> args = split_obj_file_line_into_args(file_contents_lines[i]);
            if(args.size() == 0) {
                continue;
            }
            // Parse the line
            new_content.parse_mtl_line(args);
        }
        return new_content;
    }
    static MtlFileContents from_file(std::string filepath) {
        DisableLogging;
        Debug("loading mtl file: " << filepath);

        std::vector<std::string> file_contents_lines = read_file_contents_line_by_line(filepath);
        return from_file_contents(file_contents_lines);
    }
};

class ModelsInfo {
    public:
    ObjFileContents obj_file_contents;
    std::vector<MtlFileContents> mtl_files_content;

    static ModelsInfo from_obj_file(std::string filepath) { // .mtl files are assumed to be in same directory as .obj file
        DisableLogging;
        ModelsInfo new_model = ModelsInfo();
        std::string root_path = cut_off_after_rfind(filepath, "/") + "/";

        // Load obj file
        new_model.obj_file_contents = ObjFileContents::from_file(filepath);

        Debug("Load MTL file");
        // Load associated mtl files (These contains material properties)
        for(int i = 0; i < new_model.obj_file_contents.mtl_files.size(); i++) {
            std::string mtl_file_name = new_model.obj_file_contents.mtl_files[i];
            MtlFileContents file_content = MtlFileContents::from_file(root_path + mtl_file_name);
            new_model.mtl_files_content.push_back(file_content);
        }
        return new_model;
    };

    Material get_material(std::string material_name) {
        // Search through all mtl files to find material
        for(int i = 0; i < this->mtl_files_content.size(); i++) {
            if(this->mtl_files_content[i].materials.count(material_name) != 0) {
                return this->mtl_files_content[i].materials[material_name];
            }
        }
        ThrowError("Could not find material ");
    }

    void print_contents() {
        DisableLogging;
        Debug("print model info contents");
        for (auto i = this->obj_file_contents.surfaces.begin(); i != this->obj_file_contents.surfaces.end(); ++i) {
            std::string material_name = i->first;
            std::vector<Surface>& surfaces = i->second;
            Debug(material_name);

            Material material = this->get_material(material_name);

            for(int i2 = 0; i2 < surfaces.size(); i2++) {
                Debug("new surface");
                Surface& surface = surfaces[i2];

                Debug("color: (" << material.color.v.v[0] << ", " << material.color.v.v[1] << ", " << material.color.v.v[2] << ")"); 
                for(int i3 = 0; i3 < 3; i3++) {
                    Vertex& vertex = this->obj_file_contents.verticies[surface.vertex_index.v[i3]];
                    Debug("vertex: (" << vertex.v.v[0] << ", " << vertex.v.v[1] << ", " << vertex.v.v[2] << ")"); 
                }

                Debug("");
            }
        }
    }
};
}
