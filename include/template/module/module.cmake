R"( #"
set(OUT_NAME ${%OUT_NAME%})
set(SOURCES ${%SOURCES%})
set(DEFINES ${%DEFINES%})
set(INCLUDE_DIRS ${%INCLUDE_DIRS%})
set(EXPORT_INC ${%EXPORT_INC%})
set(LINK_DIRS ${%LINK_DIRS%})
set(LINK_LIBS ${%LINK_LIBS%})
set(LIB_OUTDIR ${%LIB_OUTDIR%})

add_library(${OUT_NAME} MODULE ${%SOURCES%})
target_include_directories(${OUT_NAME} PUBLIC ${%EXPORT_INC%})
target_include_directories(${OUT_NAME} PRIVATE ${%INCLUDE_DIRS%})
target_compile_definitions(${OUT_NAME} PRIVATE ${%DEFINES%})
target_link_directories(${OUT_NAME} PUBLIC ${%LINK_DIRS%})
target_link_libraries(${OUT_NAME} PUBLIC ${%LINK_LIBS%})
set_target_properties(${OUT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${LIB_OUTDIR}
    OUTPUT_NAME ${OUT_NAME}
    PREFIX "")

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