include(CMakeParseArguments)
include(TosFunctions)

function(find_gnu_toolchain)
    cmake_parse_arguments(
        TOOLCHAIN # prefix of output variables
        "SET" # list of names of the boolean arguments (only defined ones will be true)
        "TRIPLE" # list of names of mono-valued arguments
        "HINTS" # list of names of multi-valued arguments (output variables are lists)
        ${ARGN} # arguments of the function to parse, here we take the all original ones
    )

    if (NOT TOOLCHAIN_TRIPLE)
        message(FATAL_ERROR "Target triple must be specified for find_toolchain")
    endif()

    set(SEARCH_DIRS ${TOOLCHAIN_HINTS})
    list(APPEND SEARCH_DIRS /opt/x-tools)
    list(APPEND SEARCH_DIRS C:/x-tools)

    SUBDIRLIST(X_TOOLS_SUBS C:/x-tools)
    foreach(path ${X_TOOLS_SUBS})
        list(APPEND SEARCH_DIRS C:/x-tools/${path})
    endforeach()

    SUBDIRLIST(X_TOOLS_SUBS /opt/x-tools)
    foreach(path ${X_TOOLS_SUBS})
        list(APPEND SEARCH_DIRS /opt/x-tools/${path})
    endforeach()

    find_program(TOOLCHAIN_CXX_COMPILER 
        ${TOOLCHAIN_TRIPLE}-g++
        HINTS ${SEARCH_DIRS}
        PATH_SUFFIXES bin ${TOOLCHAIN_TRIPLE}/bin)
    set(TOOLCHAIN_CXX_COMPILER ${TOOLCHAIN_CXX_COMPILER} PARENT_SCOPE)

    find_program(TOOLCHAIN_C_COMPILER 
        ${TOOLCHAIN_TRIPLE}-gcc
        HINTS ${SEARCH_DIRS}
        PATH_SUFFIXES bin ${TOOLCHAIN_TRIPLE}/bin)
    set(TOOLCHAIN_C_COMPILER ${TOOLCHAIN_C_COMPILER} PARENT_SCOPE)

    find_program(TOOLCHAIN_SIZE 
        ${TOOLCHAIN_TRIPLE}-size
        HINTS ${SEARCH_DIRS}
        PATH_SUFFIXES bin ${TOOLCHAIN_TRIPLE}/bin)
    set(TOOLCHAIN_SIZE ${TOOLCHAIN_SIZE} PARENT_SCOPE)

    find_program(TOOLCHAIN_OBJCOPY 
        ${TOOLCHAIN_TRIPLE}-objcopy
        HINTS ${SEARCH_DIRS}
        PATH_SUFFIXES bin ${TOOLCHAIN_TRIPLE}/bin)
    set(TOOLCHAIN_OBJCOPY ${TOOLCHAIN_OBJCOPY} PARENT_SCOPE)

    if (TOOLCHAIN_SET)
        set(CMAKE_CXX_COMPILER ${TOOLCHAIN_CXX_COMPILER} PARENT_SCOPE)
        set(CMAKE_C_COMPILER ${TOOLCHAIN_C_COMPILER} PARENT_SCOPE)
        set(CMAKE_SIZE ${TOOLCHAIN_SIZE} PARENT_SCOPE)
        set(CMAKE_OBJCOPY ${TOOLCHAIN_OBJCOPY} PARENT_SCOPE)
        set(CMAKE_OBJCOPY "${CMAKE_OBJCOPY}" CACHE STRING "OBJCOPY")
    endif()
endfunction()