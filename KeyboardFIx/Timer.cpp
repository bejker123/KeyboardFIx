#include "Timer.hpp"
#include<chrono>

inline int64_t Timer::GetTime() {
    auto duration = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

int64_t Timer::GetDelta() {
    m_time_last = m_time;
    m_time = GetTime();
    return (m_delta = (m_time - m_time_last));
}