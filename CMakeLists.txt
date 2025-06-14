cmake_minimum_required(VERSION 3.11)

project(constexpr-usb)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_CXX_STANDARD 20)
add_compile_options(-Wall -Wextra)

add_executable(constexpr-usb main.cpp)
