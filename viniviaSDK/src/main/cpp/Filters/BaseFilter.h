#ifndef VINIVIA_SDK_BASE_FILTER_H
#define VINIVIA_SDK_BASE_FILTER_H

#include <jni.h>

namespace ViniviaSDK
{
    enum class Filter {
        NO_FILTER = 0,
        GRAYSCALE_FILTER,
        CUBE_FILTER,
        CUBE_WITH_DEPTH_MAP_FILTER,
        DEPTH_MAP_FILTER,
        NEGATIVE_FILTER
    };

    class BaseFilter {
    public:
        /**
         * C-tor
         */
        BaseFilter();

        /**
         * D-tor
         */
        virtual ~BaseFilter();

        /**
         * Apply filter to current opengl context
         * @param nativeContext
         * @param filter
         * @return
         */
        virtual jlong InitializeFilter(jlong context) = 0;

        /**
         * Render filter and bind texture to openGl
         * @param nativeContext
         * @return
         */
        virtual jlong RenderFilter(JNIEnv *env, jlong context, jfloatArray vertexTransformArray, jfloatArray textureTransformArray) = 0;
    };
}

#endif //VINIVIA_SDK_BASE_FILTER_H
