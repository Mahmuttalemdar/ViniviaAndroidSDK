#ifndef VINIVIA_SDK_CUBE_WITH_MASK_FILTER_H
#define VINIVIA_SDK_CUBE_WITH_MASK_FILTER_H

#include "BaseFilter.h"
#include "Filters/PositiveMaskFilter.h"
#include "Graphics/Graphics.h"
#include "Log/Log.h"
#include "Utils/OpenGLChecker.h"

#include <cassert>


namespace ViniviaSDK
{
    namespace Filters::CubeOverMaskFilter
    {
        // Vertex Shader for Cube
        constexpr char VERTEX_SHADER_SRC[] = R"SRC(
            attribute vec4 vertexPosition;
            attribute vec3 vertexColour;
            uniform mat4 projection;
            uniform mat4 modelView;
            varying vec3 fragColour;
            void main() {
                gl_Position = projection * modelView * vertexPosition;
                fragColour = vertexColour;
            }
        )SRC";

        // Fragment Shader for Cube
        constexpr char FRAGMENT_SHADER_SRC[] = R"SRC(
            precision mediump float;
            varying vec3 fragColour;
            void main() {
                gl_FragColor = vec4(fragColour, 1.0); // Vertex color
            }
        )SRC";
    }

    class CubeOverMaskFilter : public BaseFilter {
    public:
        /**
         * C-tor
         */
        CubeOverMaskFilter() : BaseFilter() {}

        /**
         * D-tor
         */
        ~CubeOverMaskFilter() override {}

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
                    ViniviaSDK::Filters::PositiveMaskFilter::VERTEX_SHADER_SRC,
                    ViniviaSDK::Filters::PositiveMaskFilter::FRAGMENT_SHADER_SRC);
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

            // Setup the perspective for 3D Cube.
            matrixPerspective(m_projectionMatrix, 45.0f, (float) 3 / (float) 4, 0.1f, 100.0f);

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

            // Render cube
            return RenderCube(env, reinterpret_cast<jlong>(nativeContext), vertexTransformArray,
                              textureTransformArray);
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

        /**
         * Render filter and bind texture to openGl
         * @param nativeContext
         * @return
         */
        jlong RenderCube(JNIEnv *env, jlong context, jfloatArray vertexTransformArray,
                         jfloatArray textureTransformArray) {
            auto *nativeContext = reinterpret_cast<NativeContext *>(context);
            if (!nativeContext) {
                LOGE("RenderFilter failed; nativeContext is not valid.");
                return 0;
            }

            GLuint cubeProgram = Graphics::CreateGLProgram(
                    ViniviaSDK::Filters::CubeFilter::VERTEX_SHADER_SRC,
                    ViniviaSDK::Filters::CubeFilter::FRAGMENT_SHADER_SRC);
            assert(cubeProgram);

            GLint vertexPositionHandle = VINIVIA_CHECK_GL(
                    glGetAttribLocation(cubeProgram, "vertexPosition"));
            assert(vertexPositionHandle != -1);

            GLint vertexColourHandle = VINIVIA_CHECK_GL(
                    glGetAttribLocation(cubeProgram, "vertexColour"));
            assert(vertexColourHandle != -1);

            GLint projectionHandle = VINIVIA_CHECK_GL(
                    glGetUniformLocation(cubeProgram, "projection"));
            assert(projectionHandle != -1);

            GLint modelViewHandle = VINIVIA_CHECK_GL(
                    glGetUniformLocation(cubeProgram, "modelView"));
            assert(modelViewHandle != -1);

            // Updte perspective
            UpdatePerspective();

            // Use gl program
            VINIVIA_CHECK_GL(glUseProgram(cubeProgram));

            GLint vertexComponents = 3;
            GLenum vertexType = GL_FLOAT;
            GLboolean normalized = GL_FALSE;
            GLsizei vertexStride = 0;

            // Vertex Position Handle
            VINIVIA_CHECK_GL(glVertexAttribPointer(vertexPositionHandle,
                                                   vertexComponents, vertexType, normalized,
                                                   vertexStride, ViniviaSDK::Shapes::Cube::CubeVertices));
            VINIVIA_CHECK_GL(glEnableVertexAttribArray(vertexPositionHandle));

            // vertex Colour Handle
            VINIVIA_CHECK_GL(glVertexAttribPointer(vertexColourHandle,
                                                   vertexComponents, vertexType, normalized,
                                                   vertexStride, ViniviaSDK::Shapes::Cube::CubeColour));
            VINIVIA_CHECK_GL(glEnableVertexAttribArray(vertexColourHandle));


            GLsizei numMatrices = 1;
            GLboolean transpose = GL_FALSE;
            // Projection Handle
            VINIVIA_CHECK_GL(
                    glUniformMatrix4fv(projectionHandle, numMatrices, transpose, m_projectionMatrix));

            // Model View Handle
            VINIVIA_CHECK_GL(
                    glUniformMatrix4fv(modelViewHandle, numMatrices, transpose, m_modelViewMatrix));
            VINIVIA_CHECK_GL(glDrawElements(GL_TRIANGLES, sizeof(ViniviaSDK::Shapes::Cube::CubeIndicies) / sizeof(GLushort),
                                            GL_UNSIGNED_SHORT, ViniviaSDK::Shapes::Cube::CubeIndicies));

            // Calculate cube rotation
            RotateCube();

            // Translate cube horizontal
            MoveCubeHorizontal();

            return reinterpret_cast<jlong>(nativeContext);
        }

    private:

        /**
         * Update perspective of viewport
         */
        void UpdatePerspective() {
            matrixIdentityFunction(m_modelViewMatrix);
            matrixRotateX(m_modelViewMatrix, m_angle);
            matrixRotateY(m_modelViewMatrix, m_angle);
            matrixTranslate(m_modelViewMatrix, m_horizontalOffset, 0.0f, -7.0f);
        }

        /**
         * Calculate Rotation angle of cube
         */
        void RotateCube() {
            m_angle += 1;
            if (m_angle > 360) {
                m_angle -= 360;
            }
        }

        /**
         * Calculate horizontal movement of Cube
         * @return
         */
        void MoveCubeHorizontal() {
            if (m_isMovingRight) {
                m_horizontalOffset += m_horizontalSpeed;
            } else {
                m_horizontalOffset -= m_horizontalSpeed;
            }

            if (m_horizontalOffset > 2.0f) {
                m_horizontalOffset = 2.0f;
                m_isMovingRight = false;
            } else if (m_horizontalOffset < -2.0f) {
                m_horizontalOffset = -2.0f;
                m_isMovingRight = true;
            }
        }

    private:
        float m_projectionMatrix[16];
        float m_modelViewMatrix[16];
        float m_angle = 0;
        float m_horizontalOffset = 0;
        float m_horizontalSpeed = 0.01f;
        bool m_isMovingRight = true;
    };

}

#endif //VINIVIA_SDK_CUBE_WITH_MASK_FILTER_H