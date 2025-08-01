R"(#"

set(TEST_INCLUDE_DIRS ${%TEST_INCLUDE_DIRS%})
set(TEST_LIB_DIRS ${%TEST_LIB_DIRS%})
set(TEST_LIBS ${%TEST_LIBS%})
set(TEST_DEFINES ${%TEST_DEFINES%})
set(TEST_COPTIONS ${%TEST_COPTIONS%})
set(TEST_LINKOPTIONS ${%TEST_LINKOPTIONS%})
set(TEST_SOURCES ${%TEST_SOURCES%})
set(TEST_COMPILER_FEAT ${%TEST_COMPILER_FEAT%})

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(TEST_MODE_INCLUDE_DIRS ${%TEST_DEBUG_INCLUDE_DIRS%})
    set(TEST_MODE_LIB_DIRS ${%TEST_DEBUG_LIB_DIRS%})
    set(TEST_MODE_LIBS ${%TEST_DEBUG_LIBS%})
    set(TEST_MODE_DEFINES ${%TEST_DEBUG_DEFINES%})
    set(TEST_MODE_COPTIONS ${%TEST_DEBUG_COPTIONS%})
    set(TEST_MODE_LINKOPTIONS ${%TEST_DEBUG_LINKOPTIONS%})
    set(TEST_MODE_SOURCES ${%TEST_DEBUG_SOURCES%})
    set(TEST_MODE_COMPILER_FEAT ${%TEST_DEBUG_COMPILER_FEAT%})
else()
    set(TEST_MODE_INCLUDE_DIRS ${%TEST_RELEASE_INCLUDE_DIRS%})
    set(TEST_MODE_LIB_DIRS ${%TEST_RELEASE_LIB_DIRS%})
    set(TEST_MODE_LIBS ${%TEST_RELEASE_LIBS%})
    set(TEST_MODE_DEFINES ${%TEST_RELEASE_DEFINES%})
    set(TEST_MODE_COPTIONS ${%TEST_RELEASE_COPTIONS%})
    set(TEST_MODE_LINKOPTIONS ${%TEST_RELEASE_LINKOPTIONS%})
    set(TEST_MODE_SOURCES ${%TEST_RELEASE_SOURCES%})
    set(TEST_MODE_COMPILER_FEAT ${%TEST_RELEASE_COMPILER_FEAT%})
endif()

#)"