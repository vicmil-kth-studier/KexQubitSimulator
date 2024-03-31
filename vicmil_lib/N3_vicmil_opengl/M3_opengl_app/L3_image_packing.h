#pragma once
#include "L2_view_transform.h"

namespace vicmil {
struct RawImageRGBAPacking {
    RawImageRGBA packed_image;
    // where you can find the images(could probably use a vector, but then you have to deal with indexes etc. This is simpler to use and should not be a bottleneck)
    // (In pixel format, eg number of pixels)
    std::map<std::string, RectT<int>> image_pos;
    void _set_image_pixels(RawImageRGBA& image_, RectT<int> rect_) {
        Assert(image_.width == rect_.w);
        Assert(image_.height == rect_.h);
        Assert(image_.pixels.size() == rect_.w * rect_.h);
        Assert(rect_.min_x() >= 0);
        Assert(rect_.min_y() >= 0);
        Assert(rect_.max_x() < packed_image.width);
        Assert(rect_.max_y() < packed_image.height);
        Assert(packed_image.height*packed_image.width == packed_image.pixels.size());
        for(int x = 0; x < rect_.w; x++) {
            for(int y = 0; y < rect_.h; y++) {
                int packed_image_x = x + rect_.x;
                int packed_image_y = y + rect_.y;
                vicmil::PixelRGBA pixel = image_.get_pixel(x, y);
                packed_image.set_pixel(pixel, packed_image_x, packed_image_y);
            }
        }
    }
    RawImageRGBAPacking(std::map<std::string, RawImageRGBA>& images) {
        START_TRACE_FUNCTION();
        std::vector<std::string> indv_image_names = {};
        std::vector<RectT<int>> indv_images = {};
        auto it = images.begin();
        Debug("Setup image positions fetch");
        while(it != images.end()) {
            Debug("Get image name...");
            std::string image_name = (*it).first;
            DebugExpr(image_name);
            indv_image_names.push_back(image_name);

            RectT<int> image_rect = RectT<int>(0, 0, (*it).second.width, (*it).second.height);
            indv_images.push_back(image_rect);
            it++;
        }
        Debug("Fetch image positions");
        int size = vicmil::rect_packing_pow2_sq(indv_images);
        packed_image.width = size;
        packed_image.height = size;
        Debug("Resize pixels");
        packed_image.pixels.resize(size*size);

        Debug("Draw on texture");
        for (int i = 0; i < indv_images.size(); i++) {
            DebugExpr(indv_image_names[i]);
            RawImageRGBA& img_ref = images[indv_image_names[i]];
            _set_image_pixels(img_ref, indv_images[i]);
        }

        Debug("Set the new image positions");
        // Set the new image positions!
        for (int i = 0; i < indv_images.size(); i++) {
            image_pos[indv_image_names[i]] = indv_images[i];
        }
    }
    RectT<int> get_image_pos(std::string str) {
        return image_pos[str];
    }
};

struct TexturePacking {
    vicmil::GPUTexture packed_texture;
    // where you can find the images(could probably use a vector, but then you have to deal with indexes etc. This is simpler to use and should not be a bottleneck)
    // (In gpu texture format, eg positions between (0,0) and (1,1))
    std::map<std::string, Rect> image_pos;
    TexturePacking(RawImageRGBAPacking raw_image_packing) {
        packed_texture = vicmil::GPUTexture::from_raw_image_rgba(raw_image_packing.packed_image);
        auto it = raw_image_packing.image_pos.begin();
        while(it != raw_image_packing.image_pos.end()) {
            std::string image_name = (*it).first;

            RectT<int> image_rect = (*it).second;

            // Rescale image positions (to be between 0 and 1, eg OpenGL format)
            Rect tex_rect;
            tex_rect.x = image_rect.x / (double)raw_image_packing.packed_image.width;
            tex_rect.y = image_rect.y / (double)raw_image_packing.packed_image.height;
            tex_rect.w = image_rect.w / (double)raw_image_packing.packed_image.width;
            tex_rect.h = image_rect.h / (double)raw_image_packing.packed_image.height;
            image_pos[image_name] = tex_rect;
            it++;
        }
    }
};

RawImageRGBA _get_letter_image(char letter) {
    std::string letter_pixels = __alphabet__::mapping.find(letter).operator*().second;
    Assert(letter_pixels.size() == __alphabet__::LETTER_WIDTH * __alphabet__::LETTER_HEIGHT);

    RawImageRGBA return_image;
    return_image.width = __alphabet__::LETTER_WIDTH;
    return_image.height = __alphabet__::LETTER_HEIGHT;
    return_image.pixels.resize(return_image.width * return_image.height);
    
    // Populate the image
    for(int x = 0; x < __alphabet__::LETTER_WIDTH; x++) {
        for(int y = 0; y < __alphabet__::LETTER_HEIGHT; y++) {
            int pixel_index = x + y * __alphabet__::LETTER_WIDTH;
            bool empty_pixel = letter_pixels[pixel_index] == '.';
            if(empty_pixel) {
                return_image.set_pixel(vicmil::PixelRGBA(0, 0, 0, 0), x, y);
            }
            else {
                return_image.set_pixel(vicmil::PixelRGBA(255, 255, 255, 255), x, y);
            }
        }
    }
    return return_image;
}

/**
 * For all characters in the ASCII, return an image of it
 * TODO: Add all images in acsii, now only supports subset
*/ 
std::map<std::string, RawImageRGBA> get_ascii_images() {
    std::map<std::string, RawImageRGBA> return_map;
    auto it = __alphabet__::mapping.begin();
    while(it != __alphabet__::mapping.end()) {
        return_map[std::to_string((*it).first)] = _get_letter_image((*it).first);
        it++;
    }
    return return_map;
}

/** Help class for drawing text in any font, as long as it is mapped onto a texture
 *  Get the position of each letter
 *  Get the position and size of the entire text on screen
 * 
 * Requirements:
// Letters: 
// It should support unicode (using utf8)
// Each letter is drawn from the left
// Each letter is assumed to be the same height(the image of it)
// Each letter can differ in width
// Each letter is assumed to be scaled correctly on the texture
// Each \n letter will create a new line

// Positioning:
// It should support different line spacings between letters(vertical and horizontal)
// It should be able to align letters to left, middle or right
// It should support different letter heights (As longs as its the same for all letters)
// It should support different screen aspect ratios(and still have the letters look correct)
// It should support different positions to start drawing the text

TODO: Add text flags such as bold, italics, underline, font, text color, background color etc.
(These should be flags that you set, and then all following letters follows that convention)
*/ 
class TextDrawer {
public:
    std::string text = "Hello world!";
    double spacing_vertical = 0.01;
    double spacing_horizontal = 0.01;
    double text_position_x = 0;
    double text_position_y = 0;
    double letter_height = 0.5;
    double screen_aspect_ratio = 1; // (Width / Height)

