add_library(yavl_cpp
        yavl.cpp
        yavl.h)

target_compile_options(yavl_cpp PUBLIC -Werror -Wall -O3 -Wextra -pedantic -Wold-style-cast
        -Wdouble-promotion -Wuninitialized -Wshadow -Wuseless-cast)

target_include_directories(yavl_cpp PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(yavl_cpp PUBLIC yaml-cpp)