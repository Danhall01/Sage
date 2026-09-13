cmake_minimum_required(VERSION 3.28)
project(
    SAGE
    LANGUAGES CXX
    VERSION 0.1.0
)

set(SAGE_LIB "sage")

if (TARGET
    ${SAGE_LIB}
)
    return()
endif ()

if (PROJECT_IS_TOP_LEVEL)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/out")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/out")
endif ()

option(
    SAGE_BUILD_SHARED_LIBS
    "Build Sage as a shared library"
    OFF
)
set(BUILD_SHARED_LIBS ${SAGE_BUILD_SHARED_LIBS})

add_library(${SAGE_LIB})
add_library(
    SAGE::sage
    ALIAS
    ${SAGE_LIB}
)
target_compile_features(${SAGE_LIB} PUBLIC cxx_std_23)

add_subdirectory(src)

# Add tests
if (PROJECT_IS_TOP_LEVEL)
    include(CTest)
    if (BUILD_TESTING)
        add_subdirectory(test)
    endif ()
endif ()
