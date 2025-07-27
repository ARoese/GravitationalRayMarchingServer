cmake_minimum_required(VERSION 3.31)
project(GravitationalRayMarchingServer CXX CUDA)
set(CMAKE_CXX_STANDARD 20)
# required for protobuf
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_subdirectory(GravitationalRayMarching EXCLUDE_FROM_ALL)
add_subdirectory(libgrmproto EXCLUDE_FROM_ALL)

set(Boost_ROOT C:/Boost/)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost REQUIRED)

add_executable(GravitationalRayMarchingServer src/main.cpp
        src/GRMExceptions.cpp
        src/GRMExceptions.h
        src/conversions.h
        src/conversions.cpp
        src/requests.cpp
        src/requests.h
)
target_include_directories(GravitationalRayMarchingServer PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(GravitationalRayMarchingServer PRIVATE GravitationalRayMarching libgrmproto)