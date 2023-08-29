#ifndef VINIVIA_SDK_LOG_H
#define VINIVIA_SDK_LOG_H

#include <android/log.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <sstream>
#include <string>
#include <iomanip>

/**
 * Definition of log levels, its possible to define custom log level for example "trace"
 */
#define LOG_VERBOSE 1
#define LOG_DEBUG 2
#define LOG_INFO 3
#define LOG_WARN 4
#define LOG_ERROR 5

/**
 * Define generic tag for sdk, backend, c++
 */
auto constexpr LOG_TAG = "Vinivia SDK JNI";

/**
 * Set default log level = LOG_DEBUG
 */
#define LOG_LEVEL LOG_DEBUG

#ifndef LOG_LEVEL
#define LOGV(...)
#define LOGD(...)
#define LOGI(...)
#define LOGW(...)
#define LOGE(...)
#else
#if LOG_LEVEL <= LOG_VERBOSE
        #define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__);
    #endif
    #if LOG_LEVEL <= LOG_DEBUG
        #define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__);
    #endif
    #if LOG_LEVEL <= LOG_INFO
        #define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__);
    #endif
    #if LOG_LEVEL <= LOG_WARN
        #define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__);
    #endif
    #if LOG_LEVEL <= LOG_ERROR
        #define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__);
    #endif
#endif

namespace ViniviaSDK {
    inline std::string GLErrorString(GLenum error) {
        switch (error) {
            case GL_NO_ERROR:
                return "GL_NO_ERROR";
            case GL_INVALID_ENUM:
                return "GL_INVALID_ENUM";
            case GL_INVALID_VALUE:
                return "GL_INVALID_VALUE";
            case GL_INVALID_OPERATION:
                return "GL_INVALID_OPERATION";
            case GL_STACK_OVERFLOW_KHR:
                return "GL_STACK_OVERFLOW";
            case GL_STACK_UNDERFLOW_KHR:
                return "GL_STACK_UNDERFLOW";
            case GL_OUT_OF_MEMORY:
                return "GL_OUT_OF_MEMORY";
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                return "GL_INVALID_FRAMEBUFFER_OPERATION";
            default: {
                std::ostringstream oss;
                oss << "<Unknown GL Error 0x" << std::setfill('0') <<
                    std::setw(4) << std::right << std::hex << error << ">";
                return oss.str();
            }
        }
    }

    inline std::string EGLErrorString(EGLenum error) {
        switch (error) {
            case EGL_SUCCESS:
                return "EGL_SUCCESS";
            case EGL_NOT_INITIALIZED:
                return "EGL_NOT_INITIALIZED";
            case EGL_BAD_ACCESS:
                return "EGL_BAD_ACCESS";
            case EGL_BAD_ALLOC:
                return "EGL_BAD_ALLOC";
            case EGL_BAD_ATTRIBUTE:
                return "EGL_BAD_ATTRIBUTE";
            case EGL_BAD_CONTEXT:
                return "EGL_BAD_CONTEXT";
            case EGL_BAD_CONFIG:
                return "EGL_BAD_CONFIG";
            case EGL_BAD_CURRENT_SURFACE:
                return "EGL_BAD_CURRENT_SURFACE";
            case EGL_BAD_DISPLAY:
                return "EGL_BAD_DISPLAY";
            case EGL_BAD_SURFACE:
                return "EGL_BAD_SURFACE";
            case EGL_BAD_MATCH:
                return "EGL_BAD_MATCH";
            case EGL_BAD_PARAMETER:
                return "EGL_BAD_PARAMETER";
            case EGL_BAD_NATIVE_PIXMAP:
                return "EGL_BAD_NATIVE_PIXMAP";
            case EGL_BAD_NATIVE_WINDOW:
                return "EGL_BAD_NATIVE_WINDOW";
            case EGL_CONTEXT_LOST:
                return "EGL_CONTEXT_LOST";
            default: {
                std::ostringstream oss;
                oss << "<Unknown EGL Error 0x" << std::setfill('0') <<
                    std::setw(4) << std::right << std::hex << error << ">";
                return oss.str();
            }
        }
    }

    inline const char *ShaderTypeString(GLenum shaderType) {
        switch (shaderType) {
            case GL_VERTEX_SHADER:
                return "GL_VERTEX_SHADER";
            case GL_FRAGMENT_SHADER:
                return "GL_FRAGMENT_SHADER";
            default:
                return "<Unknown shader type>";
        }
    }
}

#endif  // VINIVIA_SDK_LOG_H
