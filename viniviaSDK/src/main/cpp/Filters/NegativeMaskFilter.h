#ifndef VINIVIA_SDK_NEGATIVE_MASK_FILTER_H
#define VINIVIA_SDK_NEGATIVE_MASK_FILTER_H

#include "BaseFilter.h"
#include "Graphics/Graphics.h"
#include "Log/Log.h"
#include "Utils/OpenGLChecker.h"

#include <cassert>

namespace ViniviaSDK {
    namespace Filters::NegativeMaskFilter {
        // Vertex Shader
        constexpr char VERTEX_SHADER_SRC[] = R"SRC(
            attribute vec4 position;
            varying vec2 texCoord;
            uniform mat4 vertTransform;

            void main()
            {
                texCoord = ((vertTransform * vec4(position.xy, 0, 1.0)).xy + vec2(1.0, 1.0)) * 0.5;
                gl_Position = position;
            }
        )SRC";

        // Fragment Shader
        constexpr char FRAGMENT_SHADER_SRC[] = R"SRC(
            precision mediump float;

            uniform mat4 texTransform;
            varying vec2 texCoord;

            uniform sampler2D maskTexture;
            uniform samplerExternalOES normalTexture;

            void main()
            {
                // Transformed Texture Coodinates
                vec2 transTexCoord = (texTransform * vec4(texCoord, 0, 1.0)).xy;

                // Read pixel values from the textures
                vec4 normalColor = texture2D(normalTexture, transTexCoord).rgba;
                vec4 maskColor = texture2D(maskTexture, transTexCoord).rgba;

                // Output the result
                gl_FragColor = mix(normalColor, maskColor, maskColor.r);
            }
        )SRC";
    }

    class NegativeMaskFilter : public BaseFilter {
    public:
        /**
         * C-tor
         */
        NegativeMaskFilter() : BaseFilter() {}

        /**
         * D-tor
         */
        ~NegativeMaskFilter() override {}

        /**
         * Apply no-filter to current opengl context
         * @param nativeContext
         * @param filter
         * @return
         */
        jlong InitializeFilter(jlong context) override {
            auto *nativeContext = reinterpret_cast<NativeContext *>(context);
            if (!nativeContext) {
                LOGE("InitializeFilter failed; nativeContext is not valid.");
                return 0;
            }

            // Enable blend operations for the masking
            VINIVIA_CHECK_GL(glEnable(GL_BLEND));
            // Use a simple blendfunc for drawing the background
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Create sahder program
            nativeContext->program = Graphics::CreateGLProgram(
                    ViniviaSDK::Filters::NegativeMaskFilter::VERTEX_SHADER_SRC,
                    ViniviaSDK::Filters::NegativeMaskFilter::FRAGMENT_SHADER_SRC);
            assert(nativeContext->program);

            // Position
            nativeContext->positionHandle = VINIVIA_CHECK_GL(
                    glGetAttribLocation(nativeContext->program, "position"));
            assert(nativeContext->positionHandle != -1);

            // Sampler, External texture from camera
            nativeContext->normalTextureHandle = VINIVIA_CHECK_GL(
                    glGetUniformLocation(nativeContext->program, "normalTexture"));
            assert(nativeContext->normalTextureHandle != -1);

            // Mask from image
            nativeContext->maskTextureHandle = VINIVIA_CHECK_GL(
                    glGetUniformLocation(nativeContext->program, "maskTexture"));
            assert(nativeContext->maskTextureHandle != -1);

            // Vertex Transform Array
            nativeContext->vertTransformHandle = VINIVIA_CHECK_GL(
                    glGetUniformLocation(nativeContext->program, "vertTransform"));
            assert(nativeContext->vertTransformHandle != -1);

            // Texture Transform Array
            nativeContext->texTransformHandle = VINIVIA_CHECK_GL(
                    glGetUniformLocation(nativeContext->program, "texTransform"));
            assert(nativeContext->texTransformHandle != -1);

            // texture from native context
            VINIVIA_CHECK_GL(glGenTextures(1, &(nativeContext->normalTextureId)));

            return reinterpret_cast<jlong>(nativeContext);
        }

        /**
         * Render filter and bind texture to openGl
         * @param nativeContext
         * @return
         */
        jlong RenderFilter(JNIEnv *env, jlong context, jfloatArray vertexTransformArray,
                           jfloatArray textureTransformArray) override {
            auto *nativeContext = reinterpret_cast<NativeContext *>(context);
            if (!nativeContext) {
                LOGE("RenderFilter failed; nativeContext is not valid.");
                return 0;
            }
            // Set Background (Black)
            VINIVIA_CHECK_GL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));

            // Clear color and depth values
            VINIVIA_CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

            // Image vertices
            constexpr GLfloat vertices[] = {-1.0f, -1.0f, 3.0f, -1.0f, -1.0f, 3.0f};

            GLint vertexComponents = 2;
            GLenum vertexType = GL_FLOAT;
            GLboolean normalized = GL_FALSE;
            GLsizei vertexStride = 0;

            // Enable Position
            {
                VINIVIA_CHECK_GL(glVertexAttribPointer(nativeContext->positionHandle,
                                                       vertexComponents, vertexType, normalized,
                                                       vertexStride, vertices));
                VINIVIA_CHECK_GL(glEnableVertexAttribArray(nativeContext->positionHandle));
            }

            // Use gl program
            VINIVIA_CHECK_GL(glUseProgram(nativeContext->program));

            GLsizei numMatrices = 1;
            GLboolean transpose = GL_FALSE;

            // Vertex Position Handle
            {

                GLfloat *vertTransformArray =
                        env->GetFloatArrayElements(vertexTransformArray, nullptr);
                VINIVIA_CHECK_GL(glUniformMatrix4fv(nativeContext->vertTransformHandle, numMatrices,
                                                    transpose, vertTransformArray));
                env->ReleaseFloatArrayElements(vertexTransformArray, vertTransformArray,
                                               JNI_ABORT);
            }

            // Texture Position Handle
            {
                GLfloat *texTransformArray =
                        env->GetFloatArrayElements(textureTransformArray, nullptr);
                VINIVIA_CHECK_GL(glUniformMatrix4fv(nativeContext->texTransformHandle, numMatrices,
                                                    transpose, texTransformArray));
                env->ReleaseFloatArrayElements(textureTransformArray, texTransformArray,
                                               JNI_ABORT);
            }

            // Bind Normal Texture
            bindNormalTexture(nativeContext);

            // Bind Mask Texture
            bindMaskTexture(nativeContext);

            // Do all draw operations
            drawTextures();

            // Disable Blend
            VINIVIA_CHECK_GL(glDisable(GL_BLEND));

            return reinterpret_cast<jlong>(nativeContext);
        }

    private:
        void bindNormalTexture(NativeContext *nativeContext) {
            // Set Normal Texture position to 1
            VINIVIA_CHECK_GL(glUniform1i(nativeContext->normalTextureHandle, 0));

            // Set active texture 1
            VINIVIA_CHECK_GL(glActiveTexture(GL_TEXTURE0));

            // Bind Texture
            VINIVIA_CHECK_GL(glBindTexture(GL_TEXTURE_EXTERNAL_OES, nativeContext->normalTextureId));
        }

        void bindMaskTexture(NativeContext *nativeContext) {
            // Set Mask Texture position to 0
            VINIVIA_CHECK_GL(glUniform1i(nativeContext->maskTextureHandle, 1));

            // Set active texture 0
            VINIVIA_CHECK_GL(glActiveTexture(GL_TEXTURE1));

            // Bind Texture
            VINIVIA_CHECK_GL(glBindTexture(GL_TEXTURE_EXTERNAL_OES, nativeContext->maskTextureId));
        }

        void drawTextures() {
            // Draw
            VINIVIA_CHECK_GL(glDrawArrays(GL_TRIANGLES, 0, 3));
        }
    };

}

#endif //VINIVIA_SDK_NEGATIVE_MASK_FILTER_H