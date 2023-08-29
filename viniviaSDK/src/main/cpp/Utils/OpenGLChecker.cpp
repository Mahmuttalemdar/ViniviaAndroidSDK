#include "OpenGLChecker.h"

#include "Log/Log.h"

#include <type_traits>

namespace ViniviaSDK
{
    CheckGlErrorOnExit::CheckGlErrorOnExit(std::string glFunStr, unsigned int lineNum) :
            mGlFunStr(std::move(glFunStr)),
            mLineNum(lineNum) {}

    CheckGlErrorOnExit::~CheckGlErrorOnExit() {
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            __android_log_assert(nullptr, LOG_TAG, "OpenGL Error: %s at %s [%s:%d]",
                                 GLErrorString(err).c_str(), mGlFunStr.c_str(), __FILE__,
                                 mLineNum);
        }
    }
}
