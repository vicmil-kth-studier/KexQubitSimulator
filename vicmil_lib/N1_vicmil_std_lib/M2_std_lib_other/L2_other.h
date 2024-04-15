#include "L1_user_interface.h"

namespace vicmil {
    /**
     * Have a class that can store a pointer to a void function.
     * Great for storing function pointers
    */ 
    class VoidFuncRef {
        void_function_type _func;
        bool _has_function_pointer = false;
    public:
        VoidFuncRef() :
            _has_function_pointer(false) {}
        VoidFuncRef(void_function_type func) :
            _func(func) {
                _has_function_pointer = true;
        }
        /** Try calling the void function
         * @return 0 if void function was succefully called, 1 if function pointer was not set
        */
        int try_call() {
            if(_has_function_pointer) {
                _func();
                return 0;
            }

            Debug("Function not set!");
            return 1;
        }
        /**
         * Call the void function, throw error if it does not exist
        */
        void call() {
            if(try_call() != 0) {
                ThrowError("Function not set!");
            }
        }
    };

    // Counts the number of class instances and assigns each instance a unique id
    class ClassInstanceCounter {
        static int _get_instance_count(bool inc) {
            static int instance_count = 0;
            if(inc) {
                instance_count += 1;
            }
            return instance_count;
        }
        int instance_id = _get_instance_count(true);
    public:
        static int get_instance_count() {
            return _get_instance_count(false);
        }
        int get_instance_id() {
            return instance_id;
        }
    };

    /**
     * Create an unique color from an index
     * Tries to space out the colors as much as possible(subsequent colors are always very spaced out)
    */
    void unique_color(unsigned int index, unsigned char& r_, unsigned char& g_, unsigned char& b_) {
        index+=1; // To avoid black
        int base = 0;
        double divide = 2;
        double r = 0;
        double g = 0;
        double b = 0;
        while((index >> base) >= 1) {
            r += ((index >> (base+0))&1) / divide;
            g += ((index >> (base+1))&1) / divide;
            b += ((index >> (base+2))&1) / divide;
            base += 3;
            divide = divide * 2;
        }
        r_ = r*255;
        g_ = g*255;
        b_ = b*255;
    }

    // Returns colors based on index(r, b, g between 0 and 255)
    // Takes in index as an argument
    // Returns r, g, b
    struct ColorRGB888 {
        unsigned char r;
        unsigned char g;
        unsigned char b;
        ColorRGB888(){}
        ColorRGB888(int r_, int g_, int b_) {
            r = r_;
            g = g_;
            b = b_;
        }
        std::string to_string() {
            return vicmil::vec_to_str<int>({r, g, b});
        }
        ColorRGB888(int color_index) {
            unique_color(color_index, r, g, b);
        }
    };
    void TEST_unique_color() {
        std::set<std::vector<int>> unique_colors = {};
        for(int i = 0; i < 100; i++) {
            ColorRGB888 new_color = ColorRGB888(i);
            PrintExpr(i << ": " << new_color.to_string());
            std::vector<int> color_vec = {new_color.r, new_color.g, new_color.b};
            Assert(unique_colors.count(color_vec) == 0);
            unique_colors.insert(color_vec);
        }
        //Assert(false);
    }
    AddTest(TEST_unique_color);
}