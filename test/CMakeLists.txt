# test/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# Define a function to add tests
function(add_pool_test test_name test_source)
    add_executable(${test_name} ${test_source})
    target_include_directories(${test_name} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(${test_name} PRIVATE threadpool)
endfunction()

# Add tests
add_pool_test(test_day1_basic test1.cpp)
add_pool_test(test_day2_basic test2.cpp)
add_pool_test(test_day3_basic test3.cpp)