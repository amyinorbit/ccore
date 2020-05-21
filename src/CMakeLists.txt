add_library(ccore STATIC list.c memory.c log.c table.c queue.c format.c value.c)
set_property(TARGET ccore PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(ccore PUBLIC $CMAKE_SOURCE_DIR/include)
