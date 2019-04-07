cmake_minimum_required(VERSION 3.13)
project(dsp_lib)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_library(dsp_lib SHARED
        dsp_algos.cpp
        dsp_algos.h
        wsfir.cpp
        wsfir.h wav_h_gen.cpp wav_h_gen.h )

add_executable(dsp_lib_test test.cpp)
target_link_libraries(dsp_lib_test dl)
target_compile_options(dsp_lib PRIVATE -O3 -Wno-undef)

install(TARGETS dsp_lib
        dsp_lib_test
        PUBLIC_HEADER RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/lib
        LIBRARY DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/lib)
