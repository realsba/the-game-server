// file   : src/util.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "util.hpp"

#include <chrono>
#include <algorithm>
#include <random>

std::string randomString(size_t length)
{
  constexpr char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  std::random_device generator;
  std::uniform_int_distribution<uint> distribution(0, sizeof(charset) - 2);
  auto randchar = [&]() -> char
  {
    return charset[distribution(generator)];
  };
  std::string str(length, 0);
  std::generate_n(str.begin(), length, randchar);
  return str;
}
