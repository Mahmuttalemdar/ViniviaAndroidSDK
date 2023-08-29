package com.vinivia.sdk.utils

import androidx.annotation.IntRange


internal class FpsRecorder(@IntRange(from = 1) bufferLength: Int) {
    private val mTimestamps: LongArray
    private var mIndex = 0
    private var mNumSamples = 0 // Number of samples used in calculation.

    /**
     * Creates an fps recorder that creates a running average of `bufferLength+1` samples.
     */
    init {
        if (bufferLength < 1) {
            throw IllegalArgumentException(
                "Invalid buffer length. Buffer must contain at "
                        + "least 1 sample"
            )
        }
        mTimestamps = LongArray(bufferLength)
    }

    /**
     * Records the latest timestamp and returns the latest fps value or NaN if not enough samples
     * have been recorded.
     */
    fun recordTimestamp(timestampNs: Long): Double {
        // Find the duration between the oldest and newest timestamp
        val nextIndex = (mIndex + 1) % mTimestamps.size
        val duration = timestampNs - mTimestamps[mIndex]
        mTimestamps[mIndex] = timestampNs
        mIndex = nextIndex
        // The discarded sample is used in the calculation, so we use a maximum of bufferLength +
        // 1 samples.
        mNumSamples = Math.min(mNumSamples + 1, mTimestamps.size + 1)
        return if (mNumSamples == mTimestamps.size + 1) {
            (NANOS_IN_SECOND * mTimestamps.size) / duration
        } else Double.NaN

        // Return NaN if we don't have enough samples
    }

    /**
     * Ignores all previously recorded timestamps and calculates timestamps from new recorded
     * timestamps.
     *
     *
     * [.recordTimestamp] will return NaN until enough samples have been
     * recorded.
     */
    fun reset() {
        mNumSamples = 0
        mIndex = 0
    }

    companion object {
        private val NANOS_IN_SECOND = 1000000000.0
    }
}
