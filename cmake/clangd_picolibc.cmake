# Injected into the build by the top-level CMakeLists.txt via CMAKE_PROJECT_INCLUDE:
# this file is processed by CMake immediately after the compiler has been initialized,
# but before ESP-IDF configures any component, so directory compile options set here
# still reach every translation unit of the build.
#
# Purpose: make the C library headers visible to clang-based tools (clangd).
#
# This project uses picolibc (CONFIG_LIBC_PICOLIBC=y). GCC is pointed at picolibc's
# headers by the toolchain's picolibc.specs file, which clang cannot read, so without
# the explicit -isystem below, clangd would not see <stdio.h>, <inttypes.h>, ... and
# would flood the editor with bogus "file not found" errors. The path is derived from
# the active compiler, so it survives toolchain upgrades. GCC silently deduplicates
# the directory (it is already in its search path via the specs file), so the actual
# firmware build is unaffected.
if(DEFINED _PROJECT_CLANGD_PICOLIBC_INCLUDED)
    return()
endif()
set(_PROJECT_CLANGD_PICOLIBC_INCLUDED TRUE)

get_filename_component(_compiler_bin_dir "${CMAKE_C_COMPILER}" DIRECTORY)
get_filename_component(_toolchain_dir "${_compiler_bin_dir}" DIRECTORY)
set(_picolibc_include_dir "${_toolchain_dir}/picolibc/include")

# Applied to all languages, including ASM: clangd infers commands for headers from
# any nearby translation unit, and .S files (e.g. freertos/.../portasm.S) are the
# inferred match for some FreeRTOS headers; without the libc include path those
# headers would fail on <stdlib.h>.
if(EXISTS "${_picolibc_include_dir}")
    add_compile_options("-isystem${_picolibc_include_dir}")
endif()

unset(_compiler_bin_dir)
unset(_toolchain_dir)
unset(_picolibc_include_dir)
