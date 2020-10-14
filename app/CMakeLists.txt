# Sets the minimum version of CMake required to build the native
# library. You should either keep the default value or only pass a
# value of 3.4.0 or lower.

cmake_minimum_required(VERSION 3.4.1)

ADD_DEFINITIONS(
-g -W -O3 -D__STDC_CONSTANT_MACROS -D_ANDROID_
)

add_library(yuv_static STATIC IMPORTED)
set_target_properties(
    yuv_static
    PROPERTIES IMPORTED_LOCATION
    #We must use full path to libraries
    ${CMAKE_SOURCE_DIR}/src/main/jni/libyuv/lib/${ANDROID_ABI}/libyuv_static.a)

add_library(x264 STATIC IMPORTED)
set_target_properties(
    x264
    PROPERTIES IMPORTED_LOCATION
    ${CMAKE_SOURCE_DIR}/src/main/jni/libx264/lib/${ANDROID_ABI}/libx264.a)

add_library(fdk-aac STATIC IMPORTED)
set_target_properties(
    fdk-aac
    PROPERTIES IMPORTED_LOCATION
    ${CMAKE_SOURCE_DIR}/src/main/jni/libfaac/lib/${ANDROID_ABI}/libfdk-aac.a)

add_library(avutil STATIC IMPORTED)
set_target_properties(
    avutil
    PROPERTIES IMPORTED_LOCATION
    ${CMAKE_SOURCE_DIR}/src/main/jni/ffmpeg/lib/${ANDROID_ABI}/libavutil.a)

add_library(swscale STATIC IMPORTED)

set_target_properties(
    swscale
    PROPERTIES IMPORTED_LOCATION
    ${CMAKE_SOURCE_DIR}/src/main/jni/ffmpeg/lib/${ANDROID_ABI}/libswscale.a)

add_library(swresample STATIC IMPORTED)
set_target_properties(
    swresample
    PROPERTIES IMPORTED_LOCATION
    ${CMAKE_SOURCE_DIR}/src/main/jni/ffmpeg/lib/${ANDROID_ABI}/libswresample.a)

add_library(avcodec STATIC IMPORTED)
set_target_properties(
    avcodec
    PROPERTIES IMPORTED_LOCATION
    ${CMAKE_SOURCE_DIR}/src/main/jni/ffmpeg/lib/${ANDROID_ABI}/libavcodec.a)

add_library(avformat STATIC IMPORTED)

set_target_properties(
    avformat
    PROPERTIES IMPORTED_LOCATION
    ${CMAKE_SOURCE_DIR}/src/main/jni/ffmpeg/lib/${ANDROID_ABI}/libavformat.a)

include_directories( # Specifies a path to header files
                     ${CMAKE_SOURCE_DIR}/src/main/jni/ffmpeg/include
                     ${CMAKE_SOURCE_DIR}/src/main/jni/libyuv/include
                     ${CMAKE_SOURCE_DIR}/src/main/jni/gpu
)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds it for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             aalive

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             # Associated headers in the same location as their source
             # file are automatically included.
             src/main/jni/gpu/GLUtil.cpp
             src/main/jni/gpu/Framebuffer.cpp
             src/main/jni/gpu/ProgramShaderUtil.cpp
             src/main/jni/gpu/Texture2d.cpp
             src/main/jni/gpu/Shader.cpp
             src/main/jni/gpu/YuvConverter.cpp
             src/main/jni/CommonGlobaldef.cpp
             src/main/jni/Thread.cpp
             src/main/jni/H264Encoder.cpp
             src/main/jni/AACEncoder.cpp
             src/main/jni/LiveMuxerInfo.cpp
             src/main/jni/LiveMuxer.cpp
             src/main/jni/AudioRecord.cpp
             src/main/jni/org_angzangy_aalive_LiveTelecastNative.cpp
)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because system libraries are included in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

find_library( # Sets the name of the path variable.
              zip-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              z )

find_library( # Sets the name of the path variable.
              GLESv2-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              GLESv2 )

find_library( # Sets the name of the path variable.
              OpenSLES-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              OpenSLES )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in the
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       aalive

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib}
                       ${zip-lib}
                       ${GLESv2-lib}
                       ${OpenSLES-lib}
                       yuv_static
                       "-Wl,--whole-archive" # This is important,because those libraries depend each other
                       x264
                       fdk-aac
                       avutil
                       swscale
                       swresample
                       avcodec
                       avformat
                       "-Wl,--no-whole-archive")