# - Find Lemon executable and provides macros to generate custom build rules
# The module defines the following variables:
#
#  LEMON_EXECUTABLE - path to the lemon program
#  LEMON_VERSION - version of lemon
#  LEMON_FOUND - true if the program was found
#
# If lemon is found, the module defines the macros:
#
#  LEMON_TARGET(<Name> <LemonInp> <CodeOutput>
#              [COMPILE_FLAGS <string>])
#
# which will create  a custom rule to generate a state machine. <LemonInp> is
# the path to a Lemon file. <CodeOutput> is the name of the source file
# generated by lemon. If  COMPILE_FLAGS option is specified, the next
# parameter is  added in the lemon command line.
#
# The macro defines a set of variables:
#  LEMON_${Name}_DEFINED       - true is the macro ran successfully
#  LEMON_${Name}_INPUT         - The input source file, an alias for <LemonInp>
#  LEMON_${Name}_OUTPUT_SOURCE - The source file generated by lemon
#  LEMON_${Name}_OUTPUT_HEADER - The header file generated by lemon
#  LEMON_${Name}_COMPILE_FLAGS - Options used in the ragel command line
#
#  ====================================================================
#  Example:
#
#   find_package(LEMON) # or e.g.: find_package(LEMON 6.6 REQUIRED)
#   LEMON_TARGET(MyMachine machine.y ${CMAKE_CURRENT_BINARY_DIR}/machine.c)
#   add_executable(Foo main.cc ${LEMON_MyMachine_OUTPUTS})
#  ====================================================================

# 2014-02-09, Georg Sauthoff <mail@georg.so>
#
# I don't think that these few lines are even copyrightable material,
# but I am fine with using the BSD/MIT/GPL license on it ...
#
# I've used following references:
# http://www.cmake.org/cmake/help/v2.8.12/cmake.html
# /usr/share/cmake/Modules/FindFLEX.cmake
# /usr/share/cmake/Modules/FindBISON.cmake
#
# 2016-06-10, Ashley Towns <mail@ashleytowns.id.au>
#
# Used FindRAGEL.cmake to make this FindLEMON.cmake.

cmake_minimum_required(VERSION 2.8)

find_program(LEMON_EXECUTABLE NAMES lemon DOC "path to the lemon executable")

mark_as_advanced(LEMON_EXECUTABLE)
mark_as_advanced(LEMON_SOURCE_FILE)
mark_as_advanced(LEMON_TEMPLATE)

if(LEMON_SOURCE_FILE)
    if(NOT EXISTS "${CMAKE_BINARY_DIR}/lemon")
        message(STATUS "Compiling ${LEMON_SOURCE_FILE} to ${CMAKE_BINARY_DIR}/lemon")

        execute_process(COMMAND cc -o ${CMAKE_BINARY_DIR}/lemon ${LEMON_SOURCE_FILE})

        set(LEMON_EXECUTABLE ${CMAKE_BINARY_DIR}/lemon)
    endif()
endif()

if(LEMON_EXECUTABLE)
    execute_process(COMMAND ${LEMON_EXECUTABLE} -x
            OUTPUT_VARIABLE LEMON_version_output
            ERROR_VARIABLE  LEMON_version_error
            RESULT_VARIABLE LEMON_version_result
            OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(${LEMON_version_result} EQUAL 0)
        string(REGEX REPLACE "^Lemon version ([^ ]+) .*$"
                "\\1"
                _VERSION "${LEMON_version_output}")
    else()
        message(SEND_ERROR
                "Command \"${LEMON_EXECUTABLE} -x\" failed with output:
    ${LEMON_version_error}")
    endif()

    #============================================================
    # LEMON_TARGET (public macro)
    #============================================================

    macro(LEMON_TARGET Name Input OutputDir)
        set(LEMON_TARGET_usage
                "LEMON_TARGET(<Name> <Input> <OutputDir> [COMPILE_FLAGS <string>]")
        if(${ARGC} GREATER 3)
            if(${ARGC} EQUAL 5)
                if("${ARGV3}" STREQUAL "COMPILE_FLAGS")
                    set(LEMON_EXECUTABLE_opts  "${ARGV4}")
                    separate_arguments(LEMON_EXECUTABLE_opts)
                else()
                    message(SEND_ERROR ${LEMON_TARGET_usage})
                endif()
            else()
                message(SEND_ERROR ${LEMON_TARGET_usage})
            endif()
        endif()

        get_filename_component(InputBasename ${Input} NAME_WE)

        add_custom_command(
                OUTPUT "${OutputDir}/${InputBasename}.c"
                "${OutputDir}/${InputBasename}.h"
                "${OutputDir}/${InputBasename}.out"
                COMMAND ${LEMON_EXECUTABLE}
                ARGS    ${LEMON_EXECUTABLE_opts} -p -T${LEMON_TEMPLATE} -o${OutputDir}/ ${Input}
                DEPENDS ${Input}
                COMMENT
                "[LEMON][${Name}] Compiling parser with Lemon ${LEMON_VERSION}"
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

        set(LEMON_${Name}_DEFINED       TRUE)
        set(LEMON_${Name}_SOURCE_OUTPUT "${OutputDir}/${InputBasename}.c")
        set(LEMON_${Name}_HEADER_OUTPUT "${OutputDir}/${InputBasename}.h")
        set(LEMON_${Name}_OUT_OUTPUT    "${OutputDir}/${InputBasename}.out")
        set(LEMON_${Name}_INPUT         ${Input})
        set(LEMON_${Name}_COMPILE_FLAGS ${LEMON_EXECUTABLE_opts})
        set(LEMON_${Name}_OUTPUTS       "${OutputDir}/${InputBasename}.c")
    endmacro()

endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LEMON REQUIRED_VARS  LEMON_EXECUTABLE
        VERSION_VAR    LEMON_VERSION)
