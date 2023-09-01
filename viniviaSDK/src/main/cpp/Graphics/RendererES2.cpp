#include "RendererES2.h"
#include "Graphics/Graphics.h"
#include "Log/Log.h"
#include "Utils/OpenGLChecker.h"
#include "Filters/NoFilter.h"
#include "Filters/GrayscaleFilter.h"
#include "Filters/NegativeFilter.h"

#include <android/bitmap.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

#include <cassert>
#include <string.h>

namespace ViniviaSDK {
    RendererES2::RendererES2()
            : Renderer(), m_configAttributes({EGL_RENDERABLE_TYPE,
                                              EGL_OPENGL_ES2_BIT,
                                              EGL_SURFACE_TYPE,
                                              EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
                                              EGL_RECORDABLE_ANDROID,
                                              EGL_TRUE,
                                              EGL_NONE}),
              m_contextAttributes({EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE}),
              m_bufferAttributes({EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE}) {}

    RendererES2::~RendererES2() {}

    jlong RendererES2::InitializeContext(JNIEnv *env) {
        EGLDisplay eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        assert(eglDisplay != EGL_NO_DISPLAY);

        EGLint majorVer;
        EGLint minorVer;
        // Initialize egl
        EGLBoolean initSuccess = eglInitialize(eglDisplay, &majorVer, &minorVer);
        if (initSuccess != EGL_TRUE) {
            Graphics::ThrowException(env, "java/lang/RuntimeException",
                                     "EGL Error: eglInitialize failed.");
            return 0;
        }

        // Print debug EGL information
        const char *eglVendorString = eglQueryString(eglDisplay, EGL_VENDOR);
        const char *eglVersionString = eglQueryString(eglDisplay, EGL_VERSION);
        LOGD("EGL Initialized [Vendor: %s, Version: %s]",
             eglVendorString == nullptr ? "Unknown" : eglVendorString,
             eglVersionString == nullptr
             ? "Unknown" : eglVersionString);

        EGLConfig config;
        EGLint numConfigs;
        EGLint configSize = 1;
        EGLBoolean chooseConfigSuccess =
                eglChooseConfig(eglDisplay, static_cast<EGLint *>(m_configAttributes.data()),
                                &config,
                                configSize, &numConfigs);
        if (chooseConfigSuccess != EGL_TRUE) {
            Graphics::ThrowException(env, "java/lang/IllegalArgumentException",
                                     "EGL Error: eglChooseConfig failed. ");
            return 0;
        }
        assert(numConfigs > 0);

        EGLContext eglContext = eglCreateContext(eglDisplay, config, EGL_NO_CONTEXT,
                                                 static_cast<EGLint *>(m_contextAttributes.data()));
        assert(eglContext != EGL_NO_CONTEXT);

        // Create 1x1 pixmap to use as a surface until one is set.
        EGLSurface eglPbuffer = eglCreatePbufferSurface(eglDisplay, config,
                                                        m_bufferAttributes.data());
        assert(eglPbuffer != EGL_NO_SURFACE);

        eglMakeCurrent(eglDisplay, eglPbuffer, eglPbuffer, eglContext);

        //Print debug OpenGL information
        const GLubyte *glVendorString = VINIVIA_CHECK_GL(glGetString(GL_VENDOR));
        const GLubyte *glVersionString = VINIVIA_CHECK_GL(glGetString(GL_VERSION));
        const GLubyte *glslVersionString = VINIVIA_CHECK_GL(
                glGetString(GL_SHADING_LANGUAGE_VERSION));
        const GLubyte *glRendererString = VINIVIA_CHECK_GL(glGetString(GL_RENDERER));
        LOGD("OpenGL Initialized [Vendor: %s, Version: %s,"
             " GLSL Version: %s, Renderer: %s]",
             glVendorString == nullptr ? "Unknown" : (const char *) glVendorString,
             glVersionString == nullptr ? "Unknown" : (const char *) glVersionString,
             glslVersionString == nullptr ? "Unknown" : (const char *) glslVersionString,
             glRendererString == nullptr ? "Unknown" : (const char *) glRendererString);

        auto *nativeContext =
                new NativeContext(eglDisplay, config, eglContext, /*window=*/nullptr,
                        /*surface=*/nullptr, eglPbuffer);

        return ActiveShaderFilter()->InitializeFilter(reinterpret_cast<jlong>(nativeContext));
    }

