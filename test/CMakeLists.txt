project(test)

add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})

# UNIT TESTS
cxx_executable(insertion_test unit gtest_main)
target_link_libraries(insertion_test fypMaps)

cxx_executable(erase_test unit gtest_main)
target_link_libraries(erase_test fypMaps)

cxx_executable(pool_test unit gtest_main)
target_link_libraries(pool_test fypMaps)

cxx_executable(allocator_test unit gtest_main)
target_link_libraries(allocator_test fypMaps)


# PERFORMANCE TESTS
option(STD "test std" OFF)
option(BOOST "test boost" OFF)
option(GOOGLE "test google" OFF)
option(FYP "test fyp" ON)

if(STD)
    add_definitions(-DSTD=1)
elseif(BOOST)
    #find_package(Boost REQUIRED)
    add_definitions(-DBOOST=1)
elseif(GOOGLE)
    add_definitions(-DGOOGLE=1)
elseif(FYP)
    add_definitions(-DFYP=1)
endif()

SET(BM_HEADS benchmarks/benchmark_utils.hpp)

# memory
add_executable(random_insert_mem benchmarks/random_insert_mem.cc ${BM_HEADS})
target_link_libraries(random_insert_mem fypMaps)

add_executable(sequential_insert_mem benchmarks/sequential_insert_mem.cc ${BM_HEADS})
target_link_libraries(sequential_insert_mem fypMaps)

#time
add_executable(random_insert_time benchmarks/random_insert_time.cc ${BM_HEADS})
target_link_libraries(random_insert_time fypMaps)

add_executable(random_search_time benchmarks/random_search_time.cc ${BM_HEADS})
target_link_libraries(random_search_time fypMaps)

add_executable(random_erase_time benchmarks/random_erase_time.cc ${BM_HEADS})
target_link_libraries(random_erase_time fypMaps)