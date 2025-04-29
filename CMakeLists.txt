cmake_minimum_required(VERSION 3.10)
project(CompilerCPP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/utf-8)
    add_definitions(-D_UNICODE -DUNICODE)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# 明确列出所有源文件
set(SOURCES
    src/main.cpp
    src/lexer/lexer.cpp
    src/parser/parser.cpp
    src/slr/slr.cpp
)

# 包含目录
include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

add_executable(compiler ${SOURCES})

if(WIN32)
    if(MSVC)
        target_compile_options(compiler PRIVATE "/utf-8")
    endif()
    add_custom_command(TARGET compiler POST_BUILD
        COMMAND powershell -Command "[Console]::OutputEncoding = [System.Text.Encoding]::UTF8"
        COMMENT "Setting console output to UTF-8"
    )
endif()