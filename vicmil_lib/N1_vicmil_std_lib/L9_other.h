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

    class Rect {
        public:
        double x;
        double y;
        double w;
        double h;
        double min_x() {
            return x;
        }
        double max_x() {
            return x + w;
        }
        double min_y() {
            return y;
        }
        double max_y() {
            return y + h;
        }
        bool is_inside_rect(double x_, double y_) {
            if(!in_range(x_, x, x + w)) {
                return false;
            }
            if(!in_range(y_, y, y + h)) {
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
        int add_subwindow(int window_id, double weight = 1, bool horizontal_windows = false) { // Return the new instance id
            if(window_id == get_instance_id()) {
                _subwindows.push_back(WindowLayout());
                _subwindows.back()._weight = weight;
                _subwindows.back()._split_window_horizontal = horizontal_windows;
                _update_subwindow_positions();
                return _subwindows.back().get_instance_id();
            }
            for(int i = 0; i < _subwindows.size(); i++) {
                int new_instance_id = _subwindows[i].add_subwindow(window_id, weight, horizontal_windows) != -1;
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
        int set_split_window_horizontal(int window_id, bool split_window_horizontal) {
            if(window_id == get_instance_id()) {
                _split_window_horizontal = split_window_horizontal;
                _update_subwindow_positions();
                return 0;
            }
            for(int i = 0; i < _subwindows.size(); i++) {
                if(_subwindows[i].set_split_window_horizontal(window_id, split_window_horizontal) != -1) {
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
        int64_t control_window = layout.add_subwindow(master_window, 1);
        int64_t game_view_window = layout.add_subwindow(master_window, 3);
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
}