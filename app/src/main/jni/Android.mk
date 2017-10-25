LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := yuv_static
LOCAL_SRC_FILES := libyuv/lib/$(TARGET_ARCH_ABI)/libyuv_static.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := x264
LOCAL_SRC_FILES := libx264/lib/$(TARGET_ARCH_ABI)/libx264.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := faac
LOCAL_SRC_FILES := libfaac/lib/$(TARGET_ARCH_ABI)/libfaac.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avutil
LOCAL_SRC_FILES := ffmpeg/lib/$(TARGET_ARCH_ABI)/libavutil.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := swscale
LOCAL_SRC_FILES := ffmpeg/lib/$(TARGET_ARCH_ABI)/libswscale.a
LOCAL_LDLIBS := -lm
LOCAL_STATIC_LIBRARIES := avutil
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := swresample
LOCAL_SRC_FILES := ffmpeg/lib/$(TARGET_ARCH_ABI)/libswresample.a
LOCAL_LDLIBS := -lm
LOCAL_STATIC_LIBRARIES := avutil
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avcodec
LOCAL_SRC_FILES := ffmpeg/lib/$(TARGET_ARCH_ABI)/libavcodec.a
LOCAL_LDLIBS := -lm -lz
LOCAL_STATIC_LIBRARIES := avutil swresample x264 faac
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avformat
LOCAL_SRC_FILES := ffmpeg/lib/$(TARGET_ARCH_ABI)/libavformat.a
LOCAL_LDLIBS := -lm -lz
LOCAL_STATIC_LIBRARIES := avutil swresample avcodec
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avfilter
LOCAL_SRC_FILES := ffmpeg/lib/$(TARGET_ARCH_ABI)/libavfilter.a
LOCAL_LDLIBS := -lm -lz
LOCAL_STATIC_LIBRARIES := avutil swresample avcodec swscale avformat
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := aalive

LOCAL_DISABLE_FATAL_LINKER_WARNINGS := true
LOCAL_CFLAGS := -D__STDC_CONSTANT_MACROS -D_ANDROID_
LOCAL_CPPFLAGS :=  -std=gnu++11 -fexceptions -frtti

LOCAL_C_INCLUDES := $(LOCAL_PATH)/ffmpeg/include $(LOCAL_PATH)/libyuv/include $(LOCAL_PATH)/gpu


LOCAL_SRC_FILES := gpu/Framebuffer.cpp \
                   gpu/ProgramShaderUtil.cpp \
                   gpu/Texture2d.cpp \
                   gpu/YuvConverter.cpp \
                   CommonGlobaldef.cpp \
                   Thread.cpp \
                   H264Encoder.cpp \
                   AACEncoder.cpp \
                   LiveMuxerInfo.cpp \
                   LiveMuxer.cpp \
                   AudioRecord.cpp \
                   org_angzangy_aalive_LiveTelecastNative.cpp

LOCAL_LDLIBS := -llog  -landroid -lz -lGLESv2 -lOpenSLES

LOCAL_STATIC_LIBRARIES := avcodec avfilter avformat avutil swresample swscale x264 faac yuv_static
include $(BUILD_SHARED_LIBRARY)