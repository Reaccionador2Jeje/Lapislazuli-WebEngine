cmake_minimum_required(VERSION 3.20)
project(LapislazuliWebEngine VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(lapislazuli_dom
    engine/cpp/src/dom.cpp
    engine/cpp/src/dom_parser.cpp
    engine/cpp/src/html_tokenizer.cpp
)
target_include_directories(lapislazuli_dom PUBLIC engine/cpp/include)

add_executable(lapislazuli
    engine/cpp/src/main.cpp
)
target_link_libraries(lapislazuli PRIVATE lapislazuli_dom)

if(MSVC)
    target_compile_options(lapislazuli_dom PRIVATE /W4 /permissive-)
    target_compile_options(lapislazuli PRIVATE /W4 /permissive-)
else()
    target_compile_options(lapislazuli_dom PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(lapislazuli PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
add_executable(html_tokenizer_tests tests/cpp/html_tokenizer_tests.cpp)
target_link_libraries(html_tokenizer_tests PRIVATE lapislazuli_dom)
add_test(NAME html_tokenizer_tests COMMAND html_tokenizer_tests)

add_executable(dom_tests tests/cpp/dom_tests.cpp)
target_link_libraries(dom_tests PRIVATE lapislazuli_dom)
add_test(NAME dom_tests COMMAND dom_tests)
