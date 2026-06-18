cmake_minimum_required(VERSION 3.22)
project(drum-machine LANGUAGES CXX)

# ── Global settings ───────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ── Dependencies ──────────────────────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.13
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

find_package(OpenGL REQUIRED)
find_package(glfw3  3.3 REQUIRED)
find_package(OpenMP REQUIRED COMPONENTS CXX)
find_package(glm    REQUIRED)

# ── Shared compile options ────────────────────────────────────────────────────
# Applied to all targets via a helper function below
function(apply_common_options target)
    target_compile_definitions(${target} PRIVATE
        $<$<CONFIG:Debug>:DEBUG=1>
    )

    target_compile_definitions(${target} PRIVATE
        $<$<CONFIG:Debug>:DEBUG=1>
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
    )
    target_compile_options(${target} PRIVATE
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:
            -Wall -Wextra -Wpedantic
            -fno-omit-frame-pointer
            $<$<CONFIG:Debug>:-g -O0 -gdwarf-4>
            $<$<CONFIG:Release>:-O3>
        >
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /Zi>
    )
    if(MSVC)
        set_target_properties(${target} PROPERTIES LINK_FLAGS "/DEBUG")
    endif()
endfunction()

# ── Main executable ───────────────────────────────────────────────────────────
add_executable(drum-machine
    src/main.cc
    src/frontend/drumRenderer.cc
    src/backend/physics/head/CircularMembrane.cc
    src/backend/physics/shell/CylindricalResonator.cc
    src/backend/physics/PhysicsThread.cc
    src/backend/audio/engine/audioEngine.cc
    src/backend/audio/dsp/audioDSP.cc
    src/3rdparty/glad.c
)

target_include_directories(drum-machine PRIVATE include dependencies)


target_link_libraries(drum-machine PRIVATE
    OpenGL::GL
    glfw
    OpenMP::OpenMP_CXX
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
)

apply_common_options(drum-machine)

# ── Tests ─────────────────────────────────────────────────────────────────────
enable_testing()

add_executable(test_rectangular_membrane
    test/rectangularMembraneUnitTest.cc
    src/backend/physics/head/RectangularMembrane.cc
    src/backend/audio/engine/audioEngine.cc
)

target_include_directories(test_rectangular_membrane PRIVATE include dependencies)

target_link_libraries(test_rectangular_membrane PRIVATE
    OpenGL::GL
    glfw
    OpenMP::OpenMP_CXX
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_audio_devices
)

apply_common_options(test_rectangular_membrane)

add_test(
    NAME    RectangularMembraneTest
    COMMAND test_rectangular_membrane ${CMAKE_SOURCE_DIR}/config/drum_config.json
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)