cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(lager)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package(Threads REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(LibUuid REQUIRED)
find_package(XercesC REQUIRED)

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
    ${UPDATE_DISCONNECTED_IF_AVAILABLE})

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Build GoogleTest
add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)

include_directories(
    ${ZeroMQ_INCLUDE_DIRS}
    ${LIBUUID_INCLUDE_DIRS}
    ${XercesC_INCLUDE_DIRS}
    ${SRC_DIR})

# Sources (@todo make this a library later)
set(BARTENDER_SRCS
    ${SRC_DIR}/bartender.cpp
    ${SRC_DIR}/chp_server.cpp 
    ${SRC_DIR}/forwarder.cpp)

set(TAP_SRCS
    ${SRC_DIR}/tap.cpp
    ${SRC_DIR}/chp_client.cpp)

set(MUG_SRCS
    ${SRC_DIR}/mug.cpp
    ${SRC_DIR}/chp_client.cpp)

set(DATA_FORMAT_SRCS
    ${SRC_DIR}/data_format.cpp
    ${SRC_DIR}/data_format_parser.cpp)

# Targets for bartender and test tap
add_executable(bartender 
    ${BARTENDER_SRCS}
    ${SRC_DIR}/bartender_main.cpp)

target_link_libraries(bartender 
    ${ZeroMQ_LIBRARIES}
    ${LIBUUID_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT})

add_executable(test_tap
    ${TAP_SRCS}
    ${SRC_DIR}/tap_test_main.cpp)

target_link_libraries(test_tap
    ${ZeroMQ_LIBRARIES}
    ${LIBUUID_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT})

add_executable(test_mug
    ${MUG_SRCS}
    ${SRC_DIR}/mug_test_main.cpp)

target_link_libraries(test_mug
    ${ZeroMQ_LIBRARIES}
    ${LIBUUID_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT})

# Copy format schema and test files
add_custom_command(
    TARGET bartender POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/share/data_format.xsd 
        $<TARGET_FILE_DIR:bartender>)

add_custom_command(
    TARGET bartender POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/share/sample_format.xml 
        $<TARGET_FILE_DIR:bartender>)

# Copies dll(s) to output directory under Windows
# Note older ZMQ builds may have different directory
# structures for the built libraries.  May need to adjust
# Windows requirements or make this more able to handle
# different versions.
if (WIN32)
    foreach(LIB ${ZeroMQ_LIBRARIES})
        if(LIB MATCHES ".lib")
            get_filename_component(LIB_WE ${LIB} NAME_WE)
            get_filename_component(LIB_DIR ${LIB} DIRECTORY)
            set(LIB_DLL "${LIB_DIR}/${LIB_WE}.dll")
            add_custom_command(TARGET bartender POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${LIB_DLL}" $<TARGET_FILE_DIR:bartender>)
        endif()
    endforeach()
    foreach(LIB ${XercesC_LIBRARIES})
        if(LIB MATCHES ".lib")
            get_filename_component(LIB_DIR ${LIB} DIRECTORY)
            string(REGEX REPLACE "lib" "bin" LIB_DIR ${LIB_DIR})
            file(GLOB XercesC_BINARIES "${LIB_DIR}/*.dll")
            foreach(XercesC_BINARY ${XercesC_BINARIES})
                add_custom_command(TARGET bartender POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${XercesC_BINARY}" $<TARGET_FILE_DIR:bartender>)
            endforeach()
        endif()
    endforeach()    
endif()

# GoogleTest
enable_testing()
include_directories(${GTEST_INCLUDE_DIRS})
file(GLOB TEST_SRCS test/*.cpp)
add_executable(unit_tests 
    ${TEST_SRCS}
    ${BARTENDER_SRCS}
    ${TAP_SRCS}
    ${DATA_FORMAT_SRCS})
target_compile_definitions(unit_tests PRIVATE -DZMQ_VER="${ZeroMQ_VER}")
target_link_libraries(unit_tests 
    ${ZeroMQ_LIBRARIES} 
    ${XercesC_LIBRARIES}
    ${LIBUUID_LIBRARIES}
    gtest)
set_target_properties(unit_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test)
add_test(NAME test_all COMMAND unit_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test)

file(GLOB TEST_FILES test/*.xml test/*.xsd)
foreach(TEST_FILE ${TEST_FILES})
    add_custom_command(TARGET unit_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${TEST_FILE}" $<TARGET_FILE_DIR:unit_tests>)
endforeach()

# Code coverage
if (CMAKE_BUILD_TYPE MATCHES Debug)
    include(CodeCoverage)
    setup_target_for_coverage(coverage unit_tests ${CMAKE_BINARY_DIR}/test coverage)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
endif()
