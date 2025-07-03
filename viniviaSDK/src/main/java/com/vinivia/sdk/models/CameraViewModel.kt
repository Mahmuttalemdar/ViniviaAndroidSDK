package com.vinivia.sdk.models

import android.app.Application
import androidx.annotation.MainThread
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.core.content.ContextCompat
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import java.util.Objects
import java.util.concurrent.CancellationException
import java.util.concurrent.ExecutionException

/**
 * This class providing access to the camera by View model
 */
class CameraXViewModel(application: Application) : AndroidViewModel(application) {
    private var mProcessCameraProviderLiveData: MutableLiveData<CameraProviderResult>? = null

    @get:MainThread
    val cameraProvider: LiveData<CameraProviderResult>
        /**
         * Returns a [LiveData] containing CameraX's [ProcessCameraProvider] once it has
         * been initialized.
         */
        get() {
            if (mProcessCameraProviderLiveData == null) {
                mProcessCameraProviderLiveData = MutableLiveData()
                //tryConfigureCameraProvider();
                try {
                    val cameraProviderFuture = ProcessCameraProvider.getInstance(getApplication())
                    cameraProviderFuture.addListener({
                        try {
                            val cameraProvider = cameraProviderFuture.get()
                            mProcessCameraProviderLiveData!!.setValue(
                                CameraProviderResult.fromProvider(
                                    cameraProvider
                                )
                            )
                        } catch (e: ExecutionException) {
                            if (e.cause !is CancellationException) {
                                mProcessCameraProviderLiveData!!.value =
                                    CameraProviderResult.fromError(
                                        e.cause ?: e
                                    )
                            }
                        } catch (e: InterruptedException) {
                            throw AssertionError("Unexpected thread interrupt.", e)
                        }
                    }, ContextCompat.getMainExecutor(getApplication()))
                } catch (e: IllegalStateException) {
                    // Failure during ProcessCameraProvider.getInstance()
                    mProcessCameraProviderLiveData!!.setValue(CameraProviderResult.fromError(e))
                }
            }
            return mProcessCameraProviderLiveData!!
        }

    /**
     * Class for wrapping success/error of initializing the [ProcessCameraProvider].
     */
    class CameraProviderResult private constructor(
        /**
         * Returns a [ProcessCameraProvider] if the result does not contain an error,
         * otherwise returns `null`.
         *
         *
         * Use [.hasProvider] to check if this result contains a provider.
         */
        val provider: ProcessCameraProvider?,
        /**
         * Returns a [Throwable] containing the error that prevented the
         * [ProcessCameraProvider] from being available. Returns `null` if no error
         * occurred.
         *
         *
         * Use [.hasProvider] to check if this result contains a provider.
         */
        var error: Throwable?
    ) {

        init {
            error = error
        }

        /**
         * Returns `true` if this result contains a [ProcessCameraProvider]. Returns
         * `false` if it contains an error.
         */
        fun hasProvider(): Boolean {
            return provider != null
        }

        companion object {
            fun fromProvider(provider: ProcessCameraProvider): CameraProviderResult {
                return CameraProviderResult(provider,  /*error=*/null)
            }

            fun fromError(error: Throwable): CameraProviderResult {
                return CameraProviderResult( /*provider=*/null, error)
            }
        }
    }

    companion object {
        private const val TAG = "CameraXViewModel"
        private const val sIsCameraProviderConfigured = false
    }
}
