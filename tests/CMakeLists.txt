find_package(Catch2 3 REQUIRED)

add_executable(tests
    ../src/geometry/AABB.cpp
    ../src/geometry/AABB.hpp
    ../src/geometry/Circle.hpp
    ../src/geometry/Vec2D.cpp
    ../src/geometry/Vec2D.hpp
    ../src/geometry/geometry.cpp
    ../src/geometry/geometry.hpp
    geometry/Test_AABB.cpp
    geometry/Test_Vec2D.cpp
    geometry/Test_geometry.cpp
)

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

#include(Catch)
#catch_discover_tests(tests)