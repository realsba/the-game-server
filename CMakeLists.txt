cmake_minimum_required(VERSION 3.25)
project(thegame)

set(CMAKE_CXX_STANDARD 23)

add_compile_definitions(
    MYSQLPP_MYSQL_HEADERS_BURIED
)

find_package(spdlog REQUIRED)

add_executable(${PROJECT_NAME}
    src/entity/Avatar.cpp
    src/entity/Avatar.hpp
    src/entity/Cell.cpp
    src/entity/Cell.hpp
    src/entity/Food.cpp
    src/entity/Food.hpp
    src/entity/Mass.cpp
    src/entity/Mass.hpp
    src/entity/Mother.cpp
    src/entity/Mother.hpp
    src/entity/Phage.cpp
    src/entity/Phage.hpp
    src/entity/Virus.cpp
    src/entity/Virus.hpp
    src/geometry/AABB.cpp
    src/geometry/AABB.hpp
    src/geometry/Circle.hpp
    src/geometry/Vec2D.cpp
    src/geometry/Vec2D.hpp
    src/geometry/geometry.cpp
    src/geometry/geometry.hpp
    src/packet/EmptyPacket.cpp
    src/packet/EmptyPacket.hpp
    src/packet/InputPacketTypes.hpp
    src/packet/OutputPacketTypes.hpp
    src/packet/serialization.hpp
    src/packet/PacketGreeting.cpp
    src/packet/PacketGreeting.hpp
    src/packet/PacketLeaderboard.cpp
    src/packet/PacketLeaderboard.hpp
    src/packet/PacketPlay.cpp
    src/packet/PacketPlay.hpp
    src/packet/PacketSpectate.cpp
    src/packet/PacketSpectate.hpp
    src/Application.cpp
    src/Application.hpp
    src/Config.cpp
    src/Config.hpp
    src/FileIOError.hpp
    src/Gridmap.cpp
    src/Gridmap.hpp
    src/MySQLConfig.hpp
    src/MySQLConnectionPool.cpp
    src/MySQLConnectionPool.hpp
    src/NextId.cpp
    src/NextId.hpp
    src/Player.cpp
    src/Player.hpp
    src/Room.cpp
    src/Room.hpp
    src/RoomManager.cpp
    src/RoomManager.hpp
    src/ScopeExit.hpp
    src/TSRoom.cpp
    src/TSRoom.hpp
    src/TimePoint.hpp
    src/Timer.cpp
    src/Timer.hpp
    src/User.cpp
    src/User.hpp
    src/UserFwd.hpp
    src/UsersCache.cpp
    src/UsersCache.hpp
    src/main.cpp
    src/types.hpp
    src/util.cpp
    src/util.hpp
    src/version.hpp
    src/Listener.cpp
    src/Listener.hpp
    src/ListenerFwd.hpp
    src/Session.cpp
    src/Session.hpp
    src/AsioFormatter.hpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog config++ mysqlpp)

add_subdirectory(tests)