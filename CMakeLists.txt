cmake_minimum_required(VERSION 3.5)
project(FYP_MAPS CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/dirtyMap/*.hpp)

add_library(fypMaps INTERFACE)
target_sources(fypMaps INTERFACE ${HEADERS})
target_include_directories(fypMaps INTERFACE ${CMAKE_SOURCE_DIR})

option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    set(BUILD_GMOCK OFF)
    set(BUILD_GTEST ON)
    # see https://github.com/Crascit/DownloadProject
    set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")
    include(DownloadProject.cmake)
    download_project(
            PROJ                googletest
            GIT_REPOSITORY      https://github.com/google/googletest.git
            GIT_TAG             master
            ${UPDATE_DISCONNECTED_IF_AVAILABLE}
    )

    add_subdirectory(test)
endif()
