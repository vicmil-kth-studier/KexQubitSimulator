#include "L4_setup.h"

namespace vicmil {
struct PixelRGB {
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;
    PixelRGB() {};
    PixelRGB(unsigned char r_, unsigned char g_, unsigned char b_) {
        r = r_;
        g = g_;
        b = b_;
    }
};

struct PixelRGBA {
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;
    unsigned char a = 0;
    PixelRGBA() {};
    PixelRGBA(unsigned char r_, unsigned char g_, unsigned char b_, unsigned char a_) {
        r = r_;
        g = g_;
        b = b_;
        a = a_;
    }
};

class RawImageRGB {
public:
    unsigned int width = 0;
    unsigned int height = 0;
    std::vector<PixelRGB> pixels = std::vector<PixelRGB>();

    RawImageRGB() {}
    RawImageRGB(std::vector<PixelRGB> pixels_, unsigned int width_, unsigned int height_) {
        width = width_;
        height = height_;
        pixels = pixels_;
        Assert(width*height == pixels.size());

        // Make sure width and height is a power of 2(otherwise it won't work)
        Assert(is_power_of_two(width));
        Assert(is_power_of_two(height));
    }
    unsigned char* get_pixel_data() {
        return (unsigned char*)((void*)(&pixels[0]));
    }
};

class RawImageRGBA {
public:
    unsigned int width = 0;
    unsigned int height = 0;
    std::vector<PixelRGBA> pixels = std::vector<PixelRGBA>();

    RawImageRGBA() {}
    RawImageRGBA(std::vector<PixelRGBA> pixels_, unsigned int width_, unsigned int height_) {
        width = width_;
        height = height_;
        pixels = pixels_;
        Assert(width*height == pixels.size());

        // Make sure width and height is a power of 2(otherwise it won't work)
        Assert(is_power_of_two(width));
        Assert(is_power_of_two(height));
    }
    unsigned char* get_pixel_data() {
        return (unsigned char*)((void*)(&pixels[0]));
    }
};

class Texture {
public:
    unsigned int renderedTexture;

    /**
     * Create a new texture on the GPU from an RGB image
     * @param raw_image The image with pixel data to use
     * @return A reference to the texture on the GPU
    */
    static Texture from_raw_image_rgb(RawImageRGB& raw_image) {
        Texture new_texture;
        glGenTextures(1, &new_texture.renderedTexture);

        // "Bind" the newly created texture : all future texture functions will modify this texture
        glBindTexture(GL_TEXTURE_2D, new_texture.renderedTexture);

        unsigned char* pixel_data = raw_image.get_pixel_data();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, raw_image.width, raw_image.height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixel_data);

        set_pixel_parameters_nearest(); // Set as default(if none set the image doesn't show)
        return new_texture;
    }

    /**
     * Create a new texture on the GPU from an RGBA image
     * @param raw_image The image with pixel data to use
     * @return A reference to the texture on the GPU
    */
    static Texture from_raw_image_rgba(RawImageRGBA& raw_image) {
        Texture new_texture;
        glGenTextures(1, &new_texture.renderedTexture);

        // "Bind" the newly created texture : all future texture functions will modify this texture
        glBindTexture(GL_TEXTURE_2D, new_texture.renderedTexture);

        unsigned char* pixel_data = raw_image.get_pixel_data();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, raw_image.width, raw_image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data);

        set_pixel_parameters_nearest(); // Set as default(if none set the image doesn't show)
        return new_texture;
    }

    /**
     * Bind the texture so it is that texture that is referenced when drawing
     * @return None
    */
    void bind() {
        glBindTexture(GL_TEXTURE_2D, renderedTexture);
    }

    /**
     * Set the mode to grab the nearest texture pixel, if texture pixels don't align with screen pixels
     * @return None
    */
    static void set_pixel_parameters_nearest() {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
    
    /**
     * Set the mode to interpolate between the nearest texture pixels, if texture pixels don't align with screen pixels
     * @return None
    */
    static void set_pixel_parameters_linear() {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
};
}