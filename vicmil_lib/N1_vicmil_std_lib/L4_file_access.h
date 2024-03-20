#include "L3_string.h"
#include <fstream>
#include <sstream>

namespace vicmil {

std::string read_file_contents(const std::string& filename) {
    std::ifstream file(filename);
    std::string contents;

    if (file.is_open()) {
        // Read the file content into the 'contents' string.
        std::string line;
        while (std::getline(file, line)) {
            contents += line + "\n"; // Add each line to the contents with a newline.
        }
        file.close();
    } else {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
    }

    return contents;
}

std::vector<std::string> read_file_contents_line_by_line(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<std::string> contents;

    if (file.is_open()) {
        // Read the file content into the 'contents' string.
        std::string line;
        while (std::getline(file, line)) {
            contents.push_back(line); // Add each line to the contents with a newline.
        }
        file.close();
    } else {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
    }

    return contents;
}

class FileManager {
    std::string filename;
public:
    std::fstream file; // Read & Write
    //std::ifstream file; // Read only
    //std::ofstream file; // Write only
    FileManager(std::string filename_, bool create_file=false) { // class constructor
        filename = filename_;
        if(create_file) {
            file = std::fstream(filename, std::ios::app | std::ios::binary);
        }
        else {
            file = std::fstream(filename, std::ios::in | std::ios::binary | std::ios::out);
        }
        
        if(!file_is_open()) {
            std::cout << "file could not open!" << std::endl;
            std::cout << filename << std::endl;
            throw;
        }
        else {
            //std::cout << "file opened successfully" << std::endl;
        }
        }
    bool file_is_open() {
        return file.is_open();
    }

    void set_read_write_position(unsigned int index) {
        file.seekg(index);
    }
    unsigned int get_read_write_position() {
        // get current read position
        std::streampos read_pos = file.tellg();
        return read_pos;
    }

    std::vector<char> read_bytes(unsigned int read_size_in_bytes) { // read bytes as binary and write into &output
        std::vector<char> output = std::vector<char>();
        output.resize(read_size_in_bytes);
        file.read(&output[0], read_size_in_bytes); // read this many bytes from read_write_position in file, to &output[0] in memory.
        return output;
    }
    void write_bytes(std::vector<char> input) {
        file.write(&input[0], input.size());
        // write this many bytes from &input[0] in memory, to read_write_position in file.
        // also increments read_write_position by the same amount of bytes.
    }

    std::string read_str(unsigned int read_size_in_bytes) {
        std::string output = std::string();
        output.resize(read_size_in_bytes);
        file.read(&output[0], read_size_in_bytes);
        return output;
    }
    void write_str(std::string& str) {
        file.write(&str[0], str.length());
    }

    void write_int32(int val) {
        int* val_pointer = &val;
        char* char_pointer = reinterpret_cast<char*>(val_pointer); // convert int* to char* without changing position
        std::vector<char> bytes = {char_pointer[0], char_pointer[1], char_pointer[2], char_pointer[3]};
        write_bytes(bytes);
    }
    int read_int32() {
        std::vector<char> bytes = read_bytes(4);
        char* char_pointer = &bytes[0];
        int* val_pointer = reinterpret_cast<int*>(char_pointer);
        return *val_pointer;
    }

    unsigned int read_uint32() {
        std::vector<char> bytes = read_bytes(4);
        char* char_pointer = &bytes[0];
        unsigned int* val_pointer = reinterpret_cast<unsigned int*>(char_pointer);
        return *val_pointer;
    }

    unsigned char read_uint8() {
        std::vector<char> bytes = read_bytes(1);
        char* char_pointer = &bytes[0];
        unsigned char* val_pointer = reinterpret_cast<unsigned char*>(char_pointer);
        return *val_pointer;
    }

    char read_int8() {
        std::vector<char> bytes = read_bytes(1);
        char* char_pointer = &bytes[0];
        char* val_pointer = reinterpret_cast<char*>(char_pointer);
        return *val_pointer;
    }

    std::string read_word() {
        std::string output = std::string();
        file >> output;
        return output;
    }
    std::string read_next_line() {
        std::string output = std::string();
        std::getline(file, output, '\n');
        return output;
    }
    bool end_of_file() {
        return file.eof();
    }
    void erase_file_contents() {
        file.close();
        file.open(filename, std::ios::trunc | std::ios::out | std::ios::in | std::ios::binary);
        if(!file_is_open()) {
            std::cout << "file could not open!" << std::endl;
            throw;
        }
        else {
            //std::cout << "file opened successfully" << std::endl;
        }
    }
    // NOTE! This will move the read/write position
    unsigned int get_file_size() {
        file.seekg( 0, std::ios::end );
        return get_read_write_position();
    }
};

namespace json {
    enum json_type_index {
        DICT = 0,
        STR,
        DOUBLE_VEC,
        DOUBLE,
    };

    std::string get_indent_spaces(int indent) {
        std::string return_str;
        for(int i = 0; i < indent; i++) {
            return_str += "    ";
        }
        return return_str;
    }

    class Json {
        std::map<std::string, class Json> _dict = std::map<std::string, Json>();
        std::string _str;
        std::vector<double> _double_vec;
        double _double;

        json_type_index _type = json_type_index::DICT;

        std::string _to_string(int indent_) {
            if(_type == json_type_index::DICT) {
                std::string return_str = "{\n";
                for (auto i = _dict.begin(); i != _dict.end(); ++i) {
                    if(i != _dict.begin()) {
                        return_str += ",\n";
                    }
                    return_str += get_indent_spaces(indent_ + 1);
                    return_str += "\"" + i->first + "\":";
                    return_str += i->second._to_string(indent_+1);
                }
                return_str += "\n" + get_indent_spaces(indent_) + "}";
                return return_str;
            }
            if(_type == json_type_index::STR) {
                return "\"" +  _str + "\"";
            }
            if(_type == json_type_index::DOUBLE_VEC) {
                if(_double_vec.size() == 0) {
                    return "[]";
                }
                std::string return_str = "[\n";
                for(int i = 0; i < _double_vec.size(); i++) {
                    if(i != 0) {
                        return_str += ",\n";
                    }
                    return_str += get_indent_spaces(indent_+1);
                    return_str += std::to_string(_double_vec[i]);
                }
                return_str += "\n";
                return_str += get_indent_spaces(indent_) + "]";
                return return_str;
            }
            ThrowNotImplemented();
        }

    public:
        Json& operator=(std::string other) {
            _str = other;
            _type = json_type_index::STR;
            return *this;
        }
        Json& operator=(std::vector<double> other) {
            _double_vec = other;
            _type = json_type_index::DOUBLE_VEC;
            return *this;
        }
        std::string to_string() {
            return _to_string(0);
        }

        Json& operator[](std::string key) {
            if(_type != json_type_index::DICT) {
                ThrowError("Cannot index wrong json type!");
            }
            if (_dict.find(key) == _dict.end()) {
                _dict[key] = Json();
            }
            return _dict[key];
            ThrowNotImplemented();
        }
    };
}
}