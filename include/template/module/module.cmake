R"( #"
set(OUT_NAME ${%OUT_NAME%})
set(SOURCES ${%SOURCES%})
set(INCLUDE_DIR ${%INCLUDE_DIR%})
set(DEPENDS ${%DEPENDS%})
set(DEFINES ${%DEFINES%})
set(COPTIONS ${%COPTIONS%})
set(LOPTIONS ${%LOPTIONS%})
set(LINK_DIRS ${%LINK_DIRS%})
set(LINK_LIBS ${%LINK_LIBS%})
set(TEST_FILES ${%TEST_FILES%})
set(TEST_OUTDIR ${%TEST_OUTDIR%})
set(TEST_DEFINES ${%TEST_DEFINES%})
set(TEST_INCLUDES ${%TEST_INCLUDES%})
set(OUT_DIR ${%OUT_DIR%})
set(UNIQUE_SUFFIX ${%UNIQUE_SUFFIX%})
set(STDC ${%STDC%})
set(STCXX ${%STCXX%})

add_library(${OUT_NAME} MODULE ${SOURCES})
target_include_directories(${OUT_NAME} PRIVATE ${INCLUDE_DIR})
target_compile_definitions(${OUT_NAME} PRIVATE ${DEFINES})
target_compile_options(${OUT_NAME} PRIVATE ${COPTIONS})
target_link_directories(${OUT_NAME} PRIVATE ${LINK_DIRS})
target_link_libraries(${OUT_NAME} PRIVATE ${LINK_LIBS} ${DEPENDS})
target_link_options(${OUT_NAME} PRIVATE ${LOPTIONS})
set_target_properties(${OUT_NAME} PROPERTIES
    PREFIX ""
    OUTPUT_NAME ${OUT_NAME}
    LIBRARY_OUTPUT_DIRECTORY ${OUT_DIR})
if(${STDC})
    set_target_properties(${OUT_NAME} PROPERTIES
        C_STANDARD ${STDC}
        C_STANDARD_REQUIRED ON)
endif(${STDC})
if(${STCXX})
    set_target_properties(${OUT_NAME} PROPERTIES
        CXX_STANDARD ${STCXX}
        CXX_STANDARD_REQUIRED ON)
endif(${STCXX})

foreach(TEST_FILE ${TEST_FILES})
    get_filename_component(FILE_NAME ${TEST_FILE} NAME_WLE)
    set(UNIQUE_NAME "test_${FILE_NAME}_${UNIQUE_SUFFIX}")
    add_executable(${UNIQUE_NAME} ${TEST_FILE} ${SOURCES})
    target_include_directories(${UNIQUE_NAME} PRIVATE ${INCLUDE_DIR})
    target_compile_definitions(${UNIQUE_NAME} PRIVATE ${DEFINES})
    target_compile_options(${UNIQUE_NAME} PRIVATE ${COPTIONS})
    target_link_directories(${UNIQUE_NAME} PRIVATE ${LINK_DIRS})
    target_link_libraries(${UNIQUE_NAME} PRIVATE ${LINK_LIBS} ${DEPENDS})
    target_link_options(${UNIQUE_NAME} PRIVATE ${LOPTIONS})
    target_compile_definitions(${UNIQUE_NAME} PRIVATE ${TEST_DEFINES})
    target_include_directories(${UNIQUE_NAME} PRIVATE ${TEST_INCLUDES})
    set_tests_properties(${UNIQUE_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTDIR}
        OUTPUT_NAME ${FILE_NAME})
    if(${STDC})
        set_target_properties(${UNIQUE_NAME} PROPERTIES
            C_STANDARD ${STDC}
            C_STANDARD_REQUIRED ON)
    endif(${STDC})
    if(${STCXX})
        set_target_properties(${UNIQUE_NAME} PROPERTIES
            CXX_STANDARD ${STCXX}
            CXX_STANDARD_REQUIRED ON)
    endif(${STCXX})
    unset(UNIQUE_NAME)
endforeach(TEST_FILE ${TEST_FILES})

unset(OUT_NAME)
unset(SOURCES)
unset(INCLUDE_DIR)
unset(DEPENDS)
unset(DEFINES)
unset(COPTIONS)
unset(LOPTIONS)
unset(LINK_DIRS)
unset(LINK_LIBS)
unset(TEST_FILES)
unset(TEST_OUTDIR)
unset(TEST_DEFINES)
unset(TEST_INCLUDES)
unset(OUT_DIR)
unset(UNIQUE_SUFFIX)
# )"