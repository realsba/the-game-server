// file   : src/TimePoint.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_TIME_POINT_HPP
#define THEGAME_TIME_POINT_HPP

#include <chrono>

using namespace std::literals;

using TimePoint = std::chrono::steady_clock::time_point;
using Duration = std::chrono::steady_clock::duration;

using SystemTimePoint = std::chrono::system_clock::time_point;

#endif /* THEGAME_TIME_POINT_HPP */
