package com.vinivia.compose.demo.ui

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.vinivia.sdk.api.ViniviaSDK
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch

/**
 * ViewModel for managing camera and filter state
 * 
 * This ViewModel demonstrates proper state management with Jetpack Compose
 * and integration with the native ViniviaSDK filtering capabilities.
 */
class CameraViewModel : ViewModel() {
    
    // Private mutable state
    private val _uiState = MutableStateFlow(CameraUiState())
    
    // Public read-only state
    val uiState: StateFlow<CameraUiState> = _uiState.asStateFlow()
    
    init {
        initializeFilters()
    }
    
    /**
     * Initialize available filters from ViniviaSDK
     */
    private fun initializeFilters() {
        val availableFilters = listOf(
            FilterItem(
                filter = ViniviaSDK.Filter.NO_FILTER,
                displayName = "No Filter",
                description = "Original camera view"
            ),
            FilterItem(
                filter = ViniviaSDK.Filter.GRAYSCALE_FILTER,
                displayName = "Grayscale",
                description = "Black and white effect"
            ),
            FilterItem(
                filter = ViniviaSDK.Filter.NEGATIVE_FILTER,
                displayName = "Negative",
                description = "Inverted colors effect"
            ),
            FilterItem(
                filter = ViniviaSDK.Filter.CUBE_FILTER,
                displayName = "Cube",
                description = "3D cube rendering"
            ),
            FilterItem(
                filter = ViniviaSDK.Filter.DEPTH_MAP_FILTER,
                displayName = "Depth Map",
                description = "Depth-based visualization"
            ),
            FilterItem(
                filter = ViniviaSDK.Filter.POSITIVE_MASK_FILTER,
                displayName = "Positive Mask",
                description = "Positive mask overlay"
            ),
            FilterItem(
                filter = ViniviaSDK.Filter.NEGATIVE_MASK_FILTER,
                displayName = "Negative Mask",
                description = "Negative mask overlay"
            ),
            FilterItem(
                filter = ViniviaSDK.Filter.CUBE_OVER_MASK_FILTER,
                displayName = "Cube Over Mask",
                description = "3D cube with mask overlay"
            )
        )
        
        _uiState.value = _uiState.value.copy(
            availableFilters = availableFilters,
            selectedFilter = availableFilters.first(),
            isLoading = false
        )
    }
    
    /**
     * Handle filter selection
     * 
     * This function demonstrates how to:
     * 1. Update UI state immediately for responsive UI
     * 2. Communicate with native SDK asynchronously
     * 3. Handle potential errors gracefully
     */
    fun onFilterSelected(filterItem: FilterItem) {
        viewModelScope.launch {
            try {
                // Update UI state immediately for responsive user experience
                _uiState.value = _uiState.value.copy(
                    selectedFilter = filterItem,
                    isApplyingFilter = true
                )
                
                // Apply filter through native SDK
                // Note: This is a blocking call to native code, so we do it in a coroutine
                ViniviaSDK.changeActiveFilter(filterItem.filter)
                
                // Update state to indicate filter application is complete
                _uiState.value = _uiState.value.copy(
                    isApplyingFilter = false
                )
                
            } catch (exception: Exception) {
                // Handle any errors that might occur during filter application
                _uiState.value = _uiState.value.copy(
                    isApplyingFilter = false,
                    errorMessage = "Failed to apply filter: ${exception.message}"
                )
            }
        }
    }
    
    /**
     * Clear any error messages
     */
    fun clearError() {
        _uiState.value = _uiState.value.copy(errorMessage = null)
    }
}

/**
 * UI State data class
 * 
 * This represents the complete state of the camera screen UI.
 * Using a single state object makes it easy to manage and test.
 */
data class CameraUiState(
    val availableFilters: List<FilterItem> = emptyList(),
    val selectedFilter: FilterItem? = null,
    val isLoading: Boolean = true,
    val isApplyingFilter: Boolean = false,
    val errorMessage: String? = null
)

/**
 * Filter item data class
 * 
 * This wraps the ViniviaSDK.Filter enum with additional UI-friendly information
 */
data class FilterItem(
    val filter: ViniviaSDK.Filter,
    val displayName: String,
    val description: String
) 