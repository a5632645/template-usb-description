cmake_minimum_required(VERSION 3.11)

project(constexpr-usb)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_compile_options(-Wall -Wextra)

file(GLOB_RECURSE sources example/*.cpp)
add_executable(constexpr-usb ${sources})
target_include_directories(constexpr-usb PUBLIC .)
