################################################################################
#   TWins library - unit tests
#   (c) 2020-2026 Mariusz Midor
#   https://bitbucket.org/marmidr/twins/
################################################################################

set(TARGETNAME TWinsUT)

add_executable(${TARGETNAME}
    src/test_ansi_esc_decoder.cpp
    src/test_cli.cpp
    src/test_input_posix.cpp
    src/test_main.cpp
    src/test_map.cpp
    src/test_queue.cpp
    src/test_secure_psw.cpp
    src/test_stack.cpp
    src/test_string.cpp
    src/test_twins.cpp
    src/test_utils.cpp
    src/test_vector.cpp
    src/test_widget.cpp
)

target_compile_options(${TARGETNAME} PRIVATE
    ${CXX_FLAGS}
    --coverage
    ${GCC_SANITIZERS}
)

target_link_libraries(${TARGETNAME}
    twins
    gtest
    gmock
)

if(NOT ${CMAKE_COLOR_MAKEFILE})
    target_compile_definitions(${TARGETNAME}
        PUBLIC
        GTEST_NOCOLOR
    )
endif()

# FIXME: causes problems when used with VSCode
# if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
#     # https://cmake.org/cmake/help/v3.19/command/target_precompile_headers.html
#     # 51s -> 40s
#     target_precompile_headers(${TARGETNAME}
#         PRIVATE
#             <string>
#             <vector>
#             <gtest/gtest.h>
#             <gmock/gmock.h>
#     )
# endif()

# ###############################################################################

# ignore external dependencies
set(GCOVR_EXCLUDES
    -e ".*/googletest/.*"
    -e ".*/test_.*cpp"
    -e ".*/twins_window_state.hpp"
)

set(GCOVR_PROJECT_TILE "GCOVR :: TWins library Coverage")

add_custom_target(twins_cover_gen
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND sh -c "mkdir -p coverage_report"

    # https://gcovr.com/en/stable/manpage.html
    COMMAND echo "~~ Generate HTML report..."
    COMMAND gcovr -r ${PROJECT_SOURCE_DIR}
                -s ${GCOVR_EXCLUDES}
                --html-title ${GCOVR_PROJECT_TILE}
                --txt-metric branch
                --html-details
                -o coverage_report/cover.html
)

# remove coverage data created during test-run
add_custom_target(twins_cover_clean
    COMMAND echo "~~ Remove gcov coverage data files..."
    COMMAND find ${CMAKE_BINARY_DIR} -name *.gcda -delete
)

# create Cobertura report
add_custom_target(twins_cobertura_gen
    COMMAND echo "~~ Generate COBERTURA report..."
    COMMAND gcovr -r ${PROJECT_SOURCE_DIR}
                -s ${GCOVR_EXCLUDES}
                --cobertura coverage_report_cobertura.xml
                --cobertura-pretty
                --txt-metric branch
)

# ###############################################################################

option(TWINS_SEPARATE_TESTS "Run each of the test group separately" ON)

# add test with filter to run every group of tests separately
macro(add_subtest SUBTEST_NAME)
    add_test(NAME ${SUBTEST_NAME} COMMAND $<TARGET_FILE:${TARGETNAME}> --gtest_filter=${SUBTEST_NAME}.*)
endmacro()

if (TWINS_SEPARATE_TESTS)
    add_subtest(ANSI_INPUTDECODER)
    add_subtest(CLI)
    add_subtest(INP_POSIX)
    add_subtest(MAP)
    add_subtest(QUEUE)
    add_subtest(SECURE_PSW)
    add_subtest(STACK)
    add_subtest(STRING)
    add_subtest(STRINGBUFF)
    add_subtest(TWINS)
    add_subtest(UTILS)
    add_subtest(VECTOR)
    add_subtest(WIDGET)
else()
    # GitHub and BitBucket pipelines reports 'pure virtual method called' on filtered test run,
    # so define a combined test for these cases
    add_test(NAME TWins-TestAll COMMAND $<TARGET_FILE:${TARGETNAME}>)
endif()
