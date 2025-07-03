package com.vinivia.sdk.utils

import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Matrix
import java.io.File
import java.io.FileInputStream
import java.io.FileOutputStream
import java.io.IOException

object ImageUtils {

    // Read image from Drawable folder
    fun readImageFromDrawable(
        context: Context,
        resId: Int
    ): Bitmap? {
        return BitmapFactory.decodeResource(context.resources, resId)
    }

    // Read Image from file
    fun readImageFromDisk(context: Context, fileName: String?): Bitmap? {
        return try {
            val file = File(context.filesDir, fileName ?: "")
            val inputStream = FileInputStream(file)
            val bitmap = BitmapFactory.decodeStream(inputStream)
            inputStream.close()
            bitmap
        } catch (e: IOException) {
            e.printStackTrace()
            null
        }
    }

    // Save Image to Memory
    fun saveImageToMemory(context: Context, fileName: String?, bitmap: Bitmap) {
        try {
            val file = File(context.filesDir, fileName ?: "")
            val outputStream = FileOutputStream(file)
            bitmap.compress(Bitmap.CompressFormat.PNG, 100, outputStream)
            outputStream.flush()
            outputStream.close()
        } catch (e: IOException) {
            e.printStackTrace()
        }
    }

    fun rotateBitmap(bitmap: Bitmap, degrees: Float): Bitmap {
        val matrix = Matrix()
        matrix.postRotate(degrees)

        return Bitmap.createBitmap(bitmap, 0, 0, bitmap.width, bitmap.height, matrix, true)
    }
}