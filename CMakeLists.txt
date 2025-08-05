cmake_minimum_required(VERSION 3.31)
#set(CMAKE_TOOLCHAIN_FILE "C:/tools/vcpkg/scripts/buildsystems/vcpkg.cmake")
project(GravitationalRayMarchingServer CXX CUDA)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(GravitationalRayMarching EXCLUDE_FROM_ALL)
add_subdirectory(libgrmproto EXCLUDE_FROM_ALL)

find_package(Boost COMPONENTS program_options asio endian REQUIRED)

add_executable(GravitationalRayMarchingServer src/main.cpp
        src/GRMExceptions.cpp
        src/GRMExceptions.h
        src/conversions.h
        src/conversions.cpp
        src/requests.cpp
        src/requests.h
)
target_include_directories(GravitationalRayMarchingServer PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(GravitationalRayMarchingServer PRIVATE GravitationalRayMarching libgrmproto Boost::program_options)