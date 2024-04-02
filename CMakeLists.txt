cmake_minimum_required(VERSION 3.25)

project(thegame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

add_compile_definitions(
    MYSQLPP_MYSQL_HEADERS_BURIED
)

find_package(Boost REQUIRED COMPONENTS headers)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(spdlog REQUIRED)

set(SOURCE_FILES
    src/Application.cpp
    src/Bot.cpp
    src/Config.cpp
    src/Gridmap.cpp
    src/HttpClient.cpp
    src/IOThreadPool.cpp
    src/Listener.cpp
    src/MySQLConnectionPool.cpp
    src/NextId.cpp
    src/OutgoingPacket.cpp
    src/Player.cpp
    src/Room.cpp
    src/RoomManager.cpp
    src/Session.cpp
    src/Timer.cpp
    src/User.cpp
    src/UsersCache.cpp
    src/boost_asio.cpp
    src/boost_mysql.cpp
    src/entity/Avatar.cpp
    src/entity/Bullet.cpp
    src/entity/Cell.cpp
    src/entity/Food.cpp
    src/entity/Mother.cpp
    src/entity/Phage.cpp
    src/entity/Virus.cpp
    src/geometry/AABB.cpp
    src/geometry/Vec2D.cpp
    src/geometry/geometry.cpp
    src/util.cpp
)

set(HEADER_FILES
    src/entity/Avatar.hpp
    src/entity/Bullet.hpp
    src/entity/Cell.hpp
    src/entity/Food.hpp
    src/entity/Mother.hpp
    src/entity/Phage.hpp
    src/entity/Virus.hpp
    src/geometry/AABB.hpp
    src/geometry/Circle.hpp
    src/geometry/Vec2D.hpp
    src/geometry/formatter.hpp
    src/geometry/geometry.hpp
    src/Application.hpp
    src/AsioFormatter.hpp
    src/Bot.hpp
    src/ChatMessage.hpp
    src/Config.hpp
    src/EventEmitter.hpp
    src/Gridmap.hpp
    src/HttpClient.hpp
    src/IEntityFactory.hpp
    src/IOThreadPool.hpp
    src/IncomingPacket.hpp
    src/Listener.hpp
    src/ListenerFwd.hpp
    src/MySQLConnectionPool.hpp
    src/NextId.hpp
    src/OutgoingPacket.hpp
    src/Player.hpp
    src/PlayerFwd.hpp
    src/Room.hpp
    src/RoomManager.hpp
    src/ScopeExit.hpp
    src/Session.hpp
    src/TimePoint.hpp
    src/Timer.hpp
    src/User.hpp
    src/UserFwd.hpp
    src/UsersCache.hpp
    src/serialization.hpp
    src/types.hpp
    src/util.hpp
    src/version.hpp
)

add_executable(${PROJECT_NAME}
    ${SOURCE_FILES}
    ${HEADER_FILES}
    src/main.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Boost::headers Threads::Threads OpenSSL::Crypto OpenSSL::SSL spdlog::spdlog mysqlpp
)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    BOOST_ASIO_SEPARATE_COMPILATION
    BOOST_MYSQL_SEPARATE_COMPILATION
)

#target_include_directories(${PROJECT_NAME} PRIVATE
#    "${CMAKE_CURRENT_SOURCE_DIR}/src"
#)

add_subdirectory(tests)