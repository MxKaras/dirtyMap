cmake_minimum_required(VERSION 3.5)
project(FYP_MAPS CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_library(fypMaps INTERFACE)
target_include_directories(
        fypMaps INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/fypMaps>
)

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
