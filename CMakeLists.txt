cmake_minimum_required(VERSION 3.10)
project(drum-machine)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# If the user hasn't specified a build type, default to Debug to make
# builds gdb-friendly. Users can override by passing -DCMAKE_BUILD_TYPE=Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
endif()
if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_subdirectory(external/dave)
    target_link_libraries(drum-machine PRIVATE dave)
    target_compile_definitions(drum-machine PRIVATE USE_DAVE_DEBUGGER=1)
endif()
# Set optimization levels for each build type
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(OpenMP REQUIRED)
find_package(glm REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)


# Add source files
set(SOURCES
    src/main.cc
    src/frontend/drumRenderer.cc
    src/backend/CircularMembrane.cc
    src/backend/RectangularMembrane.cc
    src/backend/audioEngine.cc
    src/3rdparty/glad.c
)

set(HEADERS
    include/CircularMembrane.hpp
    include/RectangularMembrane.hpp
    include/Surface.hpp
    include/audioDefs.hpp
    include/strikeDefs.hpp
    include/drumRenderer.hpp
    dependencies/glad/glad.h
    dependencies/KHR/khrplatform.h
    #/usr/include/portaudio.h
)

# Create executable
add_executable(drum-machine ${SOURCES} ${HEADERS})

if(MSVC)
    # MSVC debugging: Program Database and debug info
    target_compile_options(drum-machine PRIVATE /Zi)
    # Ensure linker produces debugging information
    set_target_properties(drum-machine PROPERTIES LINK_FLAGS "/DEBUG")
# GCC/Clang flags
else()
    target_compile_options(drum-machine PRIVATE -fno-omit-frame-pointer -gdwarf-4)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_definitions(drum-machine PRIVATE DEBUG=1)
    endif()
endif()


target_include_directories(drum-machine 
        PRIVATE 
        include
        dependencies
        ${PORTAUDIO_INCLUDE_DIRS}
)

target_link_directories(drum-machine PRIVATE ${PORTAUDIO_LIBRARY_DIRS})

# Link libraries
target_link_libraries(drum-machine
    PRIVATE
        OpenGL::GL
        glfw
        OpenMP::OpenMP_CXX
        ${PORTAUDIO_LIBRARIES}
)



# Optional: Add compiler flags for additional warnings
if(MSVC)
    target_compile_options(drum-machine PRIVATE /W4)
else()
    target_compile_options(drum-machine PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Testing
enable_testing()

set(TEST_SOURCES
    test/rectangularMembraneUnitTest.cc
    src/backend/RectangularMembrane.cc
    src/backend/audioEngine.cc
)

add_executable(test_rectangular_membrane ${TEST_SOURCES})

target_include_directories(test_rectangular_membrane
    PRIVATE
        include
        dependencies
        ${PORTAUDIO_INCLUDE_DIRS}
)

target_link_directories(test_rectangular_membrane PRIVATE ${PORTAUDIO_LIBRARY_DIRS})

target_link_libraries(test_rectangular_membrane
    PRIVATE
        OpenGL::GL
        glfw
        OpenMP::OpenMP_CXX
        ${PORTAUDIO_LIBRARIES}
)

target_compile_options(test_rectangular_membrane PRIVATE
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

add_test(
    NAME RectangularMembraneTest
    COMMAND test_rectangular_membrane 2
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
