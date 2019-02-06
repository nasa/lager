####
# Code Coverage (lcov) reporting targets and functions.
# Add project execs to be covered with `coverage_add_exec()`
# Add project targets to be covered with `coverage_add_target()`
#
# Generate coverage report by calling the `run_coverage` target.
# Reports generated in the `coverage` directory in the build directory.
#
# Requires the use of -DCMAKE_BUILD_TYPE=Coverage
####

# Set up program paths and compiler settings

find_program(GCOV_PATH gcov)
find_program(LCOV_PATH lcov)
find_program(GENHTML_PATH genhtml)

if (NOT ${CMAKE_BUILD_TYPE} STREQUAL "Coverage")
    message(ERROR "Code coverage results will fail without a Coverage build-type.")
endif()

set(CMAKE_CXX_FLAGS_COVERAGE
    "-g -O0 --coverage -fprofile-arcs -ftest-coverage"
    CACHE STRING "Flags used by the C++ compiler during coverage builds."
    FORCE
)
set(CMAKE_C_FLAGS_COVERAGE
    "-g -O0 --coverage -fprofile-arcs -ftest-coverage"
    CACHE STRING "Flags used by the C compiler during coverage builds."
    FORCE
)
set(CMAKE_EXE_LINKER_FLAGS_COVERAGE
    ""
    CACHE STRING "Flags used for linking binaries during coverage builds."
    FORCE
)
set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE
    ""
    CACHE STRING "Flags used by the shared libraries linker during coverage builds."
    FORCE
)
mark_as_advanced(
    CMAKE_CXX_FLAGS_COVERAGE
    CMAKE_C_FLAGS_COVERAGE
    CMAKE_EXE_LINKER_FLAGS_COVERAGE
    CMAKE_SHARED_LINKER_FLAGS_COVERAGE
)

set(COVERAGE_DIR ${CMAKE_BINARY_DIR}/coverage)

# User settable options

if(NOT COVERAGE_SCAN_FILTER)
    set(COVERAGE_SCAN_FILTER "${PROJECT_SOURCE_DIR}/*")
endif()

if(NOT COVERAGE_WORKING_DIR)
    set(COVERAGE_WORKING_DIR "${COVERAGE_DIR}")
endif()

# Set up coverage targets

add_custom_target(${PROJECT_NAME}_coverage_dir
    COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_DIR}
)

add_custom_target(${PROJECT_NAME}_coverage_prep
    COMMAND ${LCOV_PATH} --quiet --directory ${CMAKE_BINARY_DIR} --zerocounters

    WORKING_DIRECTORY ${COVERAGE_WORKING_DIR}
    DEPENDS ${PROJECT_NAME}_coverage_dir
)

add_custom_target(${PROJECT_NAME}_coverage_exec)

add_custom_target(run_coverage
    COMMAND ${LCOV_PATH} --directory ${CMAKE_BINARY_DIR} --base-directory ${PROJECT_SOURCE_DIR} --capture --output-file ${PROJECT_NAME}.info
    COMMAND ${LCOV_PATH} --extract ${PROJECT_NAME}.info ${COVERAGE_SCAN_FILTER} --no-external --output-file ${PROJECT_NAME}.info.cleaned
    COMMAND ${GENHTML_PATH} -o ${COVERAGE_DIR} --show-details --legend ${PROJECT_NAME}.info.cleaned
    COMMAND ${CMAKE_COMMAND} -E remove ${PROJECT_NAME}.info ${PROJECT_NAME}.info.cleaned
    COMMAND ${CMAKE_COMMAND} -E echo "Coverage report found in: ${COVERAGE_DIR}"
 
    WORKING_DIRECTORY ${COVERAGE_WORKING_DIR}
    DEPENDS ${PROJECT_NAME}_coverage_exec
)

# Functions for adding executables to the coverage analysis

function(coverage_add_exec exec)
    add_custom_target(${PROJECT_NAME}_coverage_exec_${exec}
        COMMAND ${exec}
        WORKING_DIRECTORY ${COVERAGE_WORKING_DIR}
        DEPENDS ${PROJECT_NAME}_coverage_prep ${exec}
    )
    add_dependencies(${PROJECT_NAME}_coverage_exec ${PROJECT_NAME}_coverage_exec_${exec})
endfunction()

function(coverage_add_target tgt)
    add_dependencies(${tgt} ${PROJECT_NAME}_coverage_prep)
    add_dependencies(${PROJECT_NAME}_coverage_exec ${tgt})
endfunction()
