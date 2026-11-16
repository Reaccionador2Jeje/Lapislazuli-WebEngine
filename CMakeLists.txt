cmake_minimum_required(VERSION 3.20)
project(LapislazuliWebEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(lapislazuli
    engine/cpp/src/main.cpp
    engine/cpp/src/html_tokenizer.cpp
)
target_include_directories(lapislazuli PRIVATE engine/cpp/include)

if(MSVC)
    target_compile_options(lapislazuli PRIVATE /W4 /permissive-)
else()
    target_compile_options(lapislazuli PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
add_executable(html_tokenizer_tests
    tests/cpp/html_tokenizer_tests.cpp
    engine/cpp/src/html_tokenizer.cpp
)
target_include_directories(html_tokenizer_tests PRIVATE engine/cpp/include)
add_test(NAME html_tokenizer_tests COMMAND html_tokenizer_tests)
