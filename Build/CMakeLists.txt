cmake_minimum_required(VERSION 3.20)

project("GameName")

set(CMAKE_C_STANDARD 11)

#set(CMAKE_BUILD_TYPE Debug)

if ("${PLATFORM}" STREQUAL "WEB")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s ALLOW_MEMORY_GROWTH=1 -s GL_ENABLE_GET_PROC_ADDRESS=1")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -g0")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g0")
endif()

add_library(raylib STATIC IMPORTED)
set_target_properties(raylib PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/Source/libraylib.a")

add_library(box2d STATIC IMPORTED)
set_target_properties(box2d PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/Source/box2d/libbox2d.a")

file(GLOB SOURCE_FILES
    Source/*.cpp
    Source/*.h
    Source/Scenes/*.cpp
    Source/Scenes/*.h
    Source/Components/*.cpp
    Source/Components/*.h
    Source/box2d/*.h
)

if ("${PLATFORM}" STREQUAL "WINDOWS")
    set(CMAKE_C_COMPILER "C:/MinGW/bin/gcc.exe")
    set(CMAKE_CXX_COMPILER "C:/MinGW/bin/g++.exe")

    if (DEFINED CCACHE_PATH)
        set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PATH}")
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PATH}")
    endif()

    # Generate the resource file from the template 
    configure_file(resource.rc.in resource.rc @ONLY)

    add_executable(${PROJECT_NAME} ${SOURCE_FILES} resource.rc)
else()
    add_executable(${PROJECT_NAME} ${SOURCE_FILES})
endif()

target_compile_options(${PROJECT_NAME} PRIVATE
    #-Werror=implicit-function-declaration
    -w
)

target_compile_definitions(${PROJECT_NAME} PRIVATE 
    IS2D 
    NAME="${PROJECT_NAME}" 
    ${PLATFORM}
)

if("${PLATFORM}" STREQUAL "WINDOWS")
    target_link_libraries(${PROJECT_NAME} PRIVATE raylib box2d winmm)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE raylib box2d)
endif()

if ("${PLATFORM}" STREQUAL "WEB")
    target_link_options(${PROJECT_NAME} PUBLIC
        --preload-file Resources
        --shell-file Template.html
    )
endif()

#target_link_options(${PROJECT_NAME} PRIVATE "-mwindows")