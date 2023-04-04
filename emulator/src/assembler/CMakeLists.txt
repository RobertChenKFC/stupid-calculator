set(WARNING_FLAGS -Wall -Wextra -Wconversion
    -Wunreachable-code -Wuninitialized -pedantic-errors -Wold-style-cast
    -Wshadow -Wfloat-equal -Weffc++)

add_library(assembler assembler.cpp)
target_compile_options(assembler PRIVATE ${WARNING_FLAGS})

