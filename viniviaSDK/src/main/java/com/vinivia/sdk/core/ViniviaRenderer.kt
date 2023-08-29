package com.vinivia.sdk.core

import android.graphics.SurfaceTexture
import android.opengl.Matrix
import android.os.Process
import android.util.Size
import android.view.Surface
import androidx.annotation.MainThread
import androidx.annotation.WorkerThread
import androidx.camera.core.Preview
import androidx.camera.core.SurfaceRequest
import androidx.concurrent.futures.CallbackToFutureAdapter
import androidx.core.util.Consumer
import androidx.core.util.Pair
import com.google.common.util.concurrent.ListenableFuture
import com.vinivia.sdk.api.ViniviaSDK
import java.util.Locale
import java.util.concurrent.Executor
import java.util.concurrent.RejectedExecutionException
import java.util.concurrent.atomic.AtomicInteger


class ViniviaRenderer {

    var mExecutor = SingleThreadHandlerExecutor(
        String.format(Locale.US, "GLRenderer-%03d", RENDERER_COUNT.incrementAndGet()),
        Process.THREAD_PRIORITY_DEFAULT
    ) // Use UI thread priority (DEFAULT)

    private var mNativeContext: Long = 0
    private var mPreviewResolution: Size? = null
    private var mPreviewTexture: SurfaceTexture? = null
    private val mPreviewTransform = FloatArray(16)
    private var mNaturalPreviewWidth = 0f
    private var mNaturalPreviewHeight = 0f
    private var mSurfaceSize: Size? = null
    private var mSurfaceRotationDegrees = 0
    private val mSurfaceTransform = FloatArray(16)
    private val mTempVec = FloatArray(8)
    private var mIsShutdown = false
    private var mNumOutstandingSurfaces = 0
    private var mFrameUpdateListener: Pair<Executor, Consumer<Long>>? = null


    @MainThread
    fun attachInputPreview(preview: Preview) {
        preview.setSurfaceProvider(
            mExecutor
        ) { surfaceRequest: SurfaceRequest ->
            if (mIsShutdown) {
                surfaceRequest.willNotProvideSurface()
                return@setSurfaceProvider
            }
            if (mNativeContext == 0L) {
                mNativeContext = ViniviaSDK.initializeContext()
            }
            val surfaceTexture = resetPreviewTexture(
                surfaceRequest.resolution
            )
            val inputSurface = Surface(surfaceTexture)
            mNumOutstandingSurfaces++
            surfaceRequest.provideSurface(
                inputSurface,
                mExecutor
            ) { result: SurfaceRequest.Result? ->
                inputSurface.release()
                surfaceTexture.release()
                if (surfaceTexture === mPreviewTexture) {
                    mPreviewTexture = null
                }
                mNumOutstandingSurfaces--
                doShutdownIfNeeded()
            }
        }
    }

    fun attachOutputSurface(
        surface: Surface, surfaceSize: Size, surfaceRotationDegrees: Int
    ) {
        try {
            mExecutor.execute {
                if (mIsShutdown) {
                    return@execute
                }
                if (mNativeContext == 0L) {
                    mNativeContext = ViniviaSDK.initializeContext()
                }
                if (ViniviaSDK.setWindowSurface(mNativeContext, surface)) {
                    mSurfaceRotationDegrees = surfaceRotationDegrees
                    mSurfaceSize = surfaceSize
                } else {
                    mSurfaceSize = null
                }
            }
        } catch (e: RejectedExecutionException) {
            // Renderer is shutting down. Ignore.
        }
    }

    /**
     * Sets a listener to receive updates when a frame has been drawn to the output [Surface].
     *
     *
     * Frame updates include the timestamp of the latest drawn frame.
     *
     * @param executor Executor used to call the listener.
     * @param listener Listener which receives updates in the form of a timestamp (in nanoseconds).
     */
    fun setFrameUpdateListener(executor: Executor, listener: Consumer<Long>) {
        try {
            mExecutor.execute {
                mFrameUpdateListener =
                    Pair(
                        executor,
                        listener
                    )
            }
        } catch (e: RejectedExecutionException) {
            // Renderer is shutting down. Ignore.
        }
    }

