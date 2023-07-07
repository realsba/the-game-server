// file   : util.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef UTIL_HPP
#define UTIL_HPP

#include <string>
#include <chrono>

std::string randomString(size_t length);

std::string toString(const std::chrono::system_clock::time_point& tp);

#endif /* UTIL_HPP */
