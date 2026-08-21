file(GLOB_RECURSE sources *.cpp)

add_executable(constexpr-usb ${sources})
target_include_directories(constexpr-usb PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(constexpr-usb PRIVATE tpusb)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
