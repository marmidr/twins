################################################################################
#   TWins library test application
#   (c) 2020-2025 Mariusz Midor
#   https://bitbucket.org/marmidr/twins/
#   https://github.com/marmidr/twins/
################################################################################

cmake_minimum_required(VERSION 3.15)

# https://stackoverflow.com/questions/42716466/how-can-i-tell-within-a-cmakelists-txt-whether-its-used-with-add-subdirectory
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # message("I am top-level project.")
    project(TWins LANGUAGES CXX)

    # https://stackoverflow.com/questions/6594796/how-do-i-make-cmake-output-into-a-bin-dir
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

    # specify the C++ standard
    set(CMAKE_CXX_STANDARD  14)
    set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

    # local C++ flags (list, not string)
    # https://gcc.gnu.org/onlinedocs/gcc-10.2.0/gcc/Warning-Options.html#Warning-Options
    set(CXX_FLAGS -Wall
        -Wcast-align
        # -Wcast-qual
        -Wextra
        # -Winline # genereates warnings on higher optimization levels
        -Wmissing-include-dirs
        -Wredundant-decls
        -Wshadow
        -Wunused-parameter
        -Wwrite-strings
    )

    # https://stackoverflow.com/questions/10046114/in-cmake-how-can-i-test-if-the-compiler-is-clang
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        list(APPEND CXX_FLAGS -Wno-gnu-designator)
    endif()

    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
    endif()

    # message("CXX_FLAGS => ${CXX_FLAGS}")
else()
    # message("I am called from other project with add_subdirectory().")
endif()

# Help:
# https://cmake.org/cmake/help/latest/manual/cmake-commands.7.html
# https://cmake.org/cmake/help/latest/manual/cmake-variables.7.html

# Debug messages:
# message("Features=${CMAKE_CXX_COMPILE_FEATURES}")
# message("CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}")
# message("CTEST_BINARY_DIRECTORY=${CTEST_BINARY_DIRECTORY}")
# message("PROJECT_BINARY_DIR=${PROJECT_BINARY_DIR}")


# https://medium.com/@alasher/colored-c-compiler-output-with-ninja-clang-gcc-10bfe7f2b949
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-fdiagnostics-color=always)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-fcolor-diagnostics)
endif ()

# by default, we want to have 'compile_commands.json' generated
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Generate JSON with compile commands" FORCE)

# build target switches
option(TWINS_BUILD_UT "Build TWins Unit Tests" OFF)
option(TWINS_BUILD_DEMO "Build TWins Demo App" OFF)


if (TWINS_BUILD_UT)
    # enable CTest
    enable_testing() # must be in the root script
    if (NOT TARGET gtest)
        add_subdirectory(extern/googletest EXCLUDE_FROM_ALL)
    endif()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()


if (TWINS_BUILD_DEMO)
endif()

# add library code
add_subdirectory(lib)


# add library demo
if (TWINS_BUILD_DEMO)
    add_subdirectory(demo)
endif()

if (TWINS_BUILD_UT)
    add_subdirectory(tests)
endif()

#
add_custom_target(print_preprocessor_symbols
    COMMAND echo "~~ g++ preprocessor symbols:"
    COMMAND echo | g++ -dM -E -x c++ -
)
