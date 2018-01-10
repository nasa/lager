cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(lager)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package(Threads REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(LibUuid REQUIRED)

# Fix for cmake versions without UPDATE_DISCONNECTED
if (CMAKE_VERSION VERSION_LESS 3.2)
    set(UPDATE_DISCONNECTED_IF_AVAILABLE "")
else()
    set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")
endif()

# Get GoogleTest
include(DownloadProject)
download_project(
    PROJ googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.8.0
    ${UPDATE_DISCONNECTED_IF_AVAILABLE}
)

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Build GoogleTest
add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})

include_directories(
    ${ZeroMQ_INCLUDE_DIRS}
    ${LIBUUID_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/src
)

add_executable(bartender 
    ${PROJECT_SOURCE_DIR}/src/bartender.cpp
    ${PROJECT_SOURCE_DIR}/src/chp_server.cpp 
    ${PROJECT_SOURCE_DIR}/src/bartender_main.cpp
)

target_link_libraries(bartender 
    ${ZeroMQ_LIBRARIES}
    ${LIBUUID_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

add_executable(test_tap
    ${PROJECT_SOURCE_DIR}/src/tap.cpp
    ${PROJECT_SOURCE_DIR}/src/chp_client.cpp 
    ${PROJECT_SOURCE_DIR}/src/client_main.cpp
)

target_link_libraries(test_tap
    ${ZeroMQ_LIBRARIES}
    ${LIBUUID_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

# Copies ZMQ dll(s) to output directory under Windows
# Note older ZMQ builds may have different directory
# structures for the built libraries
if (WIN32)
    foreach(LIB ${ZeroMQ_LIBRARIES})
        if(LIB MATCHES ".lib")
            get_filename_component(LIB_WE ${LIB} NAME_WE)
            get_filename_component(LIB_DIR ${LIB} DIRECTORY)
            set(LIB_DLL "${LIB_DIR}/${LIB_WE}.dll")
            add_custom_command(TARGET client POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${LIB_DLL}" $<TARGET_FILE_DIR:client>)
        endif()
    endforeach()
endif()

# GoogleTest
enable_testing()
include_directories(${GTEST_INCLUDE_DIRS})
file(GLOB TEST_SRCS test/*.cpp)
add_executable(unit_tests ${TEST_SRCS})
target_link_libraries(unit_tests 
    ${ZeroMQ_LIBRARIES} 
    gtest
)
add_test(test_all unit_tests)
