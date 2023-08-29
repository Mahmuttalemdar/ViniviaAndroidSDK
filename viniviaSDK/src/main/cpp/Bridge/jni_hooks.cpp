#include <jni.h>

/*
 * Load selected JNI version
 */
extern "C" {
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved) {
    return JNI_VERSION_1_6;
}
}