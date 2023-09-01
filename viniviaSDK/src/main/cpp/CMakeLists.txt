# -------------------------------------------------------------------------------------------------
# PROJECT: ViniviaAndroidSDK
# -------------------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 17)

# -------------------------------------------------------------------------------------------------
# Add cmake module paths
# -------------------------------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake/")

include(ProjectEnvironment)

# -------------------------------------------------------------------------------------------------
# Project settings
# -------------------------------------------------------------------------------------------------
project(ViniviaSDK
        VERSION 1.0.0
        LANGUAGES CXX C
        )

# -------------------------------------------------------------------------------------------------
# Headers & Sources
# -------------------------------------------------------------------------------------------------
set(SOURCES
        ${CMAKE_SOURCE_DIR}/Bridge/jni_hooks.cpp
        ${CMAKE_SOURCE_DIR}/Bridge/ViniviaSDKBridge.cpp
        ${CMAKE_SOURCE_DIR}/Core/Entrance.cpp
        ${CMAKE_SOURCE_DIR}/Graphics/Graphics.cpp
        ${CMAKE_SOURCE_DIR}/Graphics/Renderer.cpp
        ${CMAKE_SOURCE_DIR}/Graphics/RendererES2.cpp
        "${CMAKE_SOURCE_DIR}/Utils/OpenGLChecker.cpp"
        ${CMAKE_SOURCE_DIR}/Utils/Matrix.cpp
        ${CMAKE_SOURCE_DIR}/Filters/BaseFilter.cpp
        )

set(HEADERS
        ${CMAKE_SOURCE_DIR}/Core/Entrance_p.h
        ${CMAKE_SOURCE_DIR}/Core/Entrance.h
        ${CMAKE_SOURCE_DIR}/Graphics/Graphics.h
        ${CMAKE_SOURCE_DIR}/Graphics/Renderer.h
        ${CMAKE_SOURCE_DIR}/Graphics/RendererES2.h
        ${CMAKE_SOURCE_DIR}/Log/Log.h
        "${CMAKE_SOURCE_DIR}/Utils/OpenGLChecker.h"
        ${CMAKE_SOURCE_DIR}/Utils/Matrix.h
        ${CMAKE_SOURCE_DIR}/Filters/BaseFilter.h
        ${CMAKE_SOURCE_DIR}/Filters/NoFilter.h
        ${CMAKE_SOURCE_DIR}/Filters/CubeFilter.h
        ${CMAKE_SOURCE_DIR}/Filters/DepthMapFilter.h
        ${CMAKE_SOURCE_DIR}/Filters/GrayscaleFilter.h
        ${CMAKE_SOURCE_DIR}/Filters/NegativeFilter.h
        ${CMAKE_SOURCE_DIR}/Filters/PositiveMaskFilter.h
        ${CMAKE_SOURCE_DIR}/Filters/NegativeMaskFilter.h
        ${CMAKE_SOURCE_DIR}/Filters/CubeOverMaskFilter.h
        ${CMAKE_SOURCE_DIR}/Shapes/Cube.h
        )

# -------------------------------------------------------------------------------------------------
# Add platform based headers and sources
# -------------------------------------------------------------------------------------------------
add_library(${PROJECT_NAME} SHARED
        ${SOURCES}
        ${HEADERS}
        )

# -------------------------------------------------------------------------------------------------
# Compiler options
# -------------------------------------------------------------------------------------------------
target_compile_options(
        ${PROJECT_NAME} PUBLIC -fno-rtti -fno-exceptions -Wno-format -Wno-unused-variable
)

# -------------------------------------------------------------------------------------------------
# Find packages
# -------------------------------------------------------------------------------------------------
find_library(log-lib log)
find_library(android-lib android)
find_library(opengl-lib GLESv2)
find_library(egl-lib EGL)
find_library(jnigraphics-lib jnigraphics)

# -------------------------------------------------------------------------------------------------
# Library dependencies
# -------------------------------------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME} PUBLIC
        ${log-lib}
        ${android-lib}
        ${opengl-lib}
        ${egl-lib}
        ${jnigraphics-lib}
        )

# -------------------------------------------------------------------------------------------------
# Include directories
# -------------------------------------------------------------------------------------------------
target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_SOURCE_DIR}
        )

# -------------------------------------------------------------------------------------------------
# Project Summary
# -------------------------------------------------------------------------------------------------
include(ProjectSummary)