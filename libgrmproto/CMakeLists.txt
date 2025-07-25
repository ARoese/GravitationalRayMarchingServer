cmake_minimum_required(VERSION 3.31)
project(GravitationalRayMarchingProtobuf CXX)

set(absl_DIR "C:/Program Files/Protobuf/lib/cmake/absl")
set(utf8_range_DIR "C:/Program Files/Protobuf/lib/cmake/utf8_range")
set(GTest_DIR "C:/Program Files/Protobuf/lib/cmake/GTest")
find_package(GTest CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

file(GLOB PROTO_FILES CONFIGURE_DEPENDS
        "protobuf/*.proto"
)

add_library(libgrmproto ${CMAKE_CURRENT_BINARY_DIR}/protobuf/)
target_include_directories(libgrmproto PUBLIC "C:/Program Files/Protobuf/include")
target_include_directories(libgrmproto PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/libgrmprotoGenerated/)
target_link_libraries(libgrmproto PRIVATE protobuf::libprotobuf)
protobuf_generate(
        TARGET libgrmproto
        LANGUAGE cpp
        PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/libgrmprotoGenerated/libgrmproto/
        IMPORT_DIRS protobuf/
        PROTOS ${PROTO_FILES}
)