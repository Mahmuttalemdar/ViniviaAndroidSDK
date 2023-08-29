package com.vinivia.sdk.api

import android.view.Surface
import androidx.annotation.WorkerThread

// Wrapper for native library
object ViniviaSDK {
    init {
        System.loadLibrary("ViniviaSDK")
    }

    enum class Filter {
        NO_FILTER,
        GRAYSCALE_FILTER,
        CUBE_FILTER,
        CUBE_WITH_DEPTH_MAP_FILTER,
        DEPTH_MAP_FILTER,
        NEGATIVE_FILTER
    }

    @WorkerThread
    external fun initializeContext() : Long

    @WorkerThread
    external fun destroyContext(nativeContext: Long)

    @WorkerThread
    external fun renderTexture(
        nativeContext: Long,
        timestampNs: Long,
        vertexTransform: FloatArray,
        textureTransform: FloatArray
    ): Boolean

    @WorkerThread
    external fun setWindowSurface(nativeContext: Long, surface: Surface?): Boolean

    @WorkerThread
    external fun getTexName(nativeContext: Long): Int

    @WorkerThread
    external fun changeActiveFilter(filter: Filter)
}