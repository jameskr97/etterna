#include "Timer.hpp"


namespace Core::Timer {
    std::chrono::time_point<timer_clock> APP_START_TIME = timer_clock::now();

    time_point getCurrentTime() {
        return timer_clock::now();
    }
}