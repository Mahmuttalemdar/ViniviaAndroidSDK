#include "Core/Entrance.h"
#include "Graphics/Graphics.h"
#include "Graphics/Renderer.h"
#include "Log/Log.h"

#include <jni.h>
#include <string>


extern "C"
JNIEXPORT jlong JNICALL
Java_com_vinivia_sdk_api_ViniviaSDK_initializeContext(JNIEnv *env, jobject thiz) {
    return ViniviaSDK::Entrance::getInstance()->getRenderer()->InitializeContext(env);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_vinivia_sdk_api_ViniviaSDK_destroyContext(JNIEnv *env, jobject thiz,
                                                   jlong native_context) {
    ViniviaSDK::Entrance::getInstance()->getRenderer()->DestroyContext(native_context);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_vinivia_sdk_api_ViniviaSDK_renderTexture(JNIEnv *env, jobject thiz, jlong native_context,
                                                  jlong timestamp_ns, jfloatArray vertex_transform,
                                                  jfloatArray texture_transform) {
    return ViniviaSDK::Entrance::getInstance()->getRenderer()->RenderTexture(env, native_context,
                                                                   vertex_transform,
                                                                   texture_transform);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_vinivia_sdk_api_ViniviaSDK_setWindowSurface(JNIEnv *env, jobject thiz,
                                                     jlong native_context, jobject jSurface) {
    return ViniviaSDK::Entrance::getInstance()->getRenderer()->SetWindowSurface(env, native_context,
                                                                      jSurface);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_vinivia_sdk_api_ViniviaSDK_getTexName(JNIEnv *env, jobject thiz, jlong native_context) {
    return ViniviaSDK::Entrance::getInstance()->getRenderer()->GetTextureID(native_context);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_vinivia_sdk_api_ViniviaSDK_changeActiveFilter(JNIEnv *env, jobject thiz, jobject filter) {
    jclass javaEnumClass = env->GetObjectClass(filter);
    jmethodID nameMethod = env->GetMethodID(javaEnumClass, "name", "()Ljava/lang/String;");
    jstring enumName = static_cast<jstring>(env->CallObjectMethod(filter, nameMethod));

    const char* enumNameStr = env->GetStringUTFChars(enumName, nullptr);

    ViniviaSDK::Filter nativeFilter = ViniviaSDK::Filter::NO_FILTER;

    if (std::string(enumNameStr) == "GRAYSCALE_FILTER") {
        nativeFilter = ViniviaSDK::Filter::GRAYSCALE_FILTER;
    }
    else if (std::string(enumNameStr) == "CUBE_FILTER") {
        nativeFilter = ViniviaSDK::Filter::CUBE_FILTER;
    }
    else if (std::string(enumNameStr) == "CUBE_WITH_DEPTH_MAP_FILTER") {
        nativeFilter = ViniviaSDK::Filter::CUBE_WITH_DEPTH_MAP_FILTER;
    }
    else if (std::string(enumNameStr) == "DEPTH_MAP_FILTER") {
        nativeFilter = ViniviaSDK::Filter::DEPTH_MAP_FILTER;
    }
    else if (std::string(enumNameStr) == "NEGATIVE_FILTER") {
        nativeFilter = ViniviaSDK::Filter::NEGATIVE_FILTER;
    }
    else {
        nativeFilter = ViniviaSDK::Filter::NO_FILTER;
    }

    env->ReleaseStringUTFChars(enumName, enumNameStr);

    ViniviaSDK::Entrance::getInstance()->getRenderer()->SetFilter(nativeFilter);
}