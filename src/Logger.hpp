// file   : Logger.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <log4cpp/Priority.hh>
#include <sstream>
#include <mutex>

class Logger {
public:
  explicit Logger(int priority);
  ~Logger();

  template <class T> Logger& operator<<(const T& t)
  {
    m_stream << t;
    return *this;
  }

protected:
  static std::mutex   s_mutex;

  std::ostringstream  m_stream;
  int                 m_priority;
};

#ifdef SHORT_LOGGER_INFO
  #define LOGGER_INFO
#else
  #define LOGGER_INFO << "[" << __FILE__ << ":" << __LINE__ << " - " << __FUNCTION__ << "] "
#endif

#define LOG_DEBUG  Logger(log4cpp::Priority::DEBUG)  LOGGER_INFO
#define LOG_INFO   Logger(log4cpp::Priority::INFO)   LOGGER_INFO
#define LOG_NOTICE Logger(log4cpp::Priority::NOTICE) LOGGER_INFO
#define LOG_WARN   Logger(log4cpp::Priority::WARN)   LOGGER_INFO
#define LOG_ERROR  Logger(log4cpp::Priority::ERROR)  LOGGER_INFO
#define LOG_CRIT   Logger(log4cpp::Priority::CRIT)   LOGGER_INFO
#define LOG_ALERT  Logger(log4cpp::Priority::ALERT)  LOGGER_INFO
#define LOG_EMERG  Logger(log4cpp::Priority::EMERG)  LOGGER_INFO
#define LOG_FATAL  Logger(log4cpp::Priority::FATAL)  LOGGER_INFO

#define LOG_SYSTEM_ERROR LOG_ERROR << strerror(errno) << " (" << errno << ")."

#endif /* _LOGGER_HPP */
