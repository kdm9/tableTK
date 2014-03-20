# Targets
add_library(ktable ktable.c)
add_executable(filterTable filter_table.c)
target_link_libraries(filterTable ktable)
INSTALL(TARGETS filterTable DESTINATION "bin")
