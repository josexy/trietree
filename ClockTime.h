
#pragma once
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif
namespace ClockTime {
#ifdef _WIN32
LARGE_INTEGER nFreq;
LARGE_INTEGER nBeginTime;
LARGE_INTEGER nEndTime;
#else
clock_t g_start_clock;
clock_t g_end_clock;
#endif
void start_timeclock() {
#ifdef _WIN32
    QueryPerformanceFrequency(&nFreq);
    QueryPerformanceCounter(&nBeginTime);
#else
    g_start_clock = 0;
    g_start_clock = clock();
#endif
}
void stop_timeclock() {
#ifdef _WIN32
    QueryPerformanceCounter(&nEndTime);
#else
    g_end_clock = 0;
    g_end_clock = clock();
#endif
}

double time_duration() {
#ifdef _WIN32
    return (double)(nEndTime.QuadPart - nBeginTime.QuadPart) /
           (double)nFreq.QuadPart;
#else
    return (double)(g_end_clock - g_start_clock) / CLOCKS_PER_SEC;
#endif
}
}  // namespace ClockTime
