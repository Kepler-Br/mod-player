cmake_minimum_required(VERSION 3.20)
project(modplayer)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")

add_executable(
        modplayer

        src/main.cpp
        src/mod/Encoding.cpp
        src/mod/Generator.cpp
        src/mod/InfoString.cpp
        src/mod/loaders/DataConvertors.cpp
        src/mod/loaders/ModLoader.cpp
        src/mod/loaders/StreamUtils.cpp
        src/mod/Mod.cpp
        src/mod/Pattern.cpp
        src/mod/Row.cpp
        src/mod/Sample.cpp
        src/mod/writer/RawWriter.cpp
        src/mod/writer/WavWriter.cpp
        src/MemoryBuffer.cpp

        src/exceptions/BadStateException.h
        src/MemoryBuffer.h
        src/MemoryStream.h
        src/mod/Encoding.h
        src/mod/Generator.h
        src/mod/InfoString.h
        src/mod/loaders/DataConvertors.h
        src/mod/loaders/ModLoader.h
        src/mod/loaders/StreamUtils.h
        src/mod/loaders/TrackerLoader.h
        src/mod/Mod.h
        src/mod/Note.h
        src/mod/Pattern.h
        src/mod/Row.h
        src/mod/Sample.h
        src/mod/writer/ModWriter.h
        src/mod/writer/RawWriter.h
        src/mod/writer/WavWriter.h

        ignore-mods/arilou.mod.h
)

target_include_directories(
        modplayer
        PUBLIC
        src
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    set(USE_FLAGS "-s USE_SDL=2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${USE_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${USE_FLAGS}")
    set(CMAKE_EXECUTABLE_SUFFIX .html)
    if (TARGET SDL2::SDL2)
        target_link_libraries(modplayer
                PUBLIC
                SDL2::SDL2
                )
    else ()
        target_link_libraries(modplayer
                PUBLIC
                SDL2
                )
    endif ()
else ()
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
endif ()

set(VENDOR_FOLDER external)
add_subdirectory(${VENDOR_FOLDER}/fmt)

target_link_libraries(modplayer
        PUBLIC
        fmt::fmt
        )
