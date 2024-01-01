// file   : FileIOError.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_FILE_IO_ERROR_HPP
#define THEGAME_FILE_IO_ERROR_HPP

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

  const char* what() const noexcept override
  {
    return m_msg.c_str();
  }

private:
  std::string m_msg;
};

#endif /* THEGAME_FILE_IO_ERROR_HPP */
