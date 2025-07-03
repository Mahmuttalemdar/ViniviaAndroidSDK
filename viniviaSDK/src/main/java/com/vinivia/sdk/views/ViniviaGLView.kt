package com.vinivia.sdk.views

import android.content.Context
import android.opengl.GLSurfaceView
import android.util.AttributeSet

class ViniviaGLView : GLSurfaceView {
    
    constructor(context: Context) : super(context) {
        initializeView()
    }
    
    constructor(context: Context, attrs: AttributeSet) : super(context, attrs) {
        initializeView()
    }
    
    private fun initializeView() {
        // Pick an EGLConfig with RGB8 color, 16-bit depth, no stencil,
        // supporting OpenGL ES 2.0 or later backwards-compatible versions.
        setEGLConfigChooser(8, 8, 8, 0, 16, 0)
        setEGLContextClientVersion(3)
        
        // Set render mode after the view is properly initialized
        post {
            renderMode = GLSurfaceView.RENDERMODE_WHEN_DIRTY
        }
    }
    
    companion object {
        private const val TAG = "ViniviaSDK"
        private const val DEBUG = true
    }
}