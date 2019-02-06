# inputs and outputs
set(CPPCHECK_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/cppcheck.txt)
set(CPPCHECK_SUPPRESSIONS ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cppcheck-suppressions.txt)

find_program(CPPCHECK cppcheck)
if(CPPCHECK)
    message(STATUS "cppcheck found at ${CPPCHECK}")
    add_custom_command(
        OUTPUT ${CPPCHECK_OUTPUT}.touch
        COMMAND ${CPPCHECK}
            --enable=warning,style,information,performance,portability,missingInclude
            --template=\"{id}:{file}:{line}: {severity}: {message}\"
            --std=c++11
            --verbose
            --quiet
            --suppressions-list=${CPPCHECK_SUPPRESSIONS}
            src
            --error-exitcode=1
        # touch the "OUTPUT" file so it doesn't re-run unles dependencies change
        COMMAND ${CMAKE_COMMAND} -E touch ${CPPCHECK_OUTPUT}.touch
        DEPENDS ${BARTENDER_SRCS} ${TAP_SRCS} ${MUG_SRCS} ${KEG_SRCS} ${DATA_FORMAT_SRCS} ${CHP_SRCS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running cppcheck. Output:${CPPCHECK_OUTPUT}"
        )
else()
    message(FATAL_ERROR "cppcheck not found")
endif(CPPCHECK)