    void RendererES2::DestroyContext(jlong context) {
        auto *nativeContext = reinterpret_cast<NativeContext *>(context);

        if (nativeContext->program) {
            VINIVIA_CHECK_GL(glDeleteProgram(nativeContext->program));
            nativeContext->program = 0;
        }

        Graphics::DestroySurface(nativeContext);
        eglDestroySurface(nativeContext->display, nativeContext->pbufferSurface);
        eglMakeCurrent(nativeContext->display, EGL_NO_SURFACE, EGL_NO_SURFACE,
                       EGL_NO_CONTEXT);
        eglDestroyContext(nativeContext->display, nativeContext->context);
        eglTerminate(nativeContext->display);

        delete nativeContext;
    }

    jboolean RendererES2::RenderTexture(JNIEnv *env, jlong context,
                                        jfloatArray vertexTransformArray,
                                        jfloatArray textureTransformArray) {


        //  Render active filter and depended parameters than return the context
        auto *nativeContext = reinterpret_cast<NativeContext *>(
                ActiveShaderFilter()->RenderFilter(env, context, vertexTransformArray,
                                                   textureTransformArray));

        // Check that all GL operations completed successfully. If not, log an error and return.
        GLenum glError = glGetError();
        if (glError != GL_NO_ERROR) {
            LOGE("Failed to draw frame due to OpenGL error: %s", GLErrorString(glError).c_str());
            return JNI_FALSE;
        }

// Only attempt to set presentation time if EGL_EGLEXT_PROTOTYPES is defined.
#ifdef EGL_EGLEXT_PROTOTYPES
        eglPresentationTimeANDROID(nativeContext->display,
                               nativeContext->windowSurface.second, timestampNs);
#endif  // EGL_EGLEXT_PROTOTYPES
        EGLBoolean swapped = eglSwapBuffers(nativeContext->display,
                                            nativeContext->windowSurface.second);
        if (!swapped) {
            EGLenum eglError = eglGetError();
            LOGE("Failed to swap buffers with EGL error: %s", EGLErrorString(eglError).c_str());
            return JNI_FALSE;
        }

        return JNI_TRUE;
    }

    jboolean RendererES2::SetWindowSurface(JNIEnv *env, jlong context, jobject jSurface) {
        auto *nativeContext = reinterpret_cast<NativeContext *>(context);

        // Destroy previously connected surface
        Graphics::DestroySurface(nativeContext);

        // Null surface may have just been passed in to destroy previous surface.
        if (!jSurface) {
            return JNI_FALSE;
        }

        ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, jSurface);
        if (nativeWindow == nullptr) {
            LOGE("Failed to set window surface: Unable to acquire native window.");
            return JNI_FALSE;
        }

        EGLSurface surface = eglCreateWindowSurface(nativeContext->display, nativeContext->config,
                                                    nativeWindow, /*attrib_list=*/nullptr);
        assert(surface != EGL_NO_SURFACE);

        nativeContext->windowSurface = std::make_pair(nativeWindow, surface);
        eglMakeCurrent(nativeContext->display, surface, surface,
                       nativeContext->context);

        m_viewportWidth = ANativeWindow_getWidth(nativeWindow);
        m_viewportHeigth = ANativeWindow_getHeight(nativeWindow);

        VINIVIA_CHECK_GL(glViewport(0, 0, m_viewportWidth, m_viewportHeigth));
        VINIVIA_CHECK_GL(glScissor(0, 0, ANativeWindow_getWidth(nativeWindow),
                                   ANativeWindow_getHeight(nativeWindow)));

        return JNI_TRUE;
    }

    jboolean RendererES2::SetMask(JNIEnv *env, jlong context, jobject mask) {
        AndroidBitmapInfo bitmapInfo;
        void *bitmapPixels;

        if (AndroidBitmap_getInfo(env, mask, &bitmapInfo) < 0) {
            LOGE("Failed to set mask : Unable to access bitmapInfo.");
            return JNI_FALSE;
        }

        if (AndroidBitmap_lockPixels(env, mask, &bitmapPixels) < 0) {
            LOGE("Failed to set mask : Unable to access bitmapPixels.");
            return JNI_FALSE;
        }
        auto *nativeContext = reinterpret_cast<NativeContext *>(context);

        if (!nativeContext) {
            LOGE("Failed to set mask : Unable to access nativeContext.");
            return JNI_FALSE;
        }

        // Remove old mask
        glDeleteTextures(1, &nativeContext->maskTextureId);

        // Create OpenGL Texture
        glGenTextures(1, &nativeContext->maskTextureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmapInfo.width, bitmapInfo.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmapPixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        AndroidBitmap_unlockPixels(env, mask);
        return JNI_TRUE;
    }

    jint RendererES2::GetTextureID(jlong context) {
        auto *nativeContext = reinterpret_cast<NativeContext *>(context);
        return nativeContext->normalTextureId;
    }


}