// file   : TimePoint.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef TIME_POINT_HPP
#define TIME_POINT_HPP

#include <chrono>

typedef std::chrono::steady_clock::time_point TimePoint;
typedef std::chrono::steady_clock::duration Duration;

typedef std::chrono::system_clock::time_point SystemTimePoint;

#endif /* TIME_POINT_HPP */
