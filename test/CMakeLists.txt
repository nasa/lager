# Fix for cmake versions without UPDATE_DISCONNECTED
# The update disconnected keeps cmake from re-downloading
# the repository every time you build
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

# Include GoogleTest
include_directories(
    ${googletest_SOURCE_DIR}/googletest/include
    ${googletest_SOURCE_DIR}/googlemock/include)

# Account for windows build types
if (WIN32)
    set(TEST_DIR ${CMAKE_BINARY_DIR}/test/${CMAKE_BUILD_TYPE})
else()
    set(TEST_DIR ${CMAKE_BINARY_DIR}/test)
endif()

# The test targets
add_executable(bartender_tests src/bartender_tests.cpp)
target_link_libraries(bartender_tests bartender gtest)
set_target_properties(bartender_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test)
add_test(NAME bartender_tests COMMAND bartender_tests WORKING_DIRECTORY ${TEST_DIR})

add_executable(chp_tests src/chp_tests.cpp)
target_link_libraries(chp_tests chp gtest)
set_target_properties(chp_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test)
add_test(NAME chp_tests COMMAND chp_tests WORKING_DIRECTORY ${TEST_DIR})

add_executable(data_format_tests src/data_format_tests.cpp)
target_link_libraries(data_format_tests 
    dataformat 
    gtest 
    ${ZeroMQ_LIBRARIES} 
    ${LIBUUID_LIBRARIES})
set_target_properties(data_format_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test)
add_test(NAME data_format_tests COMMAND data_format_tests WORKING_DIRECTORY ${TEST_DIR})

add_executable(forwarder_tests src/forwarder_tests.cpp)
target_link_libraries(forwarder_tests bartender gtest)
set_target_properties(forwarder_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test)
add_test(NAME forwarder_tests COMMAND forwarder_tests WORKING_DIRECTORY ${TEST_DIR})

add_executable(mug_tests src/mug_tests.cpp)
target_link_libraries(mug_tests mug gtest)
set_target_properties(mug_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test)
add_test(NAME mug_tests COMMAND mug_tests WORKING_DIRECTORY ${TEST_DIR})

add_executable(tap_tests src/tap_tests.cpp)
target_link_libraries(tap_tests tap gtest)
set_target_properties(tap_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test)
add_test(NAME tap_tests COMMAND tap_tests WORKING_DIRECTORY ${TEST_DIR})

add_executable(util_tests src/util_tests.cpp)
target_link_libraries(util_tests
    ${ZeroMQ_LIBRARIES} 
    ${LIBUUID_LIBRARIES}
    gtest)
target_compile_definitions(util_tests PRIVATE -DZMQ_VER="${ZeroMQ_VER}")
set_target_properties(util_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test)
add_test(NAME util_tests COMMAND util_tests WORKING_DIRECTORY ${TEST_DIR})

# Copy the test files
file(GLOB TEST_FILES share/*.xml share/*.xsd ../share/*.xsd)
foreach(TEST_FILE ${TEST_FILES})
    add_custom_command(TARGET data_format_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${TEST_FILE}" $<TARGET_FILE_DIR:data_format_tests>)
endforeach()

if (WIN32)
    # Adds pathing necessary in windows for ctest to find all the binaries and the xml files
    set(TEST_PATH "PATH=${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE};${TEST_DIR};$ENV{PATH}")
    set_tests_properties(bartender_tests PROPERTIES ENVIRONMENT "${TEST_PATH}")
    set_tests_properties(chp_tests PROPERTIES ENVIRONMENT "${TEST_PATH}")
    set_tests_properties(data_format_tests PROPERTIES ENVIRONMENT "${TEST_PATH}")
    set_tests_properties(forwarder_tests PROPERTIES ENVIRONMENT "${TEST_PATH}")
    set_tests_properties(mug_tests PROPERTIES ENVIRONMENT "${TEST_PATH}")
    set_tests_properties(tap_tests PROPERTIES ENVIRONMENT "${TEST_PATH}")
    set_tests_properties(util_tests PROPERTIES ENVIRONMENT "${TEST_PATH}")
endif()

# Code coverage (don't attempt to run in windows)
if (CMAKE_BUILD_TYPE MATCHES Coverage AND NOT WIN32)
    message(STATUS "Coverage Enabled")
    set(COVERAGE_WORKING_DIR ${TEST_DIR})
    set(COVERAGE_SCAN_PATH "${PROJECT_SOURCE_DIR}/src/*")
    include(CodeCoverage)
    coverage_add_exec(bartender_tests)
    coverage_add_exec(chp_tests)
    coverage_add_exec(data_format_tests)
    coverage_add_exec(mug_tests)
    coverage_add_exec(forwarder_tests)
    coverage_add_exec(tap_tests)
    coverage_add_exec(util_tests)
endif()
