#ifndef VINIVIA_OPENGL_CHECKER_H
#define VINIVIA_OPENGL_CHECKER_H

#include <string>

#ifdef NDEBUG
#define VINIVIA_CHECK_GL(gl_func) [&]() { return gl_func; }()
#else

namespace ViniviaSDK
{
    class CheckGlErrorOnExit {
    public:
        explicit CheckGlErrorOnExit(std::string glFunStr, unsigned int lineNum);

        ~CheckGlErrorOnExit();

        CheckGlErrorOnExit(const CheckGlErrorOnExit &) = delete;

        CheckGlErrorOnExit &operator=(const CheckGlErrorOnExit &) = delete;

    private:
        std::string mGlFunStr;
        unsigned int mLineNum;
    };
}


#define VINIVIA_CHECK_GL(glFunc)                                            \
  [&]() {                                                                   \
    auto assertOnExit = ViniviaSDK::CheckGlErrorOnExit(#glFunc, __LINE__);              \
    return glFunc;                                                          \
  }()
#endif

#endif  // VINIVIA_OPENGL_CHECKER_H
