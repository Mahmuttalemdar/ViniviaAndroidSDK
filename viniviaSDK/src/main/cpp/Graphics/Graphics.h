#ifndef VINIVIA_SDK_GRAPHICS_H
#define VINIVIA_SDK_GRAPHICS_H

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <utility>
#include <jni.h>

namespace ViniviaSDK
{
    struct NativeContext {
        EGLDisplay display;
        EGLConfig config;
        EGLContext context;
        std::pair<ANativeWindow *, EGLSurface> windowSurface;
        EGLSurface pbufferSurface;
        GLuint program;
        GLint positionHandle;
        GLint samplerHandle;
        GLint vertTransformHandle;
        GLint texTransformHandle;
        GLuint textureId;

        NativeContext(EGLDisplay display, EGLConfig config, EGLContext context,
                      ANativeWindow *window, EGLSurface surface,
                      EGLSurface pbufferSurface)
                : display(display),
                  config(config),
                  context(context),
                  windowSurface(std::make_pair(window, surface)),
                  pbufferSurface(pbufferSurface),
                  program(0),
                  positionHandle(-1),
                  samplerHandle(-1),
                  vertTransformHandle(-1),
                  texTransformHandle(-1),
                  textureId(0) {}
    };

    /**
     * Instance class that is define ES2 and ES3 renderers
     */
    class Graphics {
    public:
        /**
         * C-tor
         */
        Graphics() = default;

        /**
         * D-tor
         */
        ~Graphics();

        /**
         * Load and compile shaders then create an instance
         * @param shaderType
         * @param src
         * @return
         */
        static GLuint CompileShader(GLenum shaderType, const char *shaderSrc);

        /**
         * Load opnegl program and create instance
         * @param vtxSrc
         * @param fragSrc
         * @return
         */
        static GLuint CreateGLProgram(const char *vertexSrc, const char *fragmentSrc);

        /**
         * Destroy surface from current context
         * @param nativeContext
         */
        static void DestroySurface(NativeContext *nativeContext);

        /**
         * Throw an exaception when something wrong on graphics engine
         * @param env
         * @param exceptionName
         * @param msg
         */
        static void ThrowException(_JNIEnv *env, const char *exceptionName, const char *msg);
    };
}
#endif  // VINIVIA_SDK_GRAPHICS_H
