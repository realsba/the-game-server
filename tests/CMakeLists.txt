find_package(Catch2 3 REQUIRED)
find_package(fmt REQUIRED)

list(TRANSFORM SOURCE_FILES PREPEND "../")
list(TRANSFORM HEADER_FILES PREPEND "../")

message("TEST_SOURCE_FILES: ${TEST_SOURCE_FILES}")

add_executable(tests
    ${SOURCE_FILES}
    ${HEADER_FILES}
    geometry/Test_AABB.cpp
    geometry/Test_Vec2D.cpp
    geometry/Test_geometry.cpp
)

target_include_directories(tests PRIVATE "${CMAKE_SOURCE_DIR}/src")

target_link_libraries(tests PRIVATE
    Catch2::Catch2WithMain
    Boost::headers
    Threads::Threads
    OpenSSL::Crypto
    OpenSSL::SSL
    spdlog::spdlog
    mysqlpp
)

target_compile_definitions(tests PRIVATE
    BOOST_ASIO_SEPARATE_COMPILATION
    BOOST_MYSQL_SEPARATE_COMPILATION
)

#include(Catch)
#catch_discover_tests(tests)