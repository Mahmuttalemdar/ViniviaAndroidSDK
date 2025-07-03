package com.vinivia.compose.demo.ui

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyRow
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.viewinterop.AndroidView
import androidx.lifecycle.viewmodel.compose.viewModel
import androidx.compose.ui.platform.LocalLifecycleOwner
import androidx.camera.core.CameraSelector
import androidx.camera.core.Preview
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.camera.view.PreviewView
import com.vinivia.sdk.api.ViniviaSDK
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import androidx.compose.ui.semantics.contentDescription
import androidx.compose.ui.semantics.semantics

/**
 * Main camera screen composable
 * 
 * This composable demonstrates:
 * 1. Modern Jetpack Compose UI patterns with state management
 * 2. Integration patterns for legacy Android Views using AndroidView
 * 3. Responsive UI with loading states and animations
 * 4. Material 3 design system implementation
 * 5. MVVM architecture with reactive state management
 */
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun CameraScreen(
    viewModel: CameraViewModel = viewModel()
) {
    val uiState by viewModel.uiState.collectAsState()
    // val context = LocalContext.current // not currently used
    
    // Handle error messages
    LaunchedEffect(uiState.errorMessage) {
        uiState.errorMessage?.let { _ ->
            // In a real app, you might want to show a Snackbar or dialog here
            viewModel.clearError()
        }
    }
    
    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp)
    ) {
        // Header
        Text(
            text = "Vinivia Camera Filters",
            style = MaterialTheme.typography.headlineMedium,
            fontWeight = FontWeight.Bold,
            modifier = Modifier.padding(bottom = 16.dp)
        )
        
        // Camera preview section
        Card(
            modifier = Modifier
                .fillMaxWidth()
                .weight(1f)
                .padding(bottom = 16.dp),
            elevation = CardDefaults.cardElevation(defaultElevation = 8.dp)
        ) {
            Box(
                modifier = Modifier.fillMaxSize(),
                contentAlignment = Alignment.Center
            ) {
                if (uiState.isLoading) {
                    LoadingIndicator()
                } else {
                    // Camera preview with filter overlay
                    CameraPreview(
                        selectedFilter = uiState.selectedFilter
                    )
                    
                    // Show loading overlay when applying filter
                    if (uiState.isApplyingFilter) {
                        Surface(
                            modifier = Modifier.fillMaxSize(),
                            color = MaterialTheme.colorScheme.surface.copy(alpha = 0.8f)
                        ) {
                            Box(
                                contentAlignment = Alignment.Center
                            ) {
                                Column(
                                    horizontalAlignment = Alignment.CenterHorizontally
                                ) {
                                    CircularProgressIndicator()
                                    Spacer(modifier = Modifier.height(16.dp))
                                    Text(
                                        text = "Applying ${uiState.selectedFilter?.displayName ?: "Filter"}...",
                                        style = MaterialTheme.typography.bodyMedium
                                    )
                                }
                            }
                        }
                    }
                }
            }
        }
        
        // Filter selection section
        FilterSelectionSection(
            uiState = uiState,
            onFilterSelected = viewModel::onFilterSelected
        )
    }
}

/**
 * Displays the real-time camera preview using CameraX and overlays a translucent
 * color to visually represent the currently selected [ViniviaSDK.Filter].
 *
 * @param selectedFilter The filter chosen by the user; used only for overlay color.
 */
