set(WARNING_FLAGS -Wall -Wextra -Wconversion
    -Wunreachable-code -Wuninitialized -pedantic-errors -Wold-style-cast
    -Wshadow -Wfloat-equal -Weffc++)

add_library(translator translator.cpp)
target_compile_options(translator PRIVATE ${WARNING_FLAGS})

