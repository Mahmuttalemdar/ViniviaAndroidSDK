#ifndef VINIVIA_OPENGL_CHECKER_H
#define VINIVIA_OPENGL_CHECKER_H

#include <string>

namespace ViniviaSDK
{
    class OpenGLChecker {
    public:
        explicit OpenGLChecker(std::string glFunStr, unsigned int lineNum);

        ~OpenGLChecker();

        OpenGLChecker(const OpenGLChecker &) = delete;

        OpenGLChecker &operator=(const OpenGLChecker &) = delete;

    private:
        std::string mGlFunStr;
        unsigned int mLineNum;
    };
}


#define VINIVIA_CHECK_GL(glFunc)                                            \
  [&]() {                                                                   \
    auto assertOnExit = ViniviaSDK::OpenGLChecker(#glFunc, __LINE__);       \
    return glFunc;                                                          \
  }()

#endif  // VINIVIA_OPENGL_CHECKER_H
