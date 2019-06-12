add_executable(example-code checker.cpp)

target_compile_options(example-code PUBLIC -Werror -Wall -O3 -Wextra -pedantic -Wold-style-cast
        -Wdouble-promotion -Wuninitialized -Wshadow -Wuseless-cast)

target_link_libraries(example-code PUBLIC yavl_cpp)