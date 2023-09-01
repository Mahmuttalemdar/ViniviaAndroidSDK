package com.vinivia.sdk.api

import android.graphics.Bitmap
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
        DEPTH_MAP_FILTER,
        NEGATIVE_FILTER,
        POSITIVE_MASK_FILTER,
        NEGATIVE_MASK_FILTER,
        CUBE_OVER_MASK_FILTER
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

    @WorkerThread
    external fun setMask(nativeContext: Long, mask: Bitmap)
}