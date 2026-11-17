cmake_minimum_required(VERSION 3.20)
project(LapislazuliConsumerSmokeTest LANGUAGES CXX)

find_package(Lapislazuli 0.9 CONFIG REQUIRED)

add_executable(lapislazuli_consumer main.cpp)
target_compile_features(lapislazuli_consumer PRIVATE cxx_std_20)
target_link_libraries(lapislazuli_consumer PRIVATE
    Lapislazuli::core
    Lapislazuli::browser
    Lapislazuli::render
    Lapislazuli::security
)
