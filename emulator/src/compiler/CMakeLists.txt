set(WARNING_FLAGS -Wall -Wextra -Wconversion
    -Wunreachable-code -Wuninitialized -pedantic-errors -Wold-style-cast
    -Wshadow -Wfloat-equal -Weffc++)

add_library(compiler
    classnode.h classnode.cpp
    declarationnode.h declarationnode.cpp
    donode.h donode.cpp
    expression.h expression.cpp
    ifnode.h ifnode.cpp
    letnode.h letnode.cpp
    node.h node.cpp
    parser.h parser.cpp
    returnnode.h returnnode.cpp
    subroutinecall.h subroutinecall.cpp
    subroutinenode.h subroutinenode.cpp
    term.h term.cpp
    tokenizer.h tokenizer.cpp
    vmcommand.h vmcommand.cpp
    whilenode.h whilenode.cpp
)
target_compile_options(compiler PRIVATE ${WARNING_FLAGS})

