#include "L8_typing.h"
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
     * General template class for rectangle
    */
    template<class T>
    class RectT {
        public:
        T x = 0;
        T y = 0;
        T w = 0;
        T h = 0;
        RectT() {}
        RectT(T x_, T y_, T w_, T h_) {
            x = x_;
            y = y_;
            w = w_;
            h = h_;
        }
        T min_x() {
            return x;
        }
        T max_x() {
            return x + w;
        }
        T min_y() {
            return y;
        }
        T max_y() {
            return y + h;
        }
        T center_x() {
            return x + (w / 2.0);
        }
        T center_y() {
            return y + (h / 2.0);
        }
        bool is_inside_rect(T x_, T y_) {
            if(!in_range<T>(x_, x, x + w)) {
                return false;
            }
            if(!in_range<T>(y_, y, y + h)) {
                return false;
            }
            return true;
        }
        std::string to_string() {
            return "x: " + std::to_string(x) + 
                    "   y: " + std::to_string(y) + 
                    "   w: " + std::to_string(w) + 
                    "   h: " + std::to_string(h);
        }
        std::string to_string_min_max() {
            return "min_x: " + std::to_string(min_x()) + 
                    "   min_y: " + std::to_string(min_y()) + 
                    "   max_x: " + std::to_string(max_x()) + 
                    "   max_y: " + std::to_string(max_y());
        }

    };
    typedef RectT<double> Rect;

    template<class T>
    bool is_overlap(RectT<T> rect1, RectT<T> rect2) {
        Assert(rect1.w > 0);
        Assert(rect1.h > 0);
        Assert(rect2.w > 0);
        Assert(rect2.h > 0);
        if(rect1.min_x() >= rect2.max_x() || rect2.min_x() >= rect1.max_x()) {
            return false;
        }
        if(rect1.min_y() >= rect2.max_y() || rect2.min_y() >= rect1.max_y()) {
            return false;
        }
        return true;
    }

    /**
     * Get the smallest rectangle so that no part of rect1 and rect2 is outside it
    */
    template<class T>
    Rect get_bounding_rect(RectT<T> rect1, RectT<T> rect2) {
        T min_x = std::min(rect1.min_x(), rect2.min_x());
        T min_y = std::min(rect1.min_y(), rect2.min_y());
        T max_x = std::max(rect1.max_x(), rect2.max_x());
        T max_y = std::max(rect1.max_y(), rect2.max_y());
        return Rect(min_x, min_y, max_x - min_x, max_y - min_y);
    }

    /**
     * Class for specifying the window layout, what window should go where etc.
     * The implementation is based on a divide approach, 
     * you can split each window into a number of "subwindows", and assign 
     * relative weight to how much of the window that subwindow should take up
     *    (The subwindows then make up windows themselves)
     * 
     * Each subwindow will be assigned their own id, so they should be easy to reference
     *   I think it is belneficial to use an id approach over pointers to avoid unsafe memory
     *    when subwindows and their children are deleted
    */
    class WindowLayout: public ClassInstanceCounter {
        double _weight = 1; // The weight specifies how much space it should take on page
        bool _split_window_horizontal = false; // Otherwise vertical
        std::vector<WindowLayout> _subwindows = {};

        // The calculated position on the screen
        Rect _window_pos = Rect();
        public:
        WindowLayout() {}
        void set_window_position(Rect new_window_pos) {
            _window_pos = new_window_pos;
            _update_subwindow_positions();
        }
        void set_window_position(double min_x, double min_y, double max_x, double max_y) {
            _window_pos.x = min_x;
            _window_pos.y = min_y;
            _window_pos.w = max_x - min_x;
            _window_pos.h = max_y - min_y;
            _update_subwindow_positions();
        }

        /**
         * fetches the window position of the specified window/subwindow
        */
        int get_window_position(int window_id, Rect* window_pos) {
            if(window_id == get_instance_id()) {
                *window_pos = _window_pos;
                return 0;
            }
            for(int i = 0; i < _subwindows.size(); i++) {
                if(_subwindows[i].get_window_position(window_id, window_pos) != -1) {
                    return 0; // Success!
                }
            }
            return -1;
        }
        /**
         * Create a new subwindow of the window with specified id
         * Returns the id of the newly created window if it is successfully created
         * Returns -1 if something went wrong
        */
        // Returns -1 if it failed, otherwise the the instance id
        int add_subwindow(double weight = 1, int window_id = -1, bool horizontal_windows = false) { // Return the new instance id
            if(window_id == get_instance_id() || window_id == -1) {
                _subwindows.push_back(WindowLayout());
                _subwindows.back()._weight = weight;
                _subwindows.back()._split_window_horizontal = horizontal_windows;
                _update_subwindow_positions();
                return _subwindows.back().get_instance_id();
            }
            for(int i = 0; i < _subwindows.size(); i++) {
                int new_instance_id = _subwindows[i].add_subwindow(weight, window_id, horizontal_windows);
                if(new_instance_id != -1) {
                    return new_instance_id; // Success!
                }
            }
            return -1; // Could not find the instance id requested for
        }
        // Returns -1 if it failed
        int set_weight(int window_id, double weight) {
            for(int i = 0; i < _subwindows.size(); i++) {
                if(window_id == get_instance_id()) {
                    _subwindows[i]._weight = weight;
                    _update_subwindow_positions();
                    return 0;
                }
            }
            for(int i = 0; i < _subwindows.size(); i++) {
                if(_subwindows[i].set_weight(window_id, weight) != -1) {
                    return 0; // Success!
                }
            }
            return -1;
        }
        int set_split_window_horizontal(bool split_window_horizontal, int window_id = -1) {
            if(window_id == get_instance_id() || window_id == -1) {
                _split_window_horizontal = split_window_horizontal;
                _update_subwindow_positions();
                return 0;
            }
            for(int i = 0; i < _subwindows.size(); i++) {
                if(_subwindows[i].set_split_window_horizontal(split_window_horizontal, window_id) != -1) {
                    return 0; // Success!
                }
            }
            return -1;
        }
        int delete_all_subwindows(int window_id) {
            if(window_id == get_instance_id()) {
                _subwindows = {};
                return 0;
            }
            for(int i = 0; i < _subwindows.size(); i++) {
                if(_subwindows[i].delete_all_subwindows(window_id) != -1) {
                    return 0; // Success!
                }
            }
            return -1; // Could no locate the window :(
        }
        int delete_subwindow(int window_id) {
            for(int i = 0; i < _subwindows.size(); i++) {
                if(_subwindows[i].get_instance_id() == window_id) {
                    // Delete subwindow from subwindow vector
                    vec_remove(_subwindows, i);
                    return 0; // Success!
                }
            }

            // Propagate down into subwindows
            for(int i = 0; i < _subwindows.size(); i++) {
                if(_subwindows[i].delete_subwindow(window_id) != -1) {
                    return 0; // Success!
                }
            }

            return -1;
        }
        bool window_exists(int window_id) {
            if(window_id == get_instance_id()) {
                return true;
            }
            for(int i = 0; i < _subwindows.size(); i++) {
                if(_subwindows[i].get_instance_id() == window_id) {
                    return true; // Success!
                }
            }
            // Propagate down into subwindows
            for(int i = 0; i < _subwindows.size(); i++) {
                if(_subwindows[i].window_exists(window_id) == true) {
                    return true; // Success!
                }
            }

            return false;
        }
        /**
         * Recursivly iterate through subwindows to make their position align with the new conditions
         *  eg. weights and this window position
        */
        void _update_subwindow_positions() {
            double tot_weight = 0;
            for(int i = 0; i < _subwindows.size(); i++) {
                tot_weight += _subwindows[i]._weight;
            }

            Rect new_window_pos;
            new_window_pos.x = _window_pos.x;
            new_window_pos.y = _window_pos.y;
            new_window_pos.w = 0;
            new_window_pos.h = 0;

            // Update the position of all subwindows
            for(int i = 0; i < _subwindows.size(); i++) {
                if(_split_window_horizontal) {
                    new_window_pos.w = _window_pos.w * (_subwindows[i]._weight / tot_weight);
                    new_window_pos.h = _window_pos.h;
                    _subwindows[i].set_window_position(new_window_pos);
                    new_window_pos.x += new_window_pos.w;
                }
                else {
                    new_window_pos.w = _window_pos.w;
                    new_window_pos.h = _window_pos.h * (_subwindows[i]._weight / tot_weight);
                    _subwindows[i].set_window_position(new_window_pos);
                    new_window_pos.y += new_window_pos.h;
                }
            }
        }
    };

    void TEST_WindowLayout() {
        std::cout << "this is a test!" << std::endl;
        WindowLayout layout = WindowLayout();
        layout.set_window_position(0, 0, 1000, 800);
        int64_t master_window = layout.get_instance_id();
        int64_t control_window = layout.add_subwindow(1, master_window);
        int64_t game_view_window = layout.add_subwindow(3, master_window);
        Rect pos;
        layout.get_window_position(control_window, &pos);
        std::cout << "control: " << pos.to_string() << std::endl;
        AssertEq(pos.min_x(), 0, 0.1);
        AssertEq(pos.min_y(), 0, 0.1);
        AssertEq(pos.max_x(), 1000, 0.1);
        AssertEq(pos.max_y(), 200, 0.1);
        layout.get_window_position(game_view_window, &pos);
        std::cout << "game: " << pos.to_string() << std::endl;
        AssertEq(pos.min_x(), 0, 0.1);
        AssertEq(pos.min_y(), 200, 0.1);
        AssertEq(pos.max_x(), 1000, 0.1);
        AssertEq(pos.max_y(), 800, 0.1);
    }
    AddTest(TEST_WindowLayout);

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