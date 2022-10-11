cmake_minimum_required(VERSION 3.16)
project(onvif-stream-url)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWITH_NONAMESPACES  -DWITH_DOM -DWITH_OPENSSL")
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(/usr/include)
add_definitions("-Wall -g ")
aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME}  ${SRC_LIST} soapStub.h)
target_link_libraries(${PROJECT_NAME}  -lcrypto -lssl)


