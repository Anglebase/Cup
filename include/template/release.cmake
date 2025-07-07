R"( #"
set(CMAKE_BUILD_TYPE "Release")
add_compile_definitions(-DNDEBUG -D_NDEBUG)
if (MSVC)
    add_compile_options(/O2 /GL /GS- /Gy /GF /Gm-)
else()
    add_compile_options(-O2 -g -ggdb -fno-omit-frame-pointer)
endif()
# )"