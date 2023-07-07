// file   : Logger.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Logger.hpp"

#include <log4cpp/Category.hh>

std::mutex Logger::s_mutex;

Logger::Logger(int priority) : m_priority(priority) { }

Logger::~Logger()
{
  std::lock_guard<std::mutex> lock(s_mutex);
  log4cpp::Category::getInstance("application").getStream(m_priority) << m_stream.str();
}