    fun invalidateSurface(surfaceRotationDegrees: Int) {
        try {
            mExecutor.execute {
                mSurfaceRotationDegrees = surfaceRotationDegrees
                if (mPreviewTexture != null && mNativeContext != 0L) {
                    renderLatest()
                }
            }
        } catch (e: RejectedExecutionException) {
            // Renderer is shutting down. Ignore.
        }
    }

    /**
     * Detach the current output surface from the renderer.
     *
     * @return A [ListenableFuture] that signals detach from the renderer. Some devices may
     * not be able to handle the surface being released while still attached to an EGL context.
     * It should be safe to release resources associated with the output surface once this future
     * has completed.
     */
    fun detachOutputSurface(): ListenableFuture<Void?> {
        return CallbackToFutureAdapter.getFuture { completer: CallbackToFutureAdapter.Completer<Void?> ->
            try {
                mExecutor.execute {
                    if (mNativeContext != 0L) {
                        ViniviaSDK.setWindowSurface(mNativeContext, null)
                        mSurfaceSize = null
                    }
                    completer.set(null)
                }
            } catch (e: RejectedExecutionException) {
                // Renderer is shutting down. Can notify that the surface is detached.
                completer.set(null)
            }
            "detachOutputSurface [$this]"
        }
    }

    fun shutdown() {
        try {
            mExecutor.execute {
                mIsShutdown = true
                if (mNativeContext != 0L) {
                    ViniviaSDK.destroyContext(mNativeContext)
                    mNativeContext = 0
                }
                doShutdownIfNeeded()
            }
        } catch (e: RejectedExecutionException) {
            // Renderer is shutting down. Can notify that the surface is detached.
        }
    }

    @WorkerThread
    private fun doShutdownIfNeeded() {
        if (mIsShutdown && mNumOutstandingSurfaces == 0) {
            mFrameUpdateListener = null
            mExecutor.shutdown()
        }
    }

    @WorkerThread
    private fun resetPreviewTexture(size: Size): SurfaceTexture {
        if (mPreviewTexture != null) {
            mPreviewTexture!!.detachFromGLContext()
        }
        mPreviewTexture = SurfaceTexture(ViniviaSDK.getTexName(mNativeContext))
        mPreviewTexture!!.setDefaultBufferSize(size.width, size.height)
        mPreviewTexture!!.setOnFrameAvailableListener(
            { surfaceTexture: SurfaceTexture ->
                if (surfaceTexture === mPreviewTexture && mNativeContext != 0L) {
                    surfaceTexture.updateTexImage()
                    renderLatest()
                }
            },
            mExecutor.handler
        )
        mPreviewResolution = size
        return mPreviewTexture as SurfaceTexture
    }

    @WorkerThread
    public fun renderLatest() {
        // Get the timestamp so we can pass it along to the output surface (not strictly necessary)
        val timestampNs = mPreviewTexture!!.timestamp

        // Get texture transform from surface texture (transform to natural orientation).
        // This will be used to transform texture coordinates in the fragment shader.
        mPreviewTexture!!.getTransformMatrix(mPreviewTransform)
        if (mSurfaceSize != null) {
            calculateSurfaceTransform()
            val success = ViniviaSDK.renderTexture(
                mNativeContext, timestampNs, mSurfaceTransform,
                mPreviewTransform
            )
            if (success && mFrameUpdateListener != null) {
                val executor = mFrameUpdateListener!!.first
                val listener = mFrameUpdateListener!!.second
                try {
                    executor.execute { listener.accept(timestampNs) }
                } catch (e: RejectedExecutionException) {
                    // Unable to send frame update. Ignore.
                }
            }
        }
    }

    /**
     * Calculates the dimensions of the source texture after it has been transformed from the raw
     * sensor texture to an image which is in the device's 'natural' orientation.
     *
     *
     * The required transform is passed along with each texture update and is retrieved from
     * [ ][SurfaceTexture.getTransformMatrix].
     *
     * <pre>`TEXTURE FROM SENSOR:
     * ^
     * |
     * |          .###########
     * |           ***********
     * |   ....############## ####. /           Sensor may be rotated relative
     * |  ################### #( )#.            to the device's 'natural'
     * |       ############## ######            orientation.
     * |  ################### #( )#*
     * |   ****############## ####* \
     * |           ...........
     * |          *###########
     * |
     * +-------------------------------->
     * TRANSFORMED IMAGE:
     * | |                   ^
     * | |                   |         .            .
     * | |                   |         \\ ........ //
     * Transform matrix from               |         ##############
     * SurfaceTexture#getTransformMatrix() |       ###(  )####(  )###
     * performs scale/crop/rotate on       |      ####################
     * image from sensor to produce        |     ######################
     * image in 'natural' orientation.     | ..  ......................  ..
     * | |                   |#### ###################### ####
     * | +-------\           |#### ###################### ####
     * +---------/           |#### ###################### ####
     * +-------------------------------->
    `</pre> *
     *
     *
     * The transform matrix is a 4x4 affine transform matrix that operates on standard normalized
     * texture coordinates which are in the range of [0,1] for both s and t dimensions. Once the
     * transform is applied, we scale by the width and height of the source texture.
     */
    @WorkerThread
    private fun calculateInputDimensions() {

        // Although the transform is normally used to rotate, it can also handle scale and
        // translation.
        // In order to accommodate for this, we use test vectors representing the boundaries of the
        // input, and run them through the transform to find the boundaries of the output.
        //
        //                                Top Bound (Vt):    Right Bound (Vr):
        //
        //                                ^ (0.5,1)             ^
        //                                |    ^                |
        //                                |    |                |
        //                                |    |                |        (1,0.5)
        //          Texture               |    +                |     +---->
        //          Coordinates:          |                     |
        //          ^                     |                     |
        //          |                     +----------->         +----------->
        //        (0,1)     (1,1)
        //          +---------+           Bottom Bound (Vb):     Left Bound (Vl):
        //          |         |
        //          |         |           ^                     ^
        //          |    +    |           |                     |
        //          |(0.5,0.5)|           |                     |
        //          |         |           |                  (0,0.5)
        //          +------------>        |    +                <----+
        //        (0,0)     (1,0)         |    |                |
        //                                |    |                |
        //                                +----v------>         +----------->
        //                                  (0.5,0)
        //
        // Using the above test vectors, we can calculate the transformed height using transform
        // matrix M as:
        //
        // Voh = |M x (Vt * h) - M x (Vb * h)| = |M x (Vt - Vb) * h| = |M x Vih| = |M x [0 h 0 0]|
        // where:
        // Vih = input, pre-transform height vector,
        // Voh = output transformed height vector,
        //   h = pre-transform texture height,
        //  || denotes element-wise absolute value,
        //   x denotes matrix-vector multiplication, and
        //   * denotes element-wise multiplication.
        //
        // Similarly, the transformed width will be calculated as:
        //
        // Vow = |M x (Vr * w) - M x (Vl * w)| = |M x (Vr - Vl) * w| = |M x Viw| = |M x [w 0 0 0]|
        // where:
        // Vow = output transformed width vector, and w = pre-transform texture width
        //
        // Since the transform matrix can potentially swap width and height, we must hold on to both
        // elements of each output vector. However, since we assume rotations in multiples of 90
        // degrees, and the vectors are orthogonal, we can calculate the final transformed vector
        // as:
        //
        // Vo = |M x Vih| + |M x Viw|

        // Initialize the components we care about for the output vector. This will be
        // accumulated from
        // Voh and Vow.
        mNaturalPreviewWidth = 0f
        mNaturalPreviewHeight = 0f

        // Calculate Voh. We use our allocated temporary vector to avoid excessive allocations since
        // this is done per-frame.
        val vih = mTempVec
        vih[0] = 0f
        vih[1] = mPreviewResolution!!.height.toFloat()
        vih[2] = 0f
        vih[3] = 0f

        // Apply the transform. Second half of the array is the result vector Voh.
        Matrix.multiplyMV( /*resultVec=*/
            mTempVec,  /*resultVecOffset=*/4,  /*lhsMat=*/
            mPreviewTransform,  /*lhsMatOffset=*/0,  /*rhsVec=*/
            vih,  /*rhsVecOffset=*/0
        )

        // Accumulate output from Voh.
        mNaturalPreviewWidth += Math.abs(mTempVec[4])
        mNaturalPreviewHeight += Math.abs(mTempVec[5])

        // Calculate Vow.
        val voh = mTempVec
        voh[0] = mPreviewResolution!!.width.toFloat()
        voh[1] = 0f
        voh[2] = 0f
        voh[3] = 0f

        // Apply the transform. Second half of the array is the result vector Vow.
        Matrix.multiplyMV( /*resultVec=*/
            mTempVec,  /*resultVecOffset=*/
            4,  /*lhsMat=*/
            mPreviewTransform,  /*lhsMatOffset=*/
            0,  /*rhsVec=*/
            voh,  /*rhsVecOffset=*/
            0
        )

        // Accumulate output from Vow. This now represents the fully transformed coordinates.
        mNaturalPreviewWidth += Math.abs(mTempVec[4])
        mNaturalPreviewHeight += Math.abs(mTempVec[5])
    }

