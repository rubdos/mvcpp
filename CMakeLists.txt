project(mvcpp)

enable_testing()

find_package(Threads REQUIRED)

add_subdirectory(src)
add_subdirectory(tests)