    // Where each letter can be found on the texture(image) that has all the letters
    std::map<std::string, Rect> letter_texture_mapping = {};

    double get_letter_width(std::string letter) {
        if(letter_texture_mapping.count(letter) != 0) {
            Rect tex_coord = letter_texture_mapping[letter];
            double width = (tex_coord.h / letter_height) * tex_coord.w * screen_aspect_ratio;
            return width;
        }
        return 0;
    }

    double get_line_width(std::string line_text) {
        int index = 0;
        double width = 0;
        while(index < line_text.size()) {
            std::string next_letter = get_next_utf8_char(line_text, index);
            Assert(next_letter != "\n");
            if(width != 0) {
                width += spacing_horizontal;
            }
            width += get_letter_width(next_letter);
        }
        return width;
    }

    // Get the minimum rectangle on screen that fits all the text
    Rect get_screen_position() {
        std::vector<std::string> lines = split_string(text, '\n');
        double max_line_width = 0;
        for(int i = 0; i < lines.size(); i++) {
            double width = get_line_width(lines[i]);
            max_line_width = std::max(max_line_width, width);
        }

        double text_width = max_line_width;
        double text_height = lines.size() * (letter_height + spacing_vertical) - spacing_vertical;
        return Rect(text_position_x, text_position_y, text_width, text_height);
    }

    // Get the position of each individual letter(Fetch the texture positions from the mapping)
    struct LetterPos {
        std::string letter;
        Rect screen_pos;
        LetterPos(std::string letter_, Rect screen_pos_) {
            letter = letter_;
            screen_pos = screen_pos_;
        }
    };
    void _add_letter_pos_of_line(std::string& line_text, int line_nr, std::vector<LetterPos>* letter_positions) {
        int y = line_nr * (letter_height + spacing_vertical) - spacing_vertical;
        int x = 0;
        int index = 0;
        // Iterate through all letters in each line
        while(index < line_text.size()) {
            std::string next_letter = get_next_utf8_char(line_text, index);
            double width = get_letter_width(next_letter);
            // Append the letter position
            letter_positions->push_back(LetterPos(next_letter, Rect(x, y, width, letter_height)));

            // Move x-position for the next letter
            x += width + spacing_horizontal;
        }
    }
    /**
     * Get the position on screen of each individual letter
    */
    std::vector<LetterPos> get_letter_positions() {
        std::vector<LetterPos> return_vec;

        // Iterate through all the lines
        std::vector<std::string> lines = split_string(text, '\n');
        for(int i = 0; i < lines.size(); i++) {
            _add_letter_pos_of_line(lines[i], i, &return_vec);
        }
        return return_vec;
    }
};

}
