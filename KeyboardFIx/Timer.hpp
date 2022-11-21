#pragma once
#include<stdint.h>

class Timer
{
public:
    //Get current time in milliseconds.
    static inline int64_t GetTime();

    void Start() {

    }

    int64_t GetDelta();

private:
    
    //Variables used for timing.
    int64_t m_time;
    int64_t m_time_last;
    int64_t m_delta;
};

