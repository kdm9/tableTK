#CFLAGS
add_executable(test test.c tinytest.c)
#target_link_libraries(test )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

add_test(NAME test COMMAND test)
