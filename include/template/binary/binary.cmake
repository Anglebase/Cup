R"(#"
set(MAIN_FILE ${%MAIN_FILE%})
set(SOURCES ${%SOURCES%})
set(UNIQUE_SUFFIX ${%UNIQUE_SUFFIX%})
set(TARGET_NAME ${%TARGET_NAME%})
set(UNIQUE_NAME "${TARGET_NAME}_${UNIQUE_SUFFIX}")
set(INCLUDE_DIRS ${%INCLUDE_DIRS%})
set(LIBRARY_DIRS ${%LIBRARY_DIRS%})
set(LIBRARIES ${%LIBRARIES%})
set(DEFINES ${%DEFINES%})
set(COPTIONS ${%COPTIONS%})
set(LOPTIONS ${%LOPTIONS%})
set(MAIN_OUTDIR ${%MAIN_OUTDIR%})
set(STDC ${%STDC%})
set(STDCPP ${%STDCPP%})
set(BIN_MAIN_FILES ${%BIN_MAIN_FILES%})
set(TEST_MAIN_FILES ${%TEST_MAIN_FILES%})
set(BIN_OUTDIR ${%BIN_OUTDIR%})
set(TEST_OUTDIR ${%TEST_OUTDIR%})
set(TEST_INC ${%TEST_INC%})
set(TEST_DEFINES ${%TEST_DEFINES%})
set(DEPENDS ${%DEPENDS%})

add_executable(${UNIQUE_NAME} ${MAIN_FILE} ${SOURCES})
target_include_directories(${UNIQUE_NAME} PRIVATE ${INCLUDE_DIRS})
target_link_directories(${UNIQUE_NAME} PRIVATE ${LIBRARY_DIRS})
target_link_libraries(${UNIQUE_NAME} PRIVATE ${LIBRARIES} ${DEPENDS})
target_compile_definitions(${UNIQUE_NAME} PRIVATE ${DEFINES})
target_compile_options(${UNIQUE_NAME} PRIVATE ${COPTIONS})
target_link_options(${UNIQUE_NAME} PRIVATE ${LOPTIONS})
set_target_properties(${UNIQUE_NAME} PROPERTIES
    OUTPUT_NAME ${TARGET_NAME}
    RUNTIME_OUTPUT_DIRECTORY ${MAIN_OUTDIR}
)
if(${STDC})
    set_target_properties(${UNIQUE_NAME} PROPERTIES
        C_STANDARD ${STDC}
        C_STANDARD_REQUIRED ON
    )
endif()
if(${STDCPP})
    set_target_properties(${UNIQUE_NAME} PROPERTIES
        CXX_STANDARD ${STDCPP}
        CXX_STANDARD_REQUIRED ON
    )
endif()

foreach(BIN_MAIN_FILE ${BIN_MAIN_FILES})
    get_filename_component(FILE_NAME ${BIN_MAIN_FILE} NAME_WLE)
    set(UNIQUE_BIN_NAME "bin_${TARGET_NAME}_${FILE_NAME}_${UNIQUE_SUFFIX}")
    add_executable(${UNIQUE_BIN_NAME} ${BIN_MAIN_FILE} ${SOURCES})
    target_include_directories(${UNIQUE_BIN_NAME} PRIVATE ${INCLUDE_DIRS})
    target_link_directories(${UNIQUE_BIN_NAME} PRIVATE ${LIBRARY_DIRS})
    target_link_libraries(${UNIQUE_BIN_NAME} PRIVATE ${LIBRARIES} ${DEPENDS})
    target_compile_definitions(${UNIQUE_BIN_NAME} PRIVATE ${DEFINES})
    target_compile_options(${UNIQUE_BIN_NAME} PRIVATE ${COPTIONS})
    target_link_options(${UNIQUE_BIN_NAME} PRIVATE ${LOPTIONS})
    set_target_properties(${UNIQUE_BIN_NAME} PROPERTIES
        OUTPUT_NAME ${FILE_NAME}
        RUNTIME_OUTPUT_DIRECTORY ${BIN_OUTDIR}
    )
    if(${STDC})
        set_target_properties(${UNIQUE_BIN_NAME} PROPERTIES
            C_STANDARD ${STDC}
            C_STANDARD_REQUIRED ON
        )
    endif()
    if(${STDCPP})
        set_target_properties(${UNIQUE_BIN_NAME} PROPERTIES
            CXX_STANDARD ${STDCPP}
            CXX_STANDARD_REQUIRED ON
        )
    endif()
    unset(UNIQUE_BIN_NAME)
endforeach(BIN_MAIN_FILE ${BIN_MAIN_FILES})

foreach(TEST_MAIN_FILE ${TEST_MAIN_FILES})
    get_filename_component(FILE_NAME ${TEST_MAIN_FILE} NAME_WLE)
    set(UNIQUE_TEST_NAME "test_${TARGET_NAME}_${FILE_NAME}_${UNIQUE_SUFFIX}")
    add_executable(${UNIQUE_TEST_NAME} ${TEST_MAIN_FILE} ${SOURCES})
    target_include_directories(${UNIQUE_TEST_NAME} PRIVATE ${INCLUDE_DIRS})
    target_link_directories(${UNIQUE_TEST_NAME} PRIVATE ${LIBRARY_DIRS})
    target_link_libraries(${UNIQUE_TEST_NAME} PRIVATE ${LIBRARIES} ${DEPENDS})
    target_compile_definitions(${UNIQUE_TEST_NAME} PRIVATE ${DEFINES})
    target_compile_options(${UNIQUE_TEST_NAME} PRIVATE ${COPTIONS})
    target_link_options(${UNIQUE_TEST_NAME} PRIVATE ${LOPTIONS})
    target_compile_definitions(${UNIQUE_TEST_NAME} PRIVATE ${TEST_DEFINES})
    target_include_directories(${UNIQUE_TEST_NAME} PRIVATE ${TEST_INC})
    set_target_properties(${UNIQUE_TEST_NAME} PROPERTIES
        OUTPUT_NAME ${FILE_NAME}
        RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTDIR}
    )
    if(${STDC})
        set_target_properties(${UNIQUE_TEST_NAME} PROPERTIES
            C_STANDARD ${STDC}
            C_STANDARD_REQUIRED ON
        )
    endif()
    if(${STDCPP})
        set_target_properties(${UNIQUE_TEST_NAME} PROPERTIES
            CXX_STANDARD ${STDCPP}
            CXX_STANDARD_REQUIRED ON
        )
    endif()
    unset(UNIQUE_TEST_NAME)
endforeach(TEST_MAIN_FILE ${TEST_MAIN_FILES})

unset(MAIN_FILE)
unset(SOURCES)
unset(UNIQUE_SUFFIX)
unset(TARGET_NAME)
unset(UNIQUE_NAME)
unset(INCLUDE_DIRS)
unset(LIBRARY_DIRS)
unset(LIBRARIES)
unset(DEFINES)
unset(COPTIONS)
unset(LOPTIONS)
unset(MAIN_OUTDIR)
unset(STDC)
unset(STDCPP)
unset(BIN_MAIN_FILES)
unset(TEST_MAIN_FILES)
unset(BIN_OUTDIR)
unset(TEST_OUTDIR)
unset(TEST_INC)
unset(TEST_DEFINES)
unset(DEPENDS)
#)"