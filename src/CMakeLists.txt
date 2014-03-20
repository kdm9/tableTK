# Targets
add_library(ktable ktable.c)
add_executable(filterTable filter_table.c)
target_link_libraries(filterTable ktable)
add_executable(tableDist dist.c)
target_link_libraries(tableDist ktable m)
INSTALL(TARGETS filterTable DESTINATION "bin")