@Composable
private fun CameraPreview(
    selectedFilter: FilterItem?
) {
    val context = LocalContext.current
    val lifecycleOwner = LocalLifecycleOwner.current
    val previewView = remember { PreviewView(context) }

    // Camera setup executed once when this composable enters composition
    LaunchedEffect(Unit) {
        // `get()` is a blocking call – move to IO context to avoid main-thread stall
        val cameraProvider = withContext(Dispatchers.IO) {
            ProcessCameraProvider.getInstance(context).get()
        }
        val preview = Preview.Builder().build().apply {
            setSurfaceProvider(previewView.surfaceProvider)
        }
        cameraProvider.unbindAll()
        cameraProvider.bindToLifecycle(
            lifecycleOwner,
            CameraSelector.DEFAULT_BACK_CAMERA,
            preview
        )
    }

    Box(
        modifier = Modifier.fillMaxSize(),
        contentAlignment = Alignment.Center
    ) {
        AndroidView(
            factory = { previewView },
            modifier = Modifier
                .fillMaxSize()
                .semantics { contentDescription = "Camera preview" }
        )

        // Overlay color to simulate filter effect
        val overlayColor = when (selectedFilter?.filter) {
            ViniviaSDK.Filter.GRAYSCALE_FILTER -> Color.Gray.copy(alpha = 0.4f)
            ViniviaSDK.Filter.NEGATIVE_FILTER -> Color.Magenta.copy(alpha = 0.3f)
            ViniviaSDK.Filter.CUBE_FILTER -> Color.Blue.copy(alpha = 0.3f)
            ViniviaSDK.Filter.DEPTH_MAP_FILTER -> Color.Cyan.copy(alpha = 0.3f)
            ViniviaSDK.Filter.POSITIVE_MASK_FILTER -> Color.Green.copy(alpha = 0.3f)
            ViniviaSDK.Filter.NEGATIVE_MASK_FILTER -> Color.Red.copy(alpha = 0.3f)
            ViniviaSDK.Filter.CUBE_OVER_MASK_FILTER -> Color.Yellow.copy(alpha = 0.3f)
            else -> Color.Transparent
        }

        if (overlayColor != Color.Transparent) {
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .background(overlayColor)
            )
        }
    }
}

/**
 * Filter selection section composable
 * 
 * This demonstrates:
 * 1. LazyRow for horizontal scrolling
 * 2. Custom filter chips with animations
 * 3. State-based UI updates
 */
@Composable
private fun FilterSelectionSection(
    uiState: CameraUiState,
    onFilterSelected: (FilterItem) -> Unit
) {
    Column {
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(
                text = "Filters",
                style = MaterialTheme.typography.titleMedium,
                fontWeight = FontWeight.SemiBold
            )
            
            uiState.selectedFilter?.let { filter ->
                Text(
                    text = filter.displayName,
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.primary
                )
            }
        }
        
        Spacer(modifier = Modifier.height(8.dp))
        
        // Current filter description
        uiState.selectedFilter?.let { filter ->
            Text(
                text = filter.description,
                style = MaterialTheme.typography.bodySmall,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                modifier = Modifier.padding(bottom = 12.dp)
            )
        }
        
        // Filter chips in horizontal scrollable row
        LazyRow(
            horizontalArrangement = Arrangement.spacedBy(8.dp),
            contentPadding = PaddingValues(horizontal = 4.dp)
        ) {
            items(uiState.availableFilters) { filter ->
                FilterChip(
                    filterItem = filter,
                    isSelected = filter == uiState.selectedFilter,
                    onClick = { onFilterSelected(filter) },
                    enabled = !uiState.isApplyingFilter
                )
            }
        }
    }
}

/**
 * Custom filter chip composable
 * 
 * This demonstrates:
 * 1. Custom chip design with Material 3
 * 2. Animated state changes
 * 3. Proper accessibility support
 */
@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun FilterChip(
    filterItem: FilterItem,
    isSelected: Boolean,
    onClick: () -> Unit,
    enabled: Boolean
) {
    FilterChip(
        onClick = onClick,
        label = {
            Text(
                text = filterItem.displayName,
                style = MaterialTheme.typography.labelMedium
            )
        },
        selected = isSelected,
        enabled = enabled,
        modifier = Modifier.height(40.dp)
    )
}

/**
 * Loading indicator composable
 * 
 * This shows a loading state while the camera is initializing
 */
@Composable
private fun LoadingIndicator() {
    Column(
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.Center
    ) {
        CircularProgressIndicator()
        Spacer(modifier = Modifier.height(16.dp))
        Text(
            text = "Initializing Camera...",
            style = MaterialTheme.typography.bodyMedium,
            textAlign = TextAlign.Center
        )
    }
} 