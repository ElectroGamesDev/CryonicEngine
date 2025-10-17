cmake_minimum_required(VERSION 3.20)

project("GameName")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_STANDARD 11)

include(FetchContent)

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
set_target_properties(raylib PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/Source/libraylib-d.a")

add_library(box2d STATIC IMPORTED)
set_target_properties(box2d PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/Source/box2d/libbox2d-d.a")

#add_library(jolt STATIC IMPORTED)
#set_target_properties(jolt PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/Source/Jolt/libJolt-d.a")

file(GLOB SOURCE_FILES
    Source/*.cpp
    Source/*.h
    Source/Scenes/*.cpp
    Source/Scenes/*.h
    Source/Components/*.cpp
    Source/Components/*.h
    Source/box2d/*.h
    Source/Jolt/*.h
)

if ("${PLATFORM}" STREQUAL "WINDOWS")
    # These cause issues when compiling Jolt
    #set(CMAKE_C_COMPILER "clang")
    #set(CMAKE_CXX_COMPILER "clang++")

    set(CMAKE_C_COMPILER "C:/Program Files/LLVM/bin/clang.exe")
    set(CMAKE_CXX_COMPILER "C:/Program Files/LLVM/bin/clang++.exe")

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

target_compile_definitions(${PROJECT_NAME} PRIVATE WINDOWS) # This will break web builds. It doesn't work if I put it within an if () like the one above. It also does't work if I put it in the command. For some reason 2D builds compile fine without this

target_compile_options(${PROJECT_NAME} PRIVATE
    -w
    -Weverything
    -Wno-unused-variable
)

include_directories(${CMAKE_SOURCE_DIR}/Source) # This is for Jolt since it uses #include <> to include local headers. The code below does not fix this issue.
#include_directories(${CMAKE_SOURCE_DIR}/Source/Jolt)
#target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/Source/Jolt)

target_compile_definitions(${PROJECT_NAME} PRIVATE 
    IS3D 
    NAME="${PROJECT_NAME}" 
    ${PLATFORM}
)

# Add Jolt-specific compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    USE_ASSERTS=OFF
    DOUBLE_PRECISION=OFF
    GENERATE_DEBUG_SYMBOLS=ON
    JPH_DEBUG_SYMBOL_FORMAT=""
    OVERRIDE_CXX_FLAGS=ON
    CROSS_PLATFORM_DETERMINISTIC=OFF
    CROSS_COMPILE_ARM=OFF
    BUILD_SHARED_LIBS=OFF
    INTERPROCEDURAL_OPTIMIZATION=ON
    FLOATING_POINT_EXCEPTIONS_ENABLED=ON
    CPP_EXCEPTIONS_ENABLED=OFF
    CPP_RTTI_ENABLED=OFF
    OBJECT_LAYER_BITS=16
    USE_SSE4_1=ON
    USE_SSE4_2=ON
    USE_AVX=ON
    USE_AVX2=ON
    USE_AVX512=OFF
    USE_LZCNT=ON
    USE_TZCNT=ON
    USE_F16C=ON
    USE_FMADD=ON
)

# Include Jolt. The pre-compiled version does not work as it has a "version mismatch" error
FetchContent_Declare(
        JoltPhysics
        GIT_REPOSITORY "https://github.com/jrouwe/JoltPhysics"
        GIT_TAG "v5.2.0"
		SOURCE_SUBDIR "Build"
)
FetchContent_MakeAvailable(JoltPhysics)

target_include_directories(${PROJECT_NAME} PUBLIC ${JoltPhysics_SOURCE_DIR}/..)
target_link_libraries(${PROJECT_NAME} PRIVATE Jolt)

if("${PLATFORM}" STREQUAL "WINDOWS")
    target_link_libraries(${PROJECT_NAME} PRIVATE raylib box2d winmm) # Jolt needs to be re-added once I fix compiling with the prebuild
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