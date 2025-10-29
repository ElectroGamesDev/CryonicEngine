cmake_minimum_required(VERSION 3.20)

project("GameName")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_STANDARD 11)

#set(CMAKE_BUILD_TYPE Debug)

if ("${PLATFORM}" STREQUAL "WEB")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s ALLOW_MEMORY_GROWTH=1 -s GL_ENABLE_GET_PROC_ADDRESS=1")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -g0")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g0")
else()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -march=native")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
endif()

add_library(raylib STATIC IMPORTED)
set_target_properties(raylib PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/Source/ThirdParty/raylib/lib/libraylib-d.a")

add_library(box2d STATIC IMPORTED)
set_target_properties(box2d PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/Source/ThirdParty/box2d/lib/libbox2d-d.a")

file(GLOB_RECURSE SOURCE_FILES
    CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/Source/*.cpp
	${CMAKE_SOURCE_DIR}/Source/*.c
    ${CMAKE_SOURCE_DIR}/Source/*.h
)

if ("${PLATFORM}" STREQUAL "WINDOWS")
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_CXX_COMPILER "clang++")

    if (DEFINED CCACHE_PATH)
        set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PATH}")
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PATH}")
    endif()

    # Generate the resource file from the template 
    configure_file(resource.rc.in resource.rc @ONLY)

    add_executable(${PROJECT_NAME} ${SOURCE_FILES} resource.rc)

    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-fuse-ld=lld")
else()
    add_executable(${PROJECT_NAME} ${SOURCE_FILES})
endif()

# Add all subdirectories Source as include directories
file(GLOB_RECURSE COMPONENT_DIRS LIST_DIRECTORIES true ${CMAKE_SOURCE_DIR}/*)

foreach(dir ${COMPONENT_DIRS})
    if (IS_DIRECTORY ${dir})
        target_include_directories(${PROJECT_NAME} PRIVATE ${dir})
    endif()
endforeach()

# Base include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/Source
    ${CMAKE_SOURCE_DIR}/Source/Components
    ${CMAKE_SOURCE_DIR}/ThirdParty
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -w
    -Weverything
    -Wno-unused-variable
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