#include "L9_shapes.h"
#include <random>

namespace vicmil {
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
}