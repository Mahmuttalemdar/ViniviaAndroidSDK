#ifndef VINIVIA_SDK_NEGATIVE_FILTER_H
#define VINIVIA_SDK_NEGATIVE_FILTER_H

#include "BaseFilter.h"
#include "Graphics/Graphics.h"
#include "Log/Log.h"
#include "Utils/OpenGLChecker.h"

#include <cassert>

namespace ViniviaSDK
{
    namespace Filters::NegativeFilter
    {
        // Vertex Shader
        constexpr char VERTEX_SHADER_SRC[] = R"SRC(
            attribute vec4 position;
            varying vec2 texCoord;
            uniform mat4 vertTransform;
            void main() {
                texCoord = ((vertTransform * vec4(position.xy, 0, 1.0)).xy
                + vec2(1.0, 1.0)) * 0.5;
                gl_Position = position;
            }
        )SRC";

        // Fragment Shader
        constexpr char FRAGMENT_SHADER_SRC[] = R"SRC(
            #extension GL_OES_EGL_image_external : require
            precision mediump float;
            uniform samplerExternalOES sampler;
            uniform mat4 texTransform;
            varying vec2 texCoord;
            void main() {
                vec2 transTexCoord = (texTransform * vec4(texCoord, 0, 1.0)).xy;
                vec4 originalColor = texture2D(sampler, transTexCoord);

                // Create negative color by subtracting each component from 1.0
                vec4 negativeColor = vec4(1.0 - originalColor.r, 1.0 - originalColor.g, 1.0 - originalColor.b, originalColor.a);

                gl_FragColor = negativeColor;
            }
        )SRC";
    }

    class NegativeFilter : public BaseFilter {
    public:
        /**
         * C-tor
         */
        NegativeFilter() : BaseFilter() {}

        /**
         * D-tor
         */
        ~NegativeFilter() override {}

        /**
         * Apply no-filter to current opengl context
         * @param nativeContext
         * @param filter
         * @return
         */
        jlong InitializeFilter(jlong context) override {
            auto *nativeContext = reinterpret_cast<NativeContext*>(context);
            if(!nativeContext){
                LOGE("InitializeFilter failed; nativeContext is not valid.");
                return 0;
            }

            // Create sahder program
            nativeContext->program = Graphics::CreateGLProgram(
                    ViniviaSDK::Filters::NegativeFilter::VERTEX_SHADER_SRC,
                    ViniviaSDK::Filters::NegativeFilter::FRAGMENT_SHADER_SRC);
            assert(nativeContext->program);

            // Position
            nativeContext->positionHandle = VINIVIA_CHECK_GL(glGetAttribLocation(nativeContext->program, "position"));
            assert(nativeContext->positionHandle != -1);

            // Sampler, External texture from camera
            nativeContext->samplerHandle = VINIVIA_CHECK_GL(glGetUniformLocation(nativeContext->program, "sampler"));
            assert(nativeContext->samplerHandle != -1);

            // Vertex Transform Array
            nativeContext->vertTransformHandle = VINIVIA_CHECK_GL(glGetUniformLocation(nativeContext->program, "vertTransform"));
            assert(nativeContext->vertTransformHandle != -1);

            // Texture Transform Aary
            nativeContext->texTransformHandle = VINIVIA_CHECK_GL(glGetUniformLocation(nativeContext->program, "texTransform"));
            assert(nativeContext->texTransformHandle != -1);

            // texture from native context
            VINIVIA_CHECK_GL(glGenTextures(1, &(nativeContext->textureId)));

            return reinterpret_cast<jlong>(nativeContext);
        }

        /**
         * Render filter and bind texture to openGl
         * @param nativeContext
         * @return
         */
        jlong RenderFilter(JNIEnv *env, jlong context, jfloatArray vertexTransformArray, jfloatArray textureTransformArray) override {
            auto *nativeContext = reinterpret_cast<NativeContext *>(context);
            if(!nativeContext){
                LOGE("RenderFilter failed; nativeContext is not valid.");
                return 0;
            }

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

            // Set Texture position to 0
            VINIVIA_CHECK_GL(glUniform1i(nativeContext->samplerHandle, 0));

            // Bind Texture
            VINIVIA_CHECK_GL(glBindTexture(GL_TEXTURE_EXTERNAL_OES, nativeContext->textureId));

            // Draw
            VINIVIA_CHECK_GL(glDrawArrays(GL_TRIANGLES, 0, 3));

            return reinterpret_cast<jlong>(nativeContext);
        }
    };

}

#endif //VINIVIA_SDK_GRAYSCALE_FILTER_H