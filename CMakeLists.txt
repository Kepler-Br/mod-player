cmake_minimum_required(VERSION 3.20)
project(modplayer)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")

add_executable(
        modplayer
        src/mod/Pattern.cpp
        src/mod/Reader.cpp
        src/mod/Sample.cpp
        src/mod/Serializer.cpp
        src/mod/InfoString.cpp
        src/mod/Mod.cpp
        src/mod/Row.cpp
        src/mod/Encoding.cpp
        src/main.cpp
        src/mod/Generator.cpp

        src/mod/Note.h
        src/mod/Pattern.h
        src/mod/Reader.h
        src/mod/Row.h
        src/mod/Sample.h
        src/mod/Serializer.h
        src/mod/InfoString.h
        src/mod/Mod.h
        src/mod/Encoding.h
        src/MemoryBuffer.h
        src/MemoryStream.h
        src/mod/Generator.h
)

find_package(SDL2 REQUIRED)
if (TARGET SDL2::SDL2)
    target_link_libraries(modplayer
            PUBLIC
            SDL2::SDL2-static
            )
else ()
    target_link_libraries(modplayer
            PUBLIC
            SDL2-static
            )
endif ()

set(VENDOR_FOLDER external)
add_subdirectory(${VENDOR_FOLDER}/fmt)

target_link_libraries(modplayer
        PUBLIC
        fmt::fmt
        )
