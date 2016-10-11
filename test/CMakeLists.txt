add_library(test_utils test_utils.c)
target_include_directories(test_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_include_directories(test_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(test_pager test_pager.c)
target_include_directories(test_pager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_include_directories(test_pager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_pager LINK_PUBLIC pager test_utils)

add_executable(test_hash test_hash.c)
target_include_directories(test_hash PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_include_directories(test_hash PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_hash LINK_PUBLIC hash test_utils)
