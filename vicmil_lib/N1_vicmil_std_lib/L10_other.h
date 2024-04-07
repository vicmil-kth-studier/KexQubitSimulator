#include "L9_user_interface.h"
#include <random>

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

    /**
     * Generate all kinds of random numbers, from floats to integers in different ranges!
    */
    class RandomNumberGenerator {
    public:
        std::mt19937 _gen;
        RandomNumberGenerator(): _gen(std::mt19937()) {
            set_random_seed(); // Default to random seed
        }

        /** Seed the random number generator with the specified seed */
        void set_seed(uint64_t new_seed = std::mt19937_64::default_seed) {
            _gen.seed(new_seed);
        }
        /** Seed the random number generator with a random seed(based on current time) */
        void set_random_seed() {
            int time_ms = get_time_since_epoch_ms();
            set_seed(time_ms);
            // Call random generator a few times to make it more random
            //   (each call to _gen updates the state randomly)
            rand();
            rand();
            rand();
            rand();
            rand();
        }

        /** Generate a random number between 0 and 1 */
        double rand_between_0_and_1() {
            std::uniform_real_distribution<double> dis(0.0, 1.0);
            double x = dis(_gen);
            return x;
        }

        /** Generate a random integer number of an int, min=0, max=2^64-1 */
        uint64_t rand() {
            return _gen();
        }
        /** Generate a random double in the specified interval */
        double rand_double(double min_, double max_) {
            Assert(min_ <= max_);
            double rng_val = rand_between_0_and_1() * (max_ - min_) + min_;
            return rng_val;
        }
        /** Generate a random integer in the specified interval */
        int rand_int(int min_, int max_) {
            Assert(min_ <= max_);
            int rng_val = (rand() % (max_ - min_)) + min_;
            return rng_val;
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
     * Tries to fit a list of rectangles into a bigger rectangle of a certain width and height
     * Returns true if it is succssfull!
     * Updates the positions of the rectangles so that they do not overlap
     * 
     * NOTE! Assumes the rectangles are already sorted on height in descending order
    */
    bool fit_rects_in_rect(std::vector<RectT<int>>& rects_height_descend, int width, int height) {
        if(rects_height_descend.size() == 0) {
            // Trivial case
            return true;
        }

        // Naive approach(Can be improved)
        // Tries to fit as many rects as possible on a certain y-level, if it fails, it moves to a new y-level
        int y_level = 0;
        int level_height = rects_height_descend[0].h;
        int x = 0;
        for(int i = 0; i < rects_height_descend.size(); i++) {
            if(rects_height_descend[i].w + x >= width) {
                // Cannot fit more rects on current y-level, move to new y-level
                y_level += level_height;
                level_height = rects_height_descend[i].h;
                x = 0;
            }
            if(rects_height_descend[i].h + y_level >= height) {
                // Height limit exceeded!
                return false;
            }
            rects_height_descend[i].x = x;
            rects_height_descend[i].y = y_level;
            x += rects_height_descend[i].w;
        }
        return true;
    }
    /**
     * Try to pack a list of rectangles into the smallest 2^n * 2^n grid as possible
     *   such that no rectangles overlap (no rotations)
     * Updates the rectangle positions and returns the grid size
     * 
     * Some names related to this problem
     *  - bin packing
     *  - rect packing
    */
    int rect_packing_pow2_sq(std::vector<RectT<int>>& rects) {
        // Lets start by calculating a minimum for the grid size
        //  - It has to be bigger or equal to the total area of the rectangles
        //  - It cannot be smaller than the biggest rectangle

        int area = 0;
        for(int i = 0; i < rects.size(); i++) {
            area += rects[i].h * rects[i].w;
        }

        int max_rect_size = 0;
        for(int i = 0; i < rects.size(); i++) {
            max_rect_size = std::max(rects[i].w, max_rect_size);
            max_rect_size = std::max(rects[i].h, max_rect_size);
        }
        unsigned int grid_size = upper_power_of_two(std::sqrt(area));
        grid_size = std::max(grid_size, upper_power_of_two(max_rect_size));
        

        // Lets sort the rectangles by height in descending order(while maintaining the index of the rectangles)
        std::vector<std::pair<int, int>> rects_height_and_index; // <height, rect index>
        for(int i = 0; i < rects.size(); i++) {
            rects_height_and_index.push_back(std::make_pair(rects[i].h, i));
        }
        std::sort(rects_height_and_index.begin(), 
            rects_height_and_index.end(), 
            [](const std::pair<int, int>& lhs, const std::pair<int, int>& rhs) {
                return lhs > rhs;
        });


        // Lets create a copy of rects but with the height sorted in descending order
        std::vector<RectT<int>> rects_height_descend;
        rects_height_descend.resize(rects.size());
        for(int i = 0; i < rects.size(); i++) {
            rects_height_descend[i] = rects[rects_height_and_index[i].second];
        }

        // Sanity check to see it is descending order
        for(int i = 0; i < std::min((int)rects_height_descend.size(), 10); i++) {
            if(i != 0) {
                // Make sure it is ascending!
                Assert(rects_height_descend[i-1].h >= rects_height_descend[i].h)
            }
            DebugExpr(rects_height_descend[i].to_string());
        }


        // Lets start trying to place the rectangles into the grid! And see if they fit
        //  If they don't fit, then we have to increase the size of the grid and try again...
        while(fit_rects_in_rect(rects_height_descend, grid_size, grid_size) == false) {
            grid_size = grid_size * 2;
        }

        // We found a match! Update the positions of the original rectangles
        for(int i = 0; i < rects.size(); i++) {
            rects[rects_height_and_index[i].second] = rects_height_descend[i];
        }
        return grid_size;
    }
    void TEST_rect_packing_pow2_sq() {
        // Try packing a lot of rectangles and see that
        //  - The grid size is a power of 2
        //  - All rectangles maintain their width and height
        //  - They all fit inside the provided grid size
        //  - None overlap
        RandomNumberGenerator rand_gen;
        rand_gen.set_seed(123);
        std::vector<RectT<int>> rects;
        for(int i = 0; i < 100; i++) {
            RectT<int> new_rect;
            new_rect.x = rand_gen.rand_int(1, 100);
            new_rect.y = rand_gen.rand_int(1, 100);
            new_rect.w = rand_gen.rand_int(1, 100);
            new_rect.h = rand_gen.rand_int(1, 100);
            rects.push_back(new_rect);
        }
        std::vector<RectT<int>> rects_copy = rects;
        int grid_size = rect_packing_pow2_sq(rects);
        PrintExpr(grid_size);

        // Assert the grid size is a power of 2
        Assert(vicmil::is_power_of_two(grid_size) == true);

        // Assert all rectangles maintain their width and height
        Assert(rects.size() == rects_copy.size());
        for(int i = 0; i < rects.size(); i++) {
            Assert(rects[i].w == rects_copy[i].w);
            Assert(rects[i].h == rects_copy[i].h);
        }

        // Assert all rectangles fit inside the grid
        for(int i = 0; i < rects.size(); i++) {
            Assert(rects[i].min_x() >= 0);
            Assert(rects[i].min_y() >= 0);
            Assert(rects[i].max_x() < grid_size);
            Assert(rects[i].max_y() < grid_size);
        }

        // Assert no rectangles overlap
        for(int i1 = 0; i1 < rects.size(); i1++) {
            for(int i2 = 0; i2 < i1; i2++) {
                bool overlap_ = is_overlap(rects[i1], rects[i2]);
                if(overlap_) {
                    PrintExpr(i1);
                    PrintExpr(i2);
                    PrintExpr(rects[i1].to_string_min_max());
                    PrintExpr(rects[i2].to_string_min_max());
                }
                Assert(overlap_ == false);
            }
        }
    }
    AddTest(TEST_rect_packing_pow2_sq);
}