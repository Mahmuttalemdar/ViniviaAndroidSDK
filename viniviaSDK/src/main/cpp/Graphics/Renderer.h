#ifndef VINIVIA_SDK_RENDERER_H
#define VINIVIA_SDK_RENDERER_H

#include "Filters/BaseFilter.h"

#include <jni.h>
#include <cstdint>
#include <memory>

namespace ViniviaSDK
{
    /**
     * Instance class that is define ES2 and ES3 renderers
     */
    class Renderer {
    protected:
        /**
         * C-tor
         */
        Renderer();

    public:
        /**
         * D-tor
         */
        virtual ~Renderer() = 0;

        /**
         * Initialize native context for graphics engine
         * @return native context
         */
        virtual jlong InitializeContext(JNIEnv *env) = 0;

        /**
         * Destroy current opengl context
         */
        virtual void DestroyContext(jlong context) = 0;

        /**
         * Render function that is heart of renderer
         * @param vertexTransformArray
         * @param textureTransformArray
         * @return
         */
        virtual jboolean RenderTexture(JNIEnv *env, jlong context, jfloatArray vertexTransformArray, jfloatArray textureTransformArray) = 0;

        /**
         * Set window surface
         * @param context
         * @param surface
         * @return
         */
        virtual jboolean SetWindowSurface(JNIEnv *env, jlong context, jobject jSurface) = 0;

        /**
         * Set mask for shader filters
         * @param env
         * @param context
         * @param mask
         * @return
         */
        virtual jboolean SetMask(JNIEnv *env, jlong context, jobject mask) = 0;

        /**
         * Get current texture id
         * @param context
         * @return
         */
        virtual jint GetTextureID(jlong context) = 0;

        /**
         * Set current filter
         * @param filter
         */
        void SetFilter(const Filter filter);

        /**
         * Get current filter
         * @return
         */
        const Filter GetFilter() const;

        /**
         * Get activated shader filter
         * @return
         */
        BaseFilter *const ActiveShaderFilter() const;

    private:
        Filter m_filter = Filter::NO_FILTER;
        std::unique_ptr<BaseFilter> m_shaderFilter;
    };
}

#endif  // VINIVIA_SDK_RENDERER_H
