cmake_minimum_required(VERSION 3.31)
project(GravitationalRayMarchingProtobuf CXX)

find_package(protobuf CONFIG REQUIRED)

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