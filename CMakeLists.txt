cmake_minimum_required(VERSION 3.20)
project(hello CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

find_package(Freetype CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(cairo REQUIRED IMPORTED_TARGET cairo)

add_executable(${PROJECT_NAME} src/main.cpp src/fontmanager.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Freetype::Freetype PkgConfig::cairo pthread
)