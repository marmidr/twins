################################################################################
#   TWins library
#   (c) 2020 Mariusz Midor
#   https://bitbucket.org/marmidr/twins/
#   https://github.com/marmidr/twins/
################################################################################

option(TWINS_USE_FASTFILL "Use ANSI repeat code for faster line fill" ON)
option(TWINS_PAL_FULLIMPL "Disable in case of incompatible platform" ON)
option(TWINS_USE_PRECISE_TIMESTAMP "Logs timestamp with fraction of seconds" ON)
option(TWINS_USE_LIGHTWEIGHT_CMD "Use void(*)() for twins::cli::Cmd instead of std::function<>" ON)

set(TARGETNAME twins)

add_library(${TARGETNAME} STATIC
    src/twins.cpp
    src/twins_colors.cpp
    src/twins_widget.cpp
    src/twins_widget_draw.cpp
    src/twins_utf8str.cpp
    src/twins_string.cpp
    src/twins_input_posix.cpp
    src/twins_ansi_esc_decoder.cpp
    src/twins_utils.cpp
    src/twins_window_mngr.cpp
    src/twins_cli.cpp
)

target_include_directories(${TARGETNAME}
    PUBLIC
        inc
)

# FIXME: causes problems when used with VSCode
# if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
#     target_precompile_headers(${TARGETNAME}
#         PRIVATE
#             <utility>
#             <type_traits>
#             <memory>
#     )
# endif()

if (TWINS_USE_FASTFILL)
    target_compile_definitions(${TARGETNAME} PRIVATE TWINS_FAST_FILL=1)
else()
    target_compile_definitions(${TARGETNAME} PRIVATE TWINS_FAST_FILL=0)
endif()

if (TWINS_PAL_FULLIMPL)
    target_compile_definitions(${TARGETNAME} PUBLIC TWINS_PAL_FULLIMPL=1)
else()
    target_compile_definitions(${TARGETNAME} PUBLIC TWINS_PAL_FULLIMPL=0)
endif()

if (TWINS_USE_PRECISE_TIMESTAMP)
    target_compile_definitions(${TARGETNAME} PUBLIC TWINS_PRECISE_TIMESTAMP=1)
else()
    target_compile_definitions(${TARGETNAME} PUBLIC TWINS_PRECISE_TIMESTAMP=0)
endif()

if (TWINS_USE_LIGHTWEIGHT_CMD)
    target_compile_definitions(${TARGETNAME} PUBLIC TWINS_CLI_LIGHTWEIGHT_CMD=1)
else()
    target_compile_definitions(${TARGETNAME} PUBLIC TWINS_CLI_LIGHTWEIGHT_CMD=0)
endif()

if (NOT "${TWINS_THEME_DIR}" STREQUAL "")
    target_include_directories(${TARGETNAME} PUBLIC ${TWINS_THEME_DIR})
    target_compile_definitions(${TARGETNAME} PUBLIC TWINS_THEMES=1)
    message(STATUS "TWins themes enabled")
endif()

if (TWINS_BUILD_UT)
    option(TWINS_USE_SANITIZERS "Use GCC sanitizers for Unit Tests" ON)
    set(GCC_SANITIZERS "")

    # https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html
    # set(GCC_SANITIZERS "-fno-omit-frame-pointer -fsanitize=leak -fsanitize=address -fsanitize=undefined")
    # set(GCC_SANITIZERS "-fsanitize=leak,undefined")

    if (TWINS_USE_SANITIZERS)
        if (DEFINED ENV{WSL_DISTRO_NAME})
            message(STATUS "Platform: WSL $ENV{WSL_DISTRO_NAME}")
            message(STATUS "Sanitizers not supported on WSL")
        elseif (MINGW OR MSYS)
            set(GCC_SANITIZERS "")
            message(STATUS "Platform: MINGW / MSYS")
            message(STATUS "Sanitizers not supported on MINGW / MSYS")
        elseif (UNIX)
            # target_compile_options(${TARGETNAME} PRIVATE -static-libasan)
            # target_compile_options(${TARGETNAME} PRIVATE -static-libubsan)
            set(GCC_SANITIZERS "-fsanitize=leak,address,undefined")
            message(STATUS "Platform: UNIX")
        else()
            message(STATUS "Platform: Unknown")
        endif()
    endif()

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_SANITIZERS}")

    # enable coverage only for source files
    target_compile_options(${TARGETNAME} PRIVATE --coverage)
endif()
