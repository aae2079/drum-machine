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

# Set optimization levels for each build type
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fno-omit-frame-pointer -gdwarf-4")

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)


# Add source files
set(SOURCES
    src/main.cc
    src/glad.c
    #src/CircularMembrane.cpp
    src/RectangularMembrane.cc
    src/audioEngine.cpp
)

set(HEADERS
    #include/CircularMembrane.h
    include/RectangularMembrane.h
    include/Surface.h
    include/audioDefs.h
    include/strikeDefs.h
    dependencies/glad/glad.h
    dependencies/KHR/khrplatform.h
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
    target_compile_options(drum-machine PRIVATE -fno-omit-frame-pointer)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_definitions(drum-machine PRIVATE DEBUG=1)
    endif()
endif()


target_include_directories(drum-machine 
        PRIVATE 
        include
        dependencies
)

# Link libraries
target_link_libraries(drum-machine
    PRIVATE
        OpenGL::GL
        glfw
)



# Optional: Add compiler flags for additional warnings
if(MSVC)
    target_compile_options(drum-machine PRIVATE /W4)
else()
    target_compile_options(drum-machine PRIVATE -Wall -Wextra -Wpedantic)
endif()