    /**
     * Calculates the vertex shader transform matrix needed to transform the output from device
     * 'natural' orientation coordinates to a "center-crop" view of the camera viewport.
     *
     *
     * A device's 'natural' orientation is the orientation where the Display rotation is
     * Surface.ROTATION_0. For most phones, this will be a portrait orientation, whereas some
     * tablets
     * may use landscape as their natural orientation. The Surface rotation is always provided
     * relative to the device's 'natural' orientation.
     *
     *
     * Because the camera sensor (or crop of the camera sensor) may have a different aspect ratio
     * than the Surface that is meant to display it, we also want to fit the image from the
     * camera so
     * the entire Surface is filled. This generally requires scaling the input texture and cropping
     * pixels from either the width or height. We call this transform "center-crop" and is
     * equivalent
     * to the ScaleType with the same name in ImageView.
     */
    @WorkerThread
    private fun calculateSurfaceTransform() {
        // Calculate the dimensions of the source texture in the 'natural' orientation of the
        // device.
        calculateInputDimensions()

        // Transform surface width and height to natural orientation
        Matrix.setRotateM(mSurfaceTransform, 0, -mSurfaceRotationDegrees.toFloat(), 0f, 0f, 1.0f)

        // Since rotation is a linear transform, we don't need to worry about the affine component
        mTempVec[0] = mSurfaceSize!!.width.toFloat()
        mTempVec[1] = mSurfaceSize!!.height.toFloat()

        // Apply the transform to surface dimensions
        Matrix.multiplyMV(mTempVec, 4, mSurfaceTransform, 0, mTempVec, 0)
        val naturalSurfaceWidth = Math.abs(mTempVec[4])
        val naturalSurfaceHeight = Math.abs(mTempVec[5])

        // Now that both preview and surface are in the same coordinate system, calculate the ratio
        // of width/height between preview/surface to determine which dimension to scale
        val heightRatio = mNaturalPreviewHeight / naturalSurfaceHeight
        val widthRatio = mNaturalPreviewWidth / naturalSurfaceWidth

        // Now that we have calculated scale, we must apply rotation and scale in the correct order
        // such that it will apply to the vertex shader's vertices consistently.
        Matrix.setIdentityM(mSurfaceTransform, 0)

        // Apply the scale depending on whether the width or the height needs to be scaled to match
        // a "center crop" scale type. Because vertex coordinates are already normalized, we must
        // remove
        // the implicit scaling (through division) before scaling by the opposite dimension.
        if (mNaturalPreviewWidth * naturalSurfaceHeight
            > mNaturalPreviewHeight * naturalSurfaceWidth
        ) {
            Matrix.scaleM(mSurfaceTransform, 0, heightRatio / widthRatio, 1.0f, 1.0f)
        } else {
            Matrix.scaleM(mSurfaceTransform, 0, 1.0f, widthRatio / heightRatio, 1.0f)
        }

        // Finally add in rotation. This will be applied to vertices first.
        Matrix.rotateM(mSurfaceTransform, 0, -mSurfaceRotationDegrees.toFloat(), 0f, 0f, 1.0f)
    }

    companion object {
        private val RENDERER_COUNT = AtomicInteger(0)
    }
}
