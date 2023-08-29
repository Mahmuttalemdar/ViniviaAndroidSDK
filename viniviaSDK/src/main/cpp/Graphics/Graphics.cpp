#include "Graphics.h"
#include "Log/Log.h"
#include "Utils/OpenGLChecker.h"

#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

#include <cassert>
#include <string.h>
#include <vector>

namespace ViniviaSDK {
    Graphics::~Graphics() {}

    GLuint Graphics::CompileShader(GLenum shaderType, const char *shaderSrc) {
        // Create shader instance by type
        GLuint shader = VINIVIA_CHECK_GL(glCreateShader(shaderType));
        assert(shader);

        // Prepare shader sources
        VINIVIA_CHECK_GL(glShaderSource(shader, 1, &shaderSrc, /*length=*/nullptr));

        // Compile shaders
        VINIVIA_CHECK_GL(glCompileShader(shader));

        GLint compileStatus = 0;
        // Get compile status
        VINIVIA_CHECK_GL(glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus));

        if (!compileStatus) {
            GLint logLength = 0;
            // Get compiled shader info and check is valid
            VINIVIA_CHECK_GL(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength));

            std::vector<char> logBuffer(logLength);
            if (logLength > 0) {
                VINIVIA_CHECK_GL(glGetShaderInfoLog(shader, logLength, /*length=*/nullptr,
                                                    &logBuffer[0]));
            }

            LOGE("Unable to compile %s shader:\n %s.",
                 ShaderTypeString(shaderType),
                 logLength > 0 ? &logBuffer[0] : "(unknown error)");

            // Safe delete for shader
            VINIVIA_CHECK_GL(glDeleteShader(shader));
            shader = 0;
        }

        assert(shader);
        return shader;
    }

    GLuint Graphics::CreateGLProgram(const char *vertexSrc, const char *fragmentSrc) {
        // Compile vertex shaders and create an instance
        GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSrc);
        assert(vertexShader);

        // Compile fragment shaders and create an instance
        GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);
        assert(fragmentShader);

        // Create open gl program instance
        GLuint program = VINIVIA_CHECK_GL(glCreateProgram());
        assert(program);

        // Attach vertex shaders to current context
        VINIVIA_CHECK_GL(glAttachShader(program, vertexShader));

        // Attach fragment shaders to current context
        VINIVIA_CHECK_GL(glAttachShader(program, fragmentShader));

        // Link opengl program
        VINIVIA_CHECK_GL(glLinkProgram(program));

        GLint linkStatus = 0;
        // Get link status
        VINIVIA_CHECK_GL(glGetProgramiv(program, GL_LINK_STATUS, &linkStatus));
        if (!linkStatus) {
            GLint logLength = 0;
            // Get compiled program info and check is valid
            VINIVIA_CHECK_GL(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength));

            std::vector<char> logBuffer(logLength);
            if (logLength > 0) {
                VINIVIA_CHECK_GL(glGetProgramInfoLog(program, logLength, /*length=*/nullptr,
                                                     &logBuffer[0]));
            }

            LOGE("Unable to link program:\n %s.",
                 logLength > 0 ? &logBuffer[0] : "(unknown error)");

            // Delete opengl program
            VINIVIA_CHECK_GL(glDeleteProgram(program));
            program = 0;
        }

        assert(program);
        return program;
    }

    void Graphics::DestroySurface(NativeContext *nativeContext) {
        if (nativeContext->windowSurface.first) {
            eglMakeCurrent(nativeContext->display, nativeContext->pbufferSurface,
                           nativeContext->pbufferSurface, nativeContext->context);
            eglDestroySurface(nativeContext->display,
                              nativeContext->windowSurface.second);
            nativeContext->windowSurface.second = nullptr;
            ANativeWindow_release(nativeContext->windowSurface.first);
            nativeContext->windowSurface.first = nullptr;
        }
    }

    void Graphics::ThrowException(JNIEnv *env, const char *exceptionName, const char *msg) {
        jclass exClass = env->FindClass(exceptionName);
        assert(exClass != nullptr);

        [[maybe_unused]] jint throwSuccess = env->ThrowNew(exClass, msg);
        assert(throwSuccess == JNI_OK);
    }
}


