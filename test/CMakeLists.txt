#CFLAGS
add_executable(test_ft test.c tinytest.c)
#target_link_libraries(test_ft )

add_test(NAME test_ft COMMAND test_ft)

add_custom_command(TARGET test_ft COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_CURRENT_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/data)
add_custom_command(TARGET test_ft COMMAND ${CMAKE_COMMAND} -E copy
	${CMAKE_CURRENT_SOURCE_DIR}/test.sh ${CMAKE_BINARY_DIR}/test.sh)
