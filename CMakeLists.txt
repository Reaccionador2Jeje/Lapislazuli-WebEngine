cmake_minimum_required(VERSION 3.20)
project(LapislazuliWebEngine VERSION 0.9.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(CURL REQUIRED)
find_package(SQLite3 REQUIRED)

function(lapislazuli_configure_target target export_name)
    target_include_directories(${target}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/engine/cpp/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
    set_target_properties(${target} PROPERTIES EXPORT_NAME ${export_name})
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

add_library(lapislazuli_dom engine/cpp/src/dom.cpp engine/cpp/src/dom_parser.cpp engine/cpp/src/html_tokenizer.cpp)
lapislazuli_configure_target(lapislazuli_dom dom)

add_library(lapislazuli_css engine/cpp/src/css/css_engine.cpp)
lapislazuli_configure_target(lapislazuli_css css)
target_link_libraries(lapislazuli_css PUBLIC lapislazuli_dom)

add_library(lapislazuli_layout engine/cpp/src/layout/layout_engine.cpp)
lapislazuli_configure_target(lapislazuli_layout layout)
target_link_libraries(lapislazuli_layout PUBLIC lapislazuli_css lapislazuli_dom)

add_library(lapislazuli_js engine/cpp/src/js/runtime.cpp)
lapislazuli_configure_target(lapislazuli_js js)
target_link_libraries(lapislazuli_js PUBLIC lapislazuli_dom)

add_library(lapislazuli_net engine/cpp/src/net/http_client.cpp)
lapislazuli_configure_target(lapislazuli_net net)
target_link_libraries(lapislazuli_net PUBLIC CURL::libcurl)

add_library(lapislazuli_storage engine/cpp/src/storage/storage_area.cpp)
lapislazuli_configure_target(lapislazuli_storage storage)
target_link_libraries(lapislazuli_storage PUBLIC SQLite3::SQLite3)

add_library(lapislazuli_browser engine/cpp/src/browser/engine.cpp)
lapislazuli_configure_target(lapislazuli_browser browser)
target_link_libraries(lapislazuli_browser PUBLIC lapislazuli_net lapislazuli_dom lapislazuli_js lapislazuli_css)

# Aggregate target for embedders that want the complete native SDK.
add_library(lapislazuli_core engine/cpp/src/core.cpp)
lapislazuli_configure_target(lapislazuli_core core)
target_link_libraries(lapislazuli_core
    PUBLIC
        lapislazuli_browser
        lapislazuli_css
        lapislazuli_js
        lapislazuli_layout
        lapislazuli_net
        lapislazuli_storage
)
add_library(Lapislazuli::core ALIAS lapislazuli_core)
add_library(Lapislazuli::dom ALIAS lapislazuli_dom)
add_library(Lapislazuli::css ALIAS lapislazuli_css)
add_library(Lapislazuli::layout ALIAS lapislazuli_layout)
add_library(Lapislazuli::js ALIAS lapislazuli_js)
add_library(Lapislazuli::net ALIAS lapislazuli_net)
add_library(Lapislazuli::storage ALIAS lapislazuli_storage)
add_library(Lapislazuli::browser ALIAS lapislazuli_browser)

add_executable(lapislazuli engine/cpp/src/main.cpp)
target_link_libraries(lapislazuli PRIVATE lapislazuli_core)
lapislazuli_configure_target(lapislazuli cli)

enable_testing()

add_executable(html_tokenizer_tests tests/cpp/html_tokenizer_tests.cpp)
target_link_libraries(html_tokenizer_tests PRIVATE lapislazuli_dom)
add_test(NAME html_tokenizer_tests COMMAND html_tokenizer_tests)

add_executable(dom_tests tests/cpp/dom_tests.cpp)
target_link_libraries(dom_tests PRIVATE lapislazuli_dom)
add_test(NAME dom_tests COMMAND dom_tests)

add_executable(css_engine_tests tests/cpp/css_engine_tests.cpp)
target_link_libraries(css_engine_tests PRIVATE lapislazuli_css)
add_test(NAME css_engine_tests COMMAND css_engine_tests)

add_executable(layout_engine_tests tests/cpp/layout_engine_tests.cpp)
target_link_libraries(layout_engine_tests PRIVATE lapislazuli_layout)
add_test(NAME layout_engine_tests COMMAND layout_engine_tests)

add_executable(js_runtime_tests tests/cpp/js_runtime_tests.cpp)
target_link_libraries(js_runtime_tests PRIVATE lapislazuli_js)
add_test(NAME js_runtime_tests COMMAND js_runtime_tests)

add_executable(document_script_tests tests/cpp/document_script_tests.cpp)
target_link_libraries(document_script_tests PRIVATE lapislazuli_browser)
add_test(NAME document_script_tests COMMAND document_script_tests)

add_executable(resource_loader_tests tests/cpp/resource_loader_tests.cpp)
target_link_libraries(resource_loader_tests PRIVATE lapislazuli_browser)
add_test(NAME resource_loader_tests COMMAND resource_loader_tests)

add_executable(network_tests tests/cpp/network_tests.cpp)
target_link_libraries(network_tests PRIVATE lapislazuli_net)
add_test(NAME network_tests COMMAND network_tests)

add_executable(storage_area_tests tests/cpp/storage_area_tests.cpp)
target_link_libraries(storage_area_tests PRIVATE lapislazuli_storage)
add_test(NAME storage_area_tests COMMAND storage_area_tests)

add_executable(core_tests tests/cpp/core_tests.cpp)
target_link_libraries(core_tests PRIVATE Lapislazuli::core)
add_test(NAME core_tests COMMAND core_tests)

# Installable CMake SDK package for downstream browser projects.
install(TARGETS
    lapislazuli_core
    lapislazuli_dom
    lapislazuli_css
    lapislazuli_layout
    lapislazuli_js
    lapislazuli_net
    lapislazuli_storage
    lapislazuli_browser
    EXPORT LapislazuliTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY engine/cpp/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT LapislazuliTargets
    FILE LapislazuliTargets.cmake
    NAMESPACE Lapislazuli::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Lapislazuli
)

configure_package_config_file(
    cmake/LapislazuliConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/LapislazuliConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Lapislazuli
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/LapislazuliConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/LapislazuliConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/LapislazuliConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Lapislazuli
)
