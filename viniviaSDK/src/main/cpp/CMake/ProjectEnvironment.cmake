# -------------------------------------------------------------------------------------------------
# Artifact Management
# -------------------------------------------------------------------------------------------------
set(ARTIFACTS_DIR "${PROJECT_ROOT_DIR}/artifacts" CACHE PATH "")
set(ARTIFACTS_OUT_DIR ${PROJECT_ROOT_DIR}/artifactsOut/${CMAKE_SYSTEM_NAME}-${ANDROID_ABI} CACHE PATH "")
set(ARTIFACTS_SEARCH_DIR ${ARTIFACTS_DIR}/${CMAKE_SYSTEM_NAME}-${ANDROID_ABI} CACHE PATH "")

# -------------------------------------------------------------------------------------------------
# General folders
# -------------------------------------------------------------------------------------------------
list(APPEND CMAKE_PREFIX_PATH "${ARTIFACTS_DIR}/CMake/common/Modules/")
list(REMOVE_DUPLICATES CMAKE_PREFIX_PATH)

# -------------------------------------------------------------------------------------------------
# Artifact Management
# -------------------------------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${ARTIFACTS_DIR}/CMake/common/Modules/")
list(APPEND CMAKE_MODULE_PATH "${ARTIFACTS_SEARCH_DIR}/sdk/native/jni/")


# Newer versions of the Android Toolchain only search find_package in SYSROOT directories
# by overwriting the CMAKE_FIND_ROOT_PATH_MODE_... values. Since we need our builds to recognize
# packages in artifacts folder we must override these settings
if (ANDROID)
    set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH )
    set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH )
    set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH )
    set( CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH )
endif()