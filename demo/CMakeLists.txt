################################################################################
#   TWins library
#   (c) 2020 Mariusz Midor
#   https://bitbucket.org/marmidr/twins/
#   https://github.com/marmidr/twins/
################################################################################

set(TARGETNAME TWinsDemo)
set(CMAKE_VERBOSE_MAKEFILE ON)

add_executable(${TARGETNAME}
    src/demo.cpp
    src/demo_wnd.cpp
)

target_include_directories(${TARGETNAME}
    PRIVATE
        inc
)

target_link_libraries(${TARGETNAME}
    twins
)

target_compile_options(${TARGETNAME}
    PRIVATE
        -Wno-unused-parameter
        -Wno-missing-field-initializers
)

# FIXME: causes problems when used with VSCode
# if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
#     # https://cmake.org/cmake/help/v3.19/command/target_precompile_headers.html
#     target_precompile_headers(${TARGETNAME}
#         PRIVATE
#             <utility>
#             <type_traits>
#             <memory>
#             <functional>
#     )
# endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_SANITIZERS}")
