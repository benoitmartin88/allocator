

function(add_test_helper test_name source_file)
    set(TARGET_NAME test_${test_name})
    add_executable(${TARGET_NAME} ${source_file})
    target_link_libraries(${TARGET_NAME} gtest gtest_main)
    add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
endfunction()

add_test_helper(stl_compliance test_stl_compliance.cpp)
add_test_helper(memory test_chained_block_allocator.cpp)
