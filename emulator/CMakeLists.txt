cmake_minimum_required(VERSION 3.0.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

set(WARNING_FLAGS -Wall -Wextra -Wconversion
    -Wunreachable-code -Wuninitialized -pedantic-errors -Wold-style-cast
    -Wshadow -Wfloat-equal -Weffc++ -Wmost -Wno-dollar-in-identifier-extension)

project(HackEmulator)

include_directories(src/compiler)
include_directories(src/translator)
include_directories(src/assembler)
add_subdirectory(src/compiler compiler-build)
add_subdirectory(src/translator translator-build)
add_subdirectory(src/assembler assembler-build)

add_executable(HackEmulator src/main.cpp)
target_compile_options(HackEmulator PRIVATE ${WARNING_FLAGS})
set_target_properties(HackEmulator PROPERTIES LINK_FLAGS "\
        -s DEMANGLE_SUPPORT=1\
        -s TOTAL_MEMORY=128MB\
        -s ALLOW_MEMORY_GROWTH=1\
        -s EXPORTED_FUNCTIONS='[\"_CompileFile\", \"_SetMemoryPtr\",\
        \"_InitializeExecution\", \"_Execute\", \"_SetKey\", \"_Reset\",\
        \"_Clear\",\"_malloc\",\"_free\",\"_main\",\"_GetVmCodeString\",\
        \"_TranslateFile\",\"_GetAssemblyLength\",\"_GetAssembly\",\
        \"_Assemble\",\"_GetMachineCode\",\"_Init\"]'\
        -s EXTRA_EXPORTED_RUNTIME_METHODS='[\"ccall\", \"lengthBytesUTF8\",\
        \"stringToUTF8\"]'\
        -s DISABLE_EXCEPTION_CATCHING=0\
        --pre-js ${CMAKE_CURRENT_SOURCE_DIR}/src/prefix.js\
        --post-js ${CMAKE_CURRENT_SOURCE_DIR}/src/postfix.js")
target_link_libraries(HackEmulator PRIVATE compiler translator assembler)

add_custom_command(TARGET HackEmulator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE_DIR:HackEmulator>/HackEmulator.*
    ${CMAKE_CURRENT_SOURCE_DIR}/src)

