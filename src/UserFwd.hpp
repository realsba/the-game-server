// file   : UserFwd.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef USERFWD_HPP
#define USERFWD_HPP

#include <memory>

class User;
typedef std::shared_ptr<User> UserSPtr;
typedef std::weak_ptr<User> UserWPtr;

#endif /* USERFWD_HPP */
