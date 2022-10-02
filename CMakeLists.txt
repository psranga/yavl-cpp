cmake_minimum_required( VERSION 3.18.2 )

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(yavl_cpp LANGUAGES CXX)

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.16.1/conan.cmake"
            "${CMAKE_SOURCE_DIR}/conan.cmake"
            EXPECTED_HASH SHA256=396e16d0f5eabdc6a14afddbcfff62a54a7ee75c6da23f32f7a31bc85db23484
            TLS_VERIFY ON)
endif()

include(${CMAKE_SOURCE_DIR}/conan.cmake)

conan_cmake_run(REQUIRES yaml-cpp/0.7.0
                GENERATORS cmake_find_package
                BASIC_SETUP CMAKE_TARGETS UPDATE
                PROFILE default
                BUILD missing)

add_library(yavl_cpp STATIC src/yavl.cpp src/yavl.h)
target_link_libraries(yavl_cpp PUBLIC CONAN_PKG::yaml-cpp)

add_executable(checker example-code/checker.cpp)
target_include_directories(checker PRIVATE src/)
target_link_libraries(checker PUBLIC yavl_cpp)

install(TARGETS checker DESTINATION "bin")
install(FILES src/yavl.h DESTINATION "include/yavl")
install(TARGETS yavl_cpp DESTINATION "lib")
