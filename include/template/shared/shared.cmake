R"( #"
set(OUT_NAME ${%OUT_NAME%})
set(SOURCES ${%SOURCES%})
set(DEFINES ${%DEFINES%})
set(INCLUDE_DIRS ${%INCLUDE_DIRS%})
set(EXPORT_INC ${%EXPORT_INC%})
set(LINK_DIRS ${%LINK_DIRS%})
set(LINK_LIBS ${%LINK_LIBS%})
set(AS_DEP ${%AS_DEP%})
set(EXAMPLE_MAINS ${%EXAMPLE_MAINS%})
set(EXAMPLE_OUTDIR ${%EXAMPLE_OUTDIR%})
set(LIB_OUTDIR ${%LIB_OUTDIR%})
set(EXAMPLE_INC ${%EXAMPLE_INC%})
set(EXAMPLE_DEF ${%EXAMPLE_DEF%})

add_library(${OUT_NAME} SHARED ${%SOURCES%})
target_include_directories(${OUT_NAME} PUBLIC ${%EXPORT_INC%})
target_include_directories(${OUT_NAME} PRIVATE ${%INCLUDE_DIRS%})
target_compile_definitions(${OUT_NAME} PRIVATE ${%DEFINES%})
target_link_directories(${OUT_NAME} PUBLIC ${%LINK_DIRS%})
target_link_libraries(${OUT_NAME} PUBLIC ${%LINK_LIBS%})
set_target_properties(${OUT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${LIB_OUTDIR}
    RUNTIME_OUTPUT_DIRECTORY ${EXAMPLE_OUTDIR}
    OUTPUT_NAME ${OUT_NAME}
    PREFIX "")

if (NOT AS_DEP)
    foreach(EXAMPLE_MAIN ${%EXAMPLE_MAINS%})
        get_filename_component(FILE_NAME ${EXAMPLE_MAIN} NAME_WLE)
        add_executable(${FILE_NAME} ${EXAMPLE_MAIN})
        target_link_libraries(${FILE_NAME} PRIVATE ${OUT_NAME})
        target_include_directories(${FILE_NAME} PRIVATE ${%EXAMPLE_INC%})
        target_compile_definitions(${FILE_NAME} PRIVATE ${%EXAMPLE_DEF%})
        set_target_properties(${FILE_NAME} PROPERTIES 
            OUTPUT_NAME ${FILE_NAME}
            RUNTIME_OUTPUT_DIRECTORY ${EXAMPLE_OUTDIR})
    endforeach(EXAMPLE_MAIN ${EXAMPLE_MAINS})
endif()

unset(OUT_NAME)
unset(SOURCES)
unset(DEFINES)
unset(INCLUDE_DIRS)
unset(EXPORT_INC)
unset(FEATURES)
unset(LINK_DIRS)
unset(AS_DEP)
unset(EXAMPLE_MAINS)
unset(EXAMPLE_OUTDIR)
unset(LIB_OUTDIR)
# )"