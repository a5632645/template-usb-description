cmake_minimum_required(VERSION 3.11)

project(constexpr-usb)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall -Wextra)

add_library(tpusb INTERFACE)
target_include_directories(tpusb INTERFACE include)

file(GLOB_RECURSE sources example/*.cpp)
add_executable(constexpr-usb ${sources})
target_include_directories(constexpr-usb PUBLIC .)
target_link_libraries(constexpr-usb PRIVATE tpusb)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
