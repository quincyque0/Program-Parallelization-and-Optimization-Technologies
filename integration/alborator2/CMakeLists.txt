cmake_minimum_required(VERSION 3.20)
project(lab2 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


if(APPLE)
    execute_process(
        COMMAND brew --prefix libomp
        OUTPUT_VARIABLE HOMEBREW_LIBOMP_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(HOMEBREW_LIBOMP_PREFIX)
        set(OpenMP_ROOT "${HOMEBREW_LIBOMP_PREFIX}")
    endif()
endif()

find_package(OpenMP REQUIRED)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE OpenMP::OpenMP_CXX)