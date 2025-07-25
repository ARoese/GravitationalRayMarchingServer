cmake_minimum_required(VERSION 3.31)
project(GravitationalRayMarchingServer CXX)

# required for protobuf
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_subdirectory(GravitationalRayMarching EXCLUDE_FROM_ALL)
add_subdirectory(libgrmproto EXCLUDE_FROM_ALL)

set(CMAKE_CXX_STANDARD 20)

add_executable(GravitationalRayMarchingServer main.cpp)
target_link_libraries(GravitationalRayMarchingServer PRIVATE GravitationalRayMarching libgrmproto)