package com.vinivia.sdk.activity

import android.Manifest
import android.content.pm.PackageManager
import android.hardware.display.DisplayManager
import android.hardware.display.DisplayManager.DisplayListener
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.view.View
import android.view.ViewStub
import android.view.WindowManager
import android.widget.TextView
import android.widget.Toast
import androidx.activity.result.ActivityResultCallback
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.camera.core.AspectRatio
import androidx.camera.core.CameraSelector
import androidx.camera.core.Preview
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.constraintlayout.widget.ConstraintLayout
import androidx.core.content.ContextCompat
import androidx.lifecycle.ViewModelProvider
import com.google.android.material.floatingactionbutton.FloatingActionButton
import com.vinivia.sdk.R
import com.vinivia.sdk.api.ViniviaSDK
import com.vinivia.sdk.core.SurfaceViewRenderSurface
import com.vinivia.sdk.core.Surfaces
import com.vinivia.sdk.core.TextureViewRenderSurface
import com.vinivia.sdk.core.ViniviaRenderer
import com.vinivia.sdk.models.CameraXViewModel
import com.vinivia.sdk.utils.FpsRecorder
import java.util.Locale
import java.util.Objects


/** Activity which runs the camera preview with opengl processing  */
class ViniviaMainActivity : AppCompatActivity() {
    private var mRenderer: ViniviaRenderer? = null
    private var mDisplayListener: DisplayListener? = null
    private var mCameraProvider: ProcessCameraProvider? = null

    public override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.vinivia_main_activity)

        // Setup Vinivia Renderer
        setupRenderer()

        // Add a frame update listener to display FPS
        setupFPSRecorder()

        // Set up UI elements
        setupActionButtons()

        val viewModel: CameraXViewModel = ViewModelProvider(this)[CameraXViewModel::class.java]
        viewModel
            .cameraProvider
            .observe(
                this
            ) { cameraProviderResult ->
                if (cameraProviderResult.hasProvider()) {
                    mCameraProvider = cameraProviderResult.provider
                    if (allPermissionsGranted()) {
                        startCamera()
                    }
                } else {
                    Log.e(
                        TAG, "Failed to retrieve ProcessCameraProvider",
                        cameraProviderResult.error
                    )
                    Toast.makeText(
                        applicationContext, "Unable to initialize CameraX. See logs "
                                + "for details.", Toast.LENGTH_LONG
                    ).show()
                }
            }
        if (!allPermissionsGranted()) {
            mRequestPermissions.launch(REQUIRED_PERMISSIONS)
        }
    }

    public override fun onDestroy() {
        super.onDestroy()
        val dpyMgr = Objects.requireNonNull(
            getSystemService(DISPLAY_SERVICE) as DisplayManager
        )
        dpyMgr.unregisterDisplayListener(mDisplayListener)
        mRenderer?.shutdown()
    }

    private fun restartRenderer(newFilter: ViniviaSDK.Filter) {
        mCameraProvider?.unbindAll()

        // Shutdown the current renderer
        mRenderer?.shutdown()

        // Change the active filter
        ViniviaSDK.changeActiveFilter(newFilter)

        // Create a new renderer with the specified filter
        mRenderer = ViniviaRenderer()

        // Get current preview
        val preview = Preview.Builder().setTargetAspectRatio(AspectRatio.RATIO_16_9).build()
        mRenderer?.attachInputPreview(preview)

        // Create new dynamic view stub for renderer
        createNewViewStub()
        val viewFinderStub = findViewById<ViewStub>(R.id.viewFinderStub)
        mRenderer?.let {
            chooseViewFinder(
                intent.extras, viewFinderStub,
                it
            )
        }

        // Reattach the new renderer to the camera provider
        val cameraSelector = CameraSelector.DEFAULT_BACK_CAMERA
        mCameraProvider?.bindToLifecycle(this, cameraSelector, preview)
    }

    private fun setupRenderer() {
        ViniviaSDK.changeActiveFilter(ViniviaSDK.Filter.NO_FILTER)
        mRenderer = ViniviaRenderer()

        val viewFinderStub = findViewById<ViewStub>(R.id.viewFinderStub)
        val viewFinder = mRenderer?.let {
            chooseViewFinder(
                intent.extras, viewFinderStub,
                it
            )
        }

        // A display listener is needed when the phone rotates 180 degrees without stopping at a
        // 90 degree increment. In these cases, onCreate() isn't triggered, so we need to ensure
        // the output surface uses the correct orientation.
        mDisplayListener = object : DisplayListener {
            override fun onDisplayAdded(displayId: Int) {}
            override fun onDisplayRemoved(displayId: Int) {}
            override fun onDisplayChanged(displayId: Int) {
                val viewFinderDisplay = viewFinder?.display
                if (viewFinderDisplay != null
                    && viewFinderDisplay.displayId == displayId
                ) {
                    mRenderer!!.invalidateSurface(
                        Surfaces.toSurfaceRotationDegrees(
                            viewFinderDisplay.rotation
                        )
                    )
                }
            }
        }
        val dpyMgr = Objects.requireNonNull(getSystemService(DISPLAY_SERVICE) as DisplayManager)
        dpyMgr.registerDisplayListener(mDisplayListener, Handler(Looper.getMainLooper()))
    }

    private fun setupFPSRecorder() {
        val fpsRecorder = FpsRecorder(FPS_NUM_SAMPLES)
        val fpsCounterView = findViewById<TextView>(R.id.fps_counter)
        mRenderer!!.setFrameUpdateListener(ContextCompat.getMainExecutor(this)) { timestamp ->
            val fps: Double = fpsRecorder.recordTimestamp(timestamp)
            fpsCounterView.text = getString(
                R.string.fps_counter_template,
                if ((java.lang.Double.isNaN(fps) || java.lang.Double.isInfinite(fps))) "---" else String.format(
                    Locale.US,
                    "%.0f", fps
                )
            )
        }
    }

    private fun setupActionButtons() {
        var isAllFabsVisible: Boolean?

        // Get action buttons
        var fab_main_button: FloatingActionButton? = findViewById<FloatingActionButton>(R.id.fab_main_button)
        var fab_no_filter_button: FloatingActionButton? = findViewById<FloatingActionButton>(R.id.fab_no_filter_button)
        var fab_grayscale_filter_button: FloatingActionButton? = findViewById<FloatingActionButton>(R.id.fab_grayscale_filter_button)
        var fab_cube_filter_button: FloatingActionButton? = findViewById<FloatingActionButton>(R.id.fab_cube_filter_button)
        var fab_cube_with_depth_map_filter_button: FloatingActionButton? = findViewById<FloatingActionButton>(R.id.fab_cube_with_depth_map_filter_button)
        var fab_depth_map_filter_button: FloatingActionButton? = findViewById<FloatingActionButton>(R.id.fab_depth_map_filter_button)
        var fab_negative_filter_button: FloatingActionButton? = findViewById<FloatingActionButton>(R.id.fab_negative_filter_button)

        // Get action texts
        var fab_no_filter_action_text: TextView? = findViewById<TextView>(R.id.fab_no_filter_action_text)
        var fab_grayscale_filter_action_text: TextView? = findViewById<TextView>(R.id.fab_grayscale_filter_action_text)
        var fab_cube_filter_action_text: TextView? = findViewById<TextView>(R.id.fab_cube_filter_action_text)
        var fab_cube_with_depth_map_filter_action_text: TextView? = findViewById<TextView>(R.id.fab_cube_with_depth_map_filter_action_text)
        var fab_depth_map_filter_action_text: TextView? = findViewById<TextView>(R.id.fab_depth_map_filter_action_text)
        var fab_negative_filter_action_text: TextView? = findViewById<TextView>(R.id.fab_negative_filter_action_text)

        // Set invisible all elements
        fab_no_filter_button!!.visibility = View.GONE
        fab_grayscale_filter_button!!.visibility = View.GONE
        fab_cube_filter_button!!.visibility = View.GONE
        fab_cube_with_depth_map_filter_button!!.visibility = View.GONE
        fab_depth_map_filter_button!!.visibility = View.GONE
        fab_negative_filter_button!!.visibility = View.GONE

        fab_no_filter_action_text!!.visibility = View.GONE
        fab_grayscale_filter_action_text!!.visibility = View.GONE
        fab_cube_filter_action_text!!.visibility = View.GONE
        fab_cube_with_depth_map_filter_action_text!!.visibility = View.GONE
        fab_depth_map_filter_action_text!!.visibility = View.GONE
        fab_negative_filter_action_text!!.visibility = View.GONE

        // Action name texts and all the sub FABs are invisible
        isAllFabsVisible = false

        fun toggle() {
            isAllFabsVisible = if (!isAllFabsVisible!!) {
                fab_no_filter_button.show()
                fab_grayscale_filter_button.show()
                fab_cube_filter_button.show()
                fab_cube_with_depth_map_filter_button.show()
                fab_depth_map_filter_button.show()
                fab_negative_filter_button.show()

                fab_no_filter_action_text.visibility = View.VISIBLE
                fab_grayscale_filter_action_text.visibility = View.VISIBLE
                fab_cube_filter_action_text.visibility = View.VISIBLE
                fab_cube_with_depth_map_filter_action_text.visibility = View.VISIBLE
                fab_depth_map_filter_action_text.visibility = View.VISIBLE
                fab_negative_filter_action_text.visibility = View.VISIBLE

                true
            } else {
                fab_no_filter_button.hide()
                fab_grayscale_filter_button.hide()
                fab_cube_filter_button.hide()
                fab_cube_with_depth_map_filter_button.hide()
                fab_depth_map_filter_button.hide()
                fab_negative_filter_button.hide()


                fab_no_filter_action_text.visibility = View.GONE
                fab_grayscale_filter_action_text.visibility = View.GONE
                fab_cube_filter_action_text.visibility = View.GONE
                fab_cube_with_depth_map_filter_action_text.visibility = View.GONE
                fab_depth_map_filter_action_text.visibility = View.GONE
                fab_negative_filter_action_text.visibility = View.GONE
                false
            }
        }


        fab_main_button!!.setOnClickListener(View.OnClickListener {
            toggle()
        })


        fab_no_filter_button.setOnClickListener(
            View.OnClickListener {
                Toast.makeText(
                    this@ViniviaMainActivity, "Filter changed to No Filter", Toast.LENGTH_SHORT
                ).show()
                restartRenderer(ViniviaSDK.Filter.NO_FILTER)
                toggle()
            })

        fab_grayscale_filter_button.setOnClickListener(
            View.OnClickListener {
                Toast.makeText(
                    this@ViniviaMainActivity, "Filter changed to Grayscale Filter", Toast.LENGTH_SHORT
                ).show()
                restartRenderer(ViniviaSDK.Filter.GRAYSCALE_FILTER)
                toggle()
            })

        fab_cube_filter_button.setOnClickListener(
            View.OnClickListener {
                Toast.makeText(
                    this@ViniviaMainActivity, "Filter changed to Cube Filter", Toast.LENGTH_SHORT
                ).show()
                restartRenderer(ViniviaSDK.Filter.CUBE_FILTER)
                toggle()
            })

        fab_cube_with_depth_map_filter_button.setOnClickListener(
            View.OnClickListener {
                Toast.makeText(
                    this@ViniviaMainActivity, "Filter changed to Cube With Depth Map Filter", Toast.LENGTH_SHORT
                ).show()
                restartRenderer(ViniviaSDK.Filter.CUBE_WITH_DEPTH_MAP_FILTER)
                toggle()
            })

        fab_depth_map_filter_button.setOnClickListener(
            View.OnClickListener {
                Toast.makeText(
                    this@ViniviaMainActivity, "Filter changed to Depth Map Filter", Toast.LENGTH_SHORT
                ).show()
                restartRenderer(ViniviaSDK.Filter.DEPTH_MAP_FILTER)
                toggle()
            })

        fab_negative_filter_button.setOnClickListener(
            View.OnClickListener {
                Toast.makeText(
                    this@ViniviaMainActivity, "Filter changed to Negative Filter", Toast.LENGTH_SHORT
                ).show()
                restartRenderer(ViniviaSDK.Filter.NEGATIVE_FILTER)
                toggle()
            })
    }

    private fun startCamera() {
        // Keep screen on for this app. This is just for convenience, and is not required.
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        // Set the aspect ratio of Preview to match the aspect ratio of the view finder (defined
        // with ConstraintLayout).
        val preview = Preview.Builder().setTargetAspectRatio(AspectRatio.RATIO_16_9).build()
        mRenderer?.attachInputPreview(preview)
        val cameraSelector = CameraSelector.DEFAULT_BACK_CAMERA
        mCameraProvider!!.bindToLifecycle(this, cameraSelector, preview)
    }

    private fun createNewViewStub() {
        val viewStub = ViewStub(this)
        viewStub.id = R.id.viewFinderStub // ViewStub'a benzersiz bir ID atayın

        // Gerekli özellikleri ayarlayın
        val layoutParams = ConstraintLayout.LayoutParams(
            ConstraintLayout.LayoutParams.MATCH_PARENT,
            ConstraintLayout.LayoutParams.MATCH_PARENT
        )
        layoutParams.dimensionRatio = "H,9:16"
        layoutParams.startToStart = ConstraintLayout.LayoutParams.PARENT_ID
        layoutParams.endToEnd = ConstraintLayout.LayoutParams.PARENT_ID
        layoutParams.topToTop = ConstraintLayout.LayoutParams.PARENT_ID
        layoutParams.bottomToBottom = ConstraintLayout.LayoutParams.PARENT_ID

        viewStub.layoutParams = layoutParams

        // ViewStub'ı ekrana ekleyin
        val constraintLayout = findViewById<ConstraintLayout>(R.id.mainLayout)
        constraintLayout.addView(viewStub)
    }

    // **************************** Permission handling code start *******************************//
    private val mRequestPermissions =
        registerForActivityResult<Array<String>, Map<String, Boolean>>(
            ActivityResultContracts.RequestMultiplePermissions(),
            ActivityResultCallback<Map<String, Boolean>> { result ->
                for (permission: String? in REQUIRED_PERMISSIONS) {

                    val res : Boolean? = Objects.requireNonNull(result[permission])
                    res.let {
                        if (!(it)!!) {
                            Toast.makeText(
                                this@ViniviaMainActivity, "Permissions not granted",
                                Toast.LENGTH_SHORT
                            ).show()
                            finish()
                        }
                    }
                }

                // All permissions granted.
                if (mCameraProvider != null) {
                    startCamera()
                }
            })

    private fun allPermissionsGranted(): Boolean {
        for (permission in REQUIRED_PERMISSIONS) {
            if (ContextCompat.checkSelfPermission(this, permission)
                != PackageManager.PERMISSION_GRANTED
            ) {
                return false
            }
        }
        return true
    } // **************************** Permission handling code end *********************************//

    companion object {
        private const val TAG = "OpenGLActivity"

        /**
         * Intent Extra string for choosing which type of render surface to use to display Preview.
         */
        const val INTENT_EXTRA_RENDER_SURFACE_TYPE = "render_surface_type"

        /**
         * TextureView render surface for [ViniviaMainActivity.INTENT_EXTRA_RENDER_SURFACE_TYPE].
         * This is the default render surface.
         */
        const val RENDER_SURFACE_TYPE_TEXTUREVIEW = "textureview"

        /**
         * SurfaceView render surface for [ViniviaMainActivity.INTENT_EXTRA_RENDER_SURFACE_TYPE].
         * This type will block the main thread while detaching it's [Surface] from the OpenGL
         * renderer to avoid compatibility issues on some devices.
         */
        const val RENDER_SURFACE_TYPE_SURFACEVIEW = "surfaceview"

        /**
         * SurfaceView render surface (in non-blocking mode) for
         * [ViniviaMainActivity.INTENT_EXTRA_RENDER_SURFACE_TYPE]. This type will NOT
         * block the main thread while detaching it's [Surface] from the OpenGL
         * renderer, but some devices may crash due to their OpenGL/EGL implementation not being
         * thread-safe.
         */
        const val RENDER_SURFACE_TYPE_SURFACEVIEW_NONBLOCKING = "surfaceview_nonblocking"
        private val REQUIRED_PERMISSIONS = arrayOf(
            Manifest.permission.CAMERA
        )
        private const val FPS_NUM_SAMPLES = 30

        /**
         * Chooses the type of view to use for the viewfinder based on intent extras.
         *
         * @param intentExtras   Optional extras which can contain an extra with key
         * [.INTENT_EXTRA_RENDER_SURFACE_TYPE]. Possible values are one of
         * [.RENDER_SURFACE_TYPE_TEXTUREVIEW],
         * [.RENDER_SURFACE_TYPE_SURFACEVIEW], or
         * [.RENDER_SURFACE_TYPE_SURFACEVIEW_NONBLOCKING]. If `null`,
         * or the bundle does not contain a surface type, then
         * [.RENDER_SURFACE_TYPE_TEXTUREVIEW] will be used.
         * @param viewFinderStub The stub to inflate the chosen viewfinder into.
         * @param renderer       The [ViniviaRenderer] which will render frames into the
         * viewfinder.
         * @return The inflated viewfinder View.
         */
        fun chooseViewFinder(
            intentExtras: Bundle?,
            viewFinderStub: ViewStub,
            renderer: ViniviaRenderer
        ): View {
            // By default we choose TextureView to maximize compatibility.
            var renderSurfaceType: String? = RENDER_SURFACE_TYPE_TEXTUREVIEW
            if (intentExtras != null) {
                renderSurfaceType = intentExtras.getString(
                    INTENT_EXTRA_RENDER_SURFACE_TYPE,
                    RENDER_SURFACE_TYPE_TEXTUREVIEW
                )
            }
            return when (renderSurfaceType) {
                RENDER_SURFACE_TYPE_TEXTUREVIEW -> {
                    Log.d(TAG, "Using TextureView render surface.")
                    TextureViewRenderSurface.inflateWith(viewFinderStub, renderer)
                }

                RENDER_SURFACE_TYPE_SURFACEVIEW -> {
                    Log.d(TAG, "Using SurfaceView render surface.")
                    SurfaceViewRenderSurface.inflateWith(viewFinderStub, renderer)
                }

                RENDER_SURFACE_TYPE_SURFACEVIEW_NONBLOCKING -> {
                    Log.d(TAG, "Using SurfaceView (non-blocking) render surface.")
                    SurfaceViewRenderSurface.inflateNonBlockingWith(viewFinderStub, renderer)
                }

                else -> throw IllegalArgumentException(
                    String.format(
                        Locale.US,
                        "Unknown render "
                                + "surface type: %s. Supported surface types include: [%s, %s, %s]",
                        renderSurfaceType, RENDER_SURFACE_TYPE_TEXTUREVIEW,
                        RENDER_SURFACE_TYPE_SURFACEVIEW,
                        RENDER_SURFACE_TYPE_SURFACEVIEW_NONBLOCKING
                    )
                )
            }
        }
    }
}
