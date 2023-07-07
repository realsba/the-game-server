// file   : types.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef TYPES_HPP
#define TYPES_HPP

#include <websocketpp/common/connection_hdl.hpp>

#include <set>

typedef websocketpp::connection_hdl ConnectionHdl;
typedef std::set<ConnectionHdl, std::owner_less<ConnectionHdl>> Connections;

#endif /* TYPES_HPP */
