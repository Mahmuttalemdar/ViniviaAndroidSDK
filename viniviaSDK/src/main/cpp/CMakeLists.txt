# -------------------------------------------------------------------------------------------------
# PROJECT: ViniviaAndroidSDK
# -------------------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.10.2)
set(CMAKE_CXX_STANDARD 17)

# -------------------------------------------------------------------------------------------------
# Add cmake module paths
# -------------------------------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/")

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
        ${CMAKE_CURRENT_LIST_DIR}/Bridge/jni_hooks.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Bridge/ViniviaSDKBridge.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Core/Entrance.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Graphics/Graphics.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Graphics/Renderer.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Graphics/RendererES2.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Utils/OpenGLChecker.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Utils/Matrix.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Filters/BaseFilter.cpp
        )

set(HEADERS
        ${CMAKE_CURRENT_LIST_DIR}/Core/Entrance_p.h
        ${CMAKE_CURRENT_LIST_DIR}/Core/Entrance.h
        ${CMAKE_CURRENT_LIST_DIR}/Graphics/Graphics.h
        ${CMAKE_CURRENT_LIST_DIR}/Graphics/Renderer.h
        ${CMAKE_CURRENT_LIST_DIR}/Graphics/RendererES2.h
        ${CMAKE_CURRENT_LIST_DIR}/Log/Log.h
        ${CMAKE_CURRENT_LIST_DIR}/Utils/OpenGLChecker.h
        ${CMAKE_CURRENT_LIST_DIR}/Utils/Matrix.h
        ${CMAKE_CURRENT_LIST_DIR}/Filters/BaseFilter.h
        ${CMAKE_CURRENT_LIST_DIR}/Filters/NoFilter.h
        ${CMAKE_CURRENT_LIST_DIR}/Filters/CubeFilter.h
        ${CMAKE_CURRENT_LIST_DIR}/Filters/CubeWithDepthMapFilter.h
        ${CMAKE_CURRENT_LIST_DIR}/Filters/DepthMapFilter.h
        ${CMAKE_CURRENT_LIST_DIR}/Filters/GrayscaleFilter.h
        ${CMAKE_CURRENT_LIST_DIR}/Filters/NegativeFilter.h
        ${CMAKE_CURRENT_LIST_DIR}/Shapes/Cube.h
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
        ${PROJECT_NAME} PUBLIC -fno-rtti -fno-exceptions -Wall
)

# -------------------------------------------------------------------------------------------------
# Find packages
# -------------------------------------------------------------------------------------------------
find_library(log-lib log)
find_library(android-lib android)
find_library(opengl-lib GLESv2)
find_library(egl-lib EGL)

#find_package(KYCEngine CONFIG REQUIRED)

# -------------------------------------------------------------------------------------------------
# Library dependencies
# -------------------------------------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME} PUBLIC
        ${log-lib}
        ${android-lib}
        ${opengl-lib}
        ${egl-lib}
        # ${KYCENGINE_LIBRARIES}
        )

# -------------------------------------------------------------------------------------------------
# Include directories
# -------------------------------------------------------------------------------------------------
target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
#        ${KYCENGINE_DIRS}
        )

# -------------------------------------------------------------------------------------------------
# Project Summary
# -------------------------------------------------------------------------------------------------
include(ProjectSummary)