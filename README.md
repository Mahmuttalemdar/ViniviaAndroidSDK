# ViniviaAndroidSDK

[![Android CI/CD](https://github.com/Mahmuttalemdar/ViniviaAndroidSDK/actions/workflows/github_ci.yml/badge.svg?branch=main)](https://github.com/Mahmuttalemdar/ViniviaAndroidSDK/actions/workflows/github_ci.yml)

A high-performance Android SDK for camera filtering and rendering with native C++/OpenGL ES integration.

## 🆕 NEW: Jetpack Compose Demo

This repository now includes a **Jetpack Compose demonstration** showcasing modern Android UI integration with the existing native SDK.

### Project Structure

- **`viniviaSDK/`** - Core SDK library (C++/JNI + Kotlin)
- **`compose-demo/`** - **NEW**: Jetpack Compose demonstration app

## 🚀 Compose Demo Features

The `compose-demo` module demonstrates:

- **Modern UI Architecture**: MVVM pattern with Jetpack Compose
- **Legacy Integration**: Seamless embedding of OpenGL-based camera views
- **Real-time Filtering**: Live filter application with responsive UI
- **Material 3 Design**: Latest Material Design components
- **State Management**: Reactive state handling with StateFlow
- **Performance**: Non-blocking filter operations using Coroutines

### Quick Start

1. **Build the project:**
   ```bash
   ./gradlew build
   ```

2. **Run the Compose demo:**
   ```bash
   ./gradlew :compose-demo:installDebug
   ```

3. **Or run the original SDK:**
   ```bash
   ./gradlew :viniviaSDK:installDebug  # Note: This won't work as it's now a library
   ```

### Filter Types Supported

- No Filter (Original)
- Grayscale
- Negative
- Cube (3D rendering)
- Depth Map
- Positive Mask
- Negative Mask
- Cube Over Mask

## 🏗️ Technical Architecture

### Native Layer (C++)
- OpenGL ES 2.0 rendering pipeline
- Real-time filter processing
- JNI bridge for Android integration

### Kotlin Layer
- Modern Android APIs (CameraX, etc.)
- Clean architecture patterns
- Jetpack Compose UI (in demo module)

## 📱 Requirements

- **Android API Level**: 24+ (Android 7.0)
- **Target SDK**: 34 (Android 14)
- **NDK**: Required for native compilation
- **OpenGL ES**: 2.0+

## 🔧 Development Setup

1. **Clone the repository**
2. **Open in Android Studio**
3. **Sync Gradle dependencies**
4. **Build and run**

The project uses the latest Android development tools and follows modern Android development practices.

---

**Note**: This project demonstrates both traditional Android development (native SDK) and modern approaches (Jetpack Compose integration).