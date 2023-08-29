#ifndef VINIVIA_SDK_CUBE_H
#define VINIVIA_SDK_CUBE_H


namespace ViniviaSDK
{
    namespace Shapes::Cube
    {
        /**
         * Keeping Cube Indices
         */
        static constexpr GLushort
                CubeIndicies[] = {
                0, 2, 3, 0, 1, 3, 4, 6, 7, 4, 5, 7, 8, 9, 10,
                11, 8, 10, 12, 13, 14, 15, 12, 14, 16, 17, 18,
                16, 19, 18, 20, 21, 22, 20, 23, 22
        };

        /**
         * Vertex data of Cube
         */
        static constexpr GLfloat
                CubeVertices[] = {
                -0.2f, 0.2f, -0.2f, /* Back. */
                0.2f, 0.2f, -0.2f,
                -0.2f, -0.2f, -0.2f,
                0.2f, -0.2f, -0.2f,
                -0.2f, 0.2f, 0.2f, /* Front. */
                0.2f, 0.2f, 0.2f,
                -0.2f, -0.2f, 0.2f,
                0.2f, -0.2f, 0.2f,
                -0.2f, 0.2f, -0.2f, /* Left. */
                -0.2f, -0.2f, -0.2f,
                -0.2f, -0.2f, 0.2f,
                -0.2f, 0.2f, 0.2f,
                0.2f, 0.2f, -0.2f, /* Right. */
                0.2f, -0.2f, -0.2f,
                0.2f, -0.2f, 0.2f,
                0.2f, 0.2f, 0.2f,
                -0.2f, 0.2f, -0.2f, /* Top. */
                -0.2f, 0.2f, 0.2f,
                0.2f, 0.2f, 0.2f,
                0.2f, 0.2f, -0.2f,
                -0.2f, -0.2f, -0.2f, /* Bottom. */
                -0.2f, -0.2f, 0.2f,
                0.2f, -0.2f, 0.2f,
                0.2f, -0.2f, -0.2f
        };

        /**
         * Vertex Colour Data of Cube
         */
        static constexpr GLfloat
                CubeColour[] = {
                1.0f, 0.0f, 0.0f,
                1.0f, 0.0f, 0.0f,
                1.0f, 0.0f, 0.0f,
                1.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f,
                0.0f, 1.0f, 0.0f,
                0.0f, 1.0f, 0.0f,
                0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 1.0f,
                0.0f, 0.0f, 1.0f,
                0.0f, 0.0f, 1.0f,
                0.0f, 0.0f, 1.0f,
                1.0f, 1.0f, 0.0f,
                1.0f, 1.0f, 0.0f,
                1.0f, 1.0f, 0.0f,
                1.0f, 1.0f, 0.0f,
                0.0f, 1.0f, 1.0f,
                0.0f, 1.0f, 1.0f,
                0.0f, 1.0f, 1.0f,
                0.0f, 1.0f, 1.0f,
                1.0f, 0.0f, 1.0f,
                1.0f, 0.0f, 1.0f,
                1.0f, 0.0f, 1.0f,
                1.0f, 0.0f, 1.0f
        };
    }
}

#endif //VINIVIA_SDK_CUBE_H