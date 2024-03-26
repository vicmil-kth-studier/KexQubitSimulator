#include "L3_gpu_buffer.h"

namespace vicmil {
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

/**
 * An image stored on the CPU, with all the pixel data
*/
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

        // Make sure width and height is a power of 2(otherwise OpenGL won't work)
        Assert(is_power_of_two(width));
        Assert(is_power_of_two(height));
    }
    void set_pixel(PixelRGBA p, int x, int y) {
        pixels[x + y * width] = p;
    }
    PixelRGBA get_pixel(int x, int y) {
        return pixels[x + y * width];
    }
    unsigned char* get_pixel_data() {
        return (unsigned char*)((void*)(&pixels[0]));
    }
    void to_png(std::string file_name) {
        Assert(width*height == pixels.size());
        SDL_Surface* new_surface = SDL_CreateRGBSurface(0,width,height,32,0,0,0,0);
        new_surface->pixels = get_pixel_data();
        IMG_SavePNG(new_surface, file_name.c_str());
    }
    static RawImageRGBA from_png(std::string file_name) {
        SDL_Surface* new_surface = IMG_Load(file_name.c_str());
        SDL_Surface* surface_with_format = SDL_ConvertSurfaceFormat(new_surface, SDL_PIXELFORMAT_RGBA8888, 0);
        RawImageRGBA new_raw_image;
        new_raw_image.width = surface_with_format->w;
        new_raw_image.height = surface_with_format->h;
        new_raw_image.pixels.reserve(new_raw_image.width*new_raw_image.height);
        ThrowNotImplemented();
        // Todo! Copy the surface image data to new_raw_image.pixels
    }
};



/**
 * A collage is a set of images that are put together, so that you don't have to have many small images
 *   This is especially important for the GPU since each call has A LOT of overhead.
*/
class CollageRGBA {
    public:
    RawImageRGBA _raw_image;
    std::vector<Rect> _image_rects; // Image rects of all sub-images
    /**
     * Fetch the texture coordinate based on the image index
    */
    Rect get_texture_coord(int index) {return _image_rects[index]; }
    /**
     * Get the full image of the collage
    */
    RawImageRGBA get_image() {return _raw_image; }
};

class CollageGeneratorRGBA {
    std::vector<RawImageRGBA> images;

    /**
     * Add an image into list of images
     * Returns the image index
    */
    int add_image(RawImageRGBA image) {
        images.push_back(image);
        return images.size() - 1;
    }

    // Compress all raw images into a single image
    CollageRGBA generate_collage() {
        // Uses naive approach, can be improved!
        int x_sum = 0;
        int max_y = 0;
        for(int i = 0; i < images.size(); i++) {
            x_sum += images[i].width;
            max_y = std::max((double)max_y, (double)images[i].height);
        }
        CollageRGBA new_collage;
        new_collage._raw_image.width = upper_power_of_two(x_sum);
        new_collage._raw_image.height = upper_power_of_two(max_y);
        new_collage._raw_image.pixels.resize(new_collage._raw_image.width*new_collage._raw_image.height);

        int x_index = 0;
        // Set all the pixels to the appropriate value
        for(int i = 0; i < images.size(); i++) {
            Rect rect;
            rect.x = x_index / (double)new_collage._raw_image.width;
            rect.y = 0;
            rect.w = images[i].width / (double)new_collage._raw_image.width;
            rect.h = images[i].height / (double)new_collage._raw_image.height;
            for(int x = 0; x < images[i].width; x++) {
                for(int y = 0; y < images[i].height; y++) {
                    PixelRGBA pixel = images[i].get_pixel(x, y);
                    new_collage._raw_image.set_pixel(pixel, x + x_index, y);
                }
            }
            x_index += images[i].width;
            new_collage._image_rects.push_back(rect);
        }
        return new_collage;
    }
};


/**
 * A texture lives on the GPU, and its data is not easily accessible
 * 
 * But with some OpenGL calls, data transfer is possible!
 * You can also render to a texture instead of the main window, but that is not implemented yet!
*/
class GPUTexture {
public:
    // The texture id, a reference to the GPU texture
    unsigned int renderedTexture;

    // Parameter to specify whether a texture has been set
    //  if you try to bind no texture, do nothing
    bool no_texture = true;

    /** Create a new texture on the GPU from an RGBA image on the CPU
     * @param raw_image The image with pixel data to use
     * @return A reference to the texture on the GPU
    */
    static GPUTexture from_raw_image_rgba(RawImageRGBA& raw_image) {
        GPUTexture new_texture;
        new_texture.no_texture = false;
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
        if(!no_texture) {
            glBindTexture(GL_TEXTURE_2D, renderedTexture);
        }
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