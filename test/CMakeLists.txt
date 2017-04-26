project(test)

add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})

# UNIT TESTS
cxx_executable(insertion_test unit gtest_main)
target_link_libraries(insertion_test fypMaps)


# PERFORMANCE TESTS
option(STD "test std" OFF)
option(BOOST "test boost" OFF)
option(GOOGLE "test google" OFF)
option(FYP "test fyp" OFF)
option(FYP-POOL "test fyp (pooled)" ON)

if(STD)
    add_definitions(-DSTD=1)
elseif(BOOST)
    #find_package(Boost REQUIRED)
    add_definitions(-DBOOST=1)
elseif(GOOGLE)
    add_definitions(-DGOOGLE=1)
elseif(FYP)
    add_definitions(-DFYP=1)
elseif(FYP-POOL)
    add_definitions(-DFYP_POOL=1)
    add_definitions(-DPOOL_SIZE=1000)
endif()

# memory
add_executable(random_insert_mem benchmarks/random_insert_mem.cc)
target_link_libraries(random_insert_mem fypMaps)

add_executable(sequential_insert_mem benchmarks/sequential_insert_mem.cc)
target_link_libraries(sequential_insert_mem fypMaps)

#time
add_executable(random_insert_time benchmarks/random_insert_time.cc)
target_link_libraries(random_insert_time fypMaps)

add_executable(random_search_time benchmarks/random_search_time.cc)
target_link_libraries(random_search_time fypMaps)

add_executable(random_erase_time benchmarks/random_erase_time.cc)
target_link_libraries(random_erase_time fypMaps)