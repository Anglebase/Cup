R"(#"
set(EXPORT_NAME ${%EXPORT_NAME%})
set(SOURCES ${%SOURCES%})
set(INCLUDE_DIR ${%INCLUDE_DIR%})
set(EXPORT_DIR ${%EXPORT_DIR%})
set(DEFINES ${%DEFINES%})
set(LINK_DIRS ${%LINK_DIRS%})
set(LINK_LIBS ${%LINK_LIBS%})
set(COPTIONS ${%COPTIONS%})
set(LOPTIONS ${%LOPTIONS%})
set(OUT_DIR ${%OUT_DIR%})
set(IS_DEP ${%IS_DEP%})
set(TEST_MAIN_FILES ${%TEST_MAIN_FILES%})
set(EXAMPLE_MAIN_FILES ${%EXAMPLE_MAIN_FILES%})
set(UNIQUE_SUFFIX ${%UNIQUE_SUFFIX%})
set(TEST_OUT_DIR ${%TEST_OUT_DIR%})
set(EXAMPLE_OUT_DIR ${%EXAMPLE_OUT_DIR%})
set(TEST_INC ${%TEST_INC%})
set(TEST_DEFINES ${%TEST_DEFINES%})
set(EXAMPLE_INC ${%EXAMPLE_INC%})
set(EXAMPLE_DEFINES ${%EXAMPLE_DEFINES%})
set(DEPENDS ${%DEPENDS%})
set(DLL_OUT_DIR ${%DLL_OUT_DIR%})

add_library(${EXPORT_NAME} SHARED ${SOURCES})
target_include_directories(${EXPORT_NAME} PRIVATE ${INCLUDE_DIR})
target_include_directories(${EXPORT_NAME} PUBLIC ${EXPORT_DIR})
target_compile_definitions(${EXPORT_NAME} PRIVATE ${DEFINES})
target_compile_options(${EXPORT_NAME} PRIVATE ${COPTIONS})
target_link_options(${EXPORT_NAME} PUBLIC ${LOPTIONS})
target_link_directories(${EXPORT_NAME} PUBLIC ${LINK_DIRS})
target_link_libraries(${EXPORT_NAME} PUBLIC ${LINK_LIBS} ${DEPENDS})
set_target_properties(
    ${EXPORT_NAME} PROPERTIES
    OUTPUT_NAME ${EXPORT_NAME}
    ARCHIVE_OUTPUT_DIRECTORY ${OUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${DLL_OUT_DIR}
    PREFIX ""
)

if(NOT ${IS_DEP})
    foreach(TEST_MAIN_FILE ${TEST_MAIN_FILES})
        get_filename_component(FILE_NAME ${TEST_MAIN_FILE} NAME_WLE)
        set(UNIQUE_NAME "test_${FILE_NAME}_${UNIQUE_SUFFIX}")
        add_executable(${UNIQUE_NAME} ${TEST_MAIN_FILE})
        target_link_libraries(${UNIQUE_NAME} ${EXPORT_NAME})
        target_compile_definitions(${UNIQUE_NAME} PRIVATE ${TEST_DEFINES})
        target_include_directories(${UNIQUE_NAME} PRIVATE ${TEST_INC})
        set_target_properties(
            ${UNIQUE_NAME} PROPERTIES
            OUTPUT_NAME ${FILE_NAME}
            RUNTIME_OUTPUT_DIRECTORY ${TEST_OUT_DIR}
            PREFIX ""
        )
        unset(UNIQUE_NAME)
    endforeach(TEST_MAIN_FILE ${TEST_MAIN_FILES})
    
    foreach(EXAMPLE_MAIN_FILE ${EXAMPLE_MAIN_FILES})
        get_filename_component(FILE_NAME ${EXAMPLE_MAIN_FILE} NAME_WLE)
        set(UNIQUE_NAME "example_${FILE_NAME}_${UNIQUE_SUFFIX}")
        add_executable(${UNIQUE_NAME} ${EXAMPLE_MAIN_FILE})
        target_link_libraries(${UNIQUE_NAME} ${EXPORT_NAME})
        target_compile_definitions(${UNIQUE_NAME} PRIVATE ${EXAMPLE_DEFINES})
        target_include_directories(${UNIQUE_NAME} PRIVATE ${EXAMPLE_INC})
        set_target_properties(
            ${UNIQUE_NAME} PROPERTIES
            OUTPUT_NAME ${FILE_NAME}
            RUNTIME_OUTPUT_DIRECTORY ${EXAMPLE_OUT_DIR}
            PREFIX ""
        )
        unset(UNIQUE_NAME)
    endforeach(EXAMPLE_MAIN_FILE ${EXAMPLE_MAIN_FILES})
endif(NOT ${IS_DEP})

unset(EXPORT_NAME)
unset(SOURCES)
unset(INCLUDE_DIR)
unset(EXPORT_DIR)
unset(DEFINES)
unset(LINK_DIRS)
unset(LINK_LIBS)
unset(COPTIONS)
unset(LOPTIONS)
unset(OUT_DIR)
unset(IS_DEP)
unset(TEST_MAIN_FILES)
unset(EXAMPLE_MAIN_FILES)
unset(UNIQUE_SUFFIX)
unset(TEST_OUT_DIR)
unset(EXAMPLE_OUT_DIR)
unset(TEST_INC)
unset(TEST_DEFINES)
unset(EXAMPLE_INC)
unset(EXAMPLE_DEFINES)
unset(DEPENDS)
unset(DLL_OUT_DIR)
#)"