package com.vinivia.sdk.core

import android.graphics.SurfaceTexture
import android.util.Log
import android.util.Size
import android.view.Surface
import android.view.TextureView
import android.view.TextureView.SurfaceTextureListener
import android.view.ViewStub
import androidx.core.content.ContextCompat
import com.vinivia.sdk.R


/**
 * Utilities for instantiating a [TextureView] and attaching to an [ViniviaRenderer].
 */
object TextureViewRenderSurface {
    private const val TAG = "TextureViewRenderSurface"

    /**
     * Inflates a [TextureView] into the provided [ViewStub] and attaches it to the
     * provided [ViniviaRenderer].
     * @param viewStub Stub which will be replaced by TextureView.
     * @param renderer Renderer which will be used to update the TextureView.
     * @return The inflated TextureView.
     */
    fun inflateWith(
        viewStub: ViewStub,
        renderer: ViniviaRenderer
    ): TextureView {
        Log.d(TAG, "Inflating TextureView into view stub.")
        viewStub.layoutResource = R.layout.texture_view_render_surface
        val textureView = viewStub.inflate() as TextureView
        textureView.surfaceTextureListener = object : SurfaceTextureListener {
            private var mSurface: Surface? = null
            override fun onSurfaceTextureAvailable(st: SurfaceTexture, width: Int, height: Int) {
                mSurface = Surface(st)
                renderer.attachOutputSurface(
                    mSurface!!, Size(width, height),
                    Surfaces.toSurfaceRotationDegrees(textureView.display.rotation)
                )
            }

            override fun onSurfaceTextureSizeChanged(st: SurfaceTexture, width: Int, height: Int) {
                mSurface?.let {
                    renderer.attachOutputSurface(
                        it, Size(width, height),
                        Surfaces.toSurfaceRotationDegrees(textureView.display.rotation)
                    )
                }
            }

            override fun onSurfaceTextureDestroyed(st: SurfaceTexture): Boolean {
                val surface = mSurface
                mSurface = null
                renderer.detachOutputSurface().addListener({
                    surface!!.release()
                    st.release()
                }, ContextCompat.getMainExecutor(textureView.context))
                return false
            }

            override fun onSurfaceTextureUpdated(st: SurfaceTexture) {}
        }
        return textureView
    }
}