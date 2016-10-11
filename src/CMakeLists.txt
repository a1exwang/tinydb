add_library(pager pager.c)
target_link_libraries(pager hash)
target_include_directories(pager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


add_library(hash hash.c)
target_include_directories(hash PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
