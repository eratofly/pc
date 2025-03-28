cmake_minimum_required(VERSION 3.22)
project(music-player)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)

include_directories(./lib)

add_executable(music-player main.cpp)
target_include_directories(music-player PRIVATE ${PORTAUDIO_INCLUDE_DIRS})
target_link_directories(music-player PRIVATE ${PORTAUDIO_LIBRARY_DIRS})
target_link_libraries(music-player PRIVATE ${PORTAUDIO_LIBRARIES})
