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

add_executable(${PROJECT_NAME}
    src/entity/Avatar.cpp
    src/entity/Avatar.hpp
    src/entity/Bullet.cpp
    src/entity/Bullet.hpp
    src/entity/Cell.cpp
    src/entity/Cell.hpp
    src/entity/Food.cpp
    src/entity/Food.hpp
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
    src/geometry/formatter.hpp
    src/Application.cpp
    src/Application.hpp
    src/AsioFormatter.hpp
    src/Bot.cpp
    src/Bot.hpp
    src/ChatMessage.hpp
    src/Config.cpp
    src/Config.hpp
    src/EventEmitter.hpp
    src/Gridmap.cpp
    src/Gridmap.hpp
    src/HttpClient.cpp
    src/HttpClient.hpp
    src/IEntityFactory.hpp
    src/IncomingPacket.hpp
    src/Listener.cpp
    src/Listener.hpp
    src/ListenerFwd.hpp
    src/MySQLConnectionPool.cpp
    src/MySQLConnectionPool.hpp
    src/NextId.cpp
    src/NextId.hpp
    src/OutgoingPacket.cpp
    src/OutgoingPacket.hpp
    src/Player.cpp
    src/Player.hpp
    src/Room.cpp
    src/Room.hpp
    src/RoomManager.cpp
    src/RoomManager.hpp
    src/ScopeExit.hpp
    src/Session.cpp
    src/Session.hpp
    src/TimePoint.hpp
    src/Timer.cpp
    src/Timer.hpp
    src/User.cpp
    src/User.hpp
    src/UserFwd.hpp
    src/UsersCache.cpp
    src/UsersCache.hpp
    src/boost_asio.cpp
    src/boost_mysql.cpp
    src/main.cpp
    src/serialization.hpp
    src/types.hpp
    src/util.cpp
    src/util.hpp
    src/version.hpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Boost::headers Threads::Threads OpenSSL::Crypto OpenSSL::SSL spdlog::spdlog mysqlpp
)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    BOOST_ASIO_SEPARATE_COMPILATION
    BOOST_MYSQL_SEPARATE_COMPILATION
)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_subdirectory(tests)