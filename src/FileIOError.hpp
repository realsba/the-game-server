// file   : FileIOError.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef FILE_IO_ERROR_HPP
#define FILE_IO_ERROR_HPP

#include <exception>
#include <sstream>
#include <cstring>
#include <string>

class FileIoError : public std::exception {
public:
  explicit FileIoError(const std::string& filename, int errnum)
  {
    std::stringstream ss;
    ss << "FileIoError: " << strerror(errnum) << " (" << errnum
       << "). FileName=\"" << filename << "\"";
    m_msg = ss.str();
  }

  virtual const char* what() const noexcept
  {
    return m_msg.c_str();
  }

private:
  std::string m_msg;
};

#endif /* FILE_IO_ERROR_HPP */
