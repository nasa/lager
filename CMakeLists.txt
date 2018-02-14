cmake_minimum_required(VERSION 3.1.3 FATAL_ERROR)
project(lager)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package(Threads REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(LibUuid REQUIRED)
find_package(XercesC REQUIRED)

option(BUILD_TESTS "Build unit tests" ON)

if (WIN32)
    # Automatically creates the module definition files in windows
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

    # because cmake in windows doesn't set build type unless you explicitly set it
    if(NOT CMAKE_BUILD_TYPE)
        message(STATUS "Setting default build type to Debug")
        set(CMAKE_BUILD_TYPE Debug)
    endif()
endif()

# TODO switch to use target_include_dir when conversion to library is made
include_directories(
    ${ZeroMQ_INCLUDE_DIRS}
    ${LIBUUID_INCLUDE_DIRS}
    ${XercesC_INCLUDE_DIRS}
    include
    src)

# Sources 
set(BARTENDER_SRCS
    src/bartender.cpp
    src/forwarder.cpp)

set(TAP_SRCS
    src/tap.cpp)

set(MUG_SRCS
    src/mug.cpp)

set(KEG_SRCS
    src/keg.cpp)

set(DATA_FORMAT_SRCS
    src/data_format.cpp
    src/data_format_parser.cpp)

set(CHP_SRCS
    src/chp_server.cpp
    src/chp_client.cpp)

# libraries
add_library(chp SHARED ${CHP_SRCS})
target_link_libraries(chp
    ${LIBUUID_LIBRARIES}
    ${ZeroMQ_LIBRARIES}    
    ${CMAKE_THREAD_LIBS_INIT})

add_library(dataformat SHARED ${DATA_FORMAT_SRCS})
target_link_libraries(dataformat ${XercesC_LIBRARIES})

add_library(keg SHARED ${KEG_SRCS})
target_link_libraries(keg dataformat chp)

add_library(bartender SHARED ${BARTENDER_SRCS})
target_link_libraries(bartender chp)

add_library(tap SHARED ${TAP_SRCS} ${DATA_FORMAT_SRCS})
target_link_libraries(tap dataformat chp)

add_library(mug SHARED ${MUG_SRCS})
target_link_libraries(mug chp keg dataformat)

# Targets for bartender and test apps
# TODO remove these later
add_executable(test_bartender src/bartender_main.cpp)
target_link_libraries(test_bartender bartender)

add_executable(test_tap src/tap_test_main.cpp)
target_link_libraries(test_tap tap)

add_executable(test_mug src/mug_test_main.cpp)
target_link_libraries(test_mug mug)

# Copy format schema and test files
# TODO later will install the schema file and not copy sample formats
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
            set(ZMQ_BINARIES ${ZMQ_BINARIES} ${LIB_DLL})
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

# cppcheck
if (NOT WIN32)
    include(cppcheck)
    add_custom_target(cppcheck DEPENDS ${CPPCHECK_OUTPUT}.touch)
endif()

# testing
if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
