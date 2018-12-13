

function(add_bench_helper bench_name source_file)
    set(TARGET_NAME bench_${bench_name})
    add_executable(${TARGET_NAME} ${source_file})
    target_link_libraries(${TARGET_NAME} benchmark)
    # uncomment to run bench as unit tests
#    add_test(NAME ${bench_name} COMMAND ${TARGET_NAME})
endfunction()


add_bench_helper(memory bench_main.cpp)
