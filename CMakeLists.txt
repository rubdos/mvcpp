project(mvcpp CXX)
cmake_minimum_required(VERSION 2.8)

enable_testing()

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
CHECK_CXX_COMPILER_FLAG("-std=c++1y" COMPILER_SUPPORTS_CXX1Y)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
elseif(COMPILER_SUPPORTS_CXX1Y)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support. Please use a different C++ compiler.")
endif()

find_package(Threads REQUIRED)

set(LINK_LIBS
    ${CMAKE_THREAD_LIBS_INIT})

include_directories(include)

add_subdirectory(src)
add_subdirectory(tests)

file(COPY tests/testapplication/static
    DESTINATION ${CMAKE_BINARY_DIR}/tests/testapplication)
file(COPY tests/testapplication/views 
    DESTINATION ${CMAKE_BINARY_DIR}/tests/testapplication)
