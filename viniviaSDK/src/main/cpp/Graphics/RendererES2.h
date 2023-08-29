#ifndef VINIVIA_SDK_RENDERER_ES2_H
#define VINIVIA_SDK_RENDERER_ES2_H

#include "Renderer.h"

#include <vector>

namespace ViniviaSDK
{
    /**
     * Instance class that is define ES2 and ES3 renderers
     */
    class RendererES2 : public Renderer {
    public:
        /**
         * C-tor
         */
        RendererES2();

        /**
         * D-tor
         */
        virtual ~RendererES2();

        /**
         * Override initialize native context for graphics engine
         * @return native context
         */
        jlong InitializeContext(JNIEnv *env) override;

        /**
         * Override destroy current opengl context
         */
        void DestroyContext(jlong context) override;

        /**
         * Override Render function that is heart of renderer
         * @param vertexTransformArray
         * @param textureTransformArray
         * @return
         */
        jboolean RenderTexture(JNIEnv *env, jlong context, jfloatArray vertexTransformArray, jfloatArray textureTransformArray) override;

        /**
         * Override Set window surface
         * @param context
         * @param surface
         * @return
         */
        jboolean SetWindowSurface(JNIEnv *env, jlong context, jobject jSurface) override;

        /**
         * Override Get current texture id
         * @param context
         * @return
         */
        jint GetTextureID(jlong context) override;

    private:
        std::vector<int> m_configAttributes;
        std::vector<int> m_contextAttributes;
        std::vector<int> m_bufferAttributes;
        int32_t m_viewportWidth = 0;
        int32_t m_viewportHeigth = 0;
    };
}

#endif  // VINIVIA_SDK_RENDERER_ES2_H
