#include "L4_file_access.h"

#include <chrono>
#include <thread>

namespace vicmil {
double nanoseconds_to_seconds(double nanoseconds) {
    /* 1 second = 1000 milli seconds
       1 second = 1000*1000 micro seconds
       1 second = 1000*1000*1000 nano seconds
    */
    return nanoseconds / (1000.0 * 1000.0 * 1000.0);
}
double seconds_to_nanoseconds(double seconds) {
    /* 1 second = 1000 milli seconds
       1 second = 1000*1000 micro seconds
       1 second = 1000*1000*1000 nano seconds
    */
    return seconds * (1000.0 * 1000.0 * 1000.0);
}


/**
 * Returns the time since epoch in seconds, 
 * NOTE! Different behaviour on different devices
 * on some devices epoch refers to January 1: 1970, 
 * on other devices epoch might refer to time since last boot
*/
double get_time_since_epoch_s() {
    using namespace std::chrono;
    auto time = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch());
    double nano_secs = time.count();
    return nanoseconds_to_seconds(nano_secs);
}
double get_time_since_epoch_ms() {
    return get_time_since_epoch_s() * 1000;
}


/**
 * Sleep for a specified time, measured in seconds
 * NOTE! 
 * On some devices, the actual sleep time might deviate a bit from the one specified
*/
void sleep_s(double sleep_time_s) {
    double time_ms = sleep_time_s * 1000;
    std::this_thread::sleep_for(std::chrono::milliseconds((int64_t)time_ms));
}
void TEST_sleep() {
    double start_time = get_time_since_epoch_s();
    sleep_s(0.7);
    double end_time = get_time_since_epoch_s();
    double duration = end_time - start_time;
    AssertEq(duration, 0.7, 0.1);
}
AddTest(TEST_sleep);


/**
 * Timer to time things
*/
class Timer {
    double _start_s; // The start time(measured in seconds)
public:
    Timer() {
        reset();
    }
    /**
     * Get time in seconds since the timer was either created or reset
    */
    double get_time_s() {
        return get_time_since_epoch_s() - _start_s;
    }
    /**
     * Get time in milliseconds since the timer was either created or reset
    */
    double get_time_ms() {
        return get_time_s() * 1000.0;
    }
    void reset() {
        _start_s = get_time_since_epoch_s();
    }
};


class FrameStabilizer {
public:
    unsigned int _frames_per_second;
    unsigned int _update_count;
    Timer timer;
    FrameStabilizer() {}
    FrameStabilizer(unsigned int frames_per_second_) {
        _frames_per_second = frames_per_second_;
        reset();
    }
    void reset() {
        _update_count = 0;
        timer = Timer();
    }
    double get_time_to_next_frame_s() {
        double next_frame_time_s = ((double)_update_count) / (double)_frames_per_second;
        double time_to_next_frame_s = next_frame_time_s - timer.get_time_s();
        return time_to_next_frame_s;
    }
    void record_frame() {
        _update_count += 1;
    }
    double get_expected_frame_count() {
        return timer.get_time_s() * _frames_per_second;
    }
};


class FPSCounter {
    Timer timer;
    unsigned int frame_count = 0;
    double fps = 0;
public:
    FPSCounter() {
        timer = Timer();
    }
    void record_frame() {
        frame_count += 1;
        double timer_time = timer.get_time_s();
        DebugExpr(timer_time);

        // Reset the timer each second and calculate the fps count
        if(timer_time > 1.0) {
            fps = frame_count / timer_time;
            timer = Timer();
            frame_count = 0;
        }
    }
    double get_fps() {
        return fps;
    }
};
}
