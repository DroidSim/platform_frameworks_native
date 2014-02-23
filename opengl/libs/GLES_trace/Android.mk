LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    src/gltrace_api.cpp \
    src/gltrace_context.cpp \
    src/gltrace_egl.cpp \
    src/gltrace_eglapi.cpp \
    src/gltrace_fixup.cpp \
    src/gltrace_hooks.cpp \
    src/gltrace.pb.cpp \
    src/gltrace_transport.cpp

ifeq ($(TARGET_OS),gnu_linux)
LOCAL_C_INCLUDES := \
    $(LOCAL_PATH) \
    $(LOCAL_PATH)/../ \
    external/protobuf/src \
    external
else
LOCAL_C_INCLUDES := \
    $(LOCAL_PATH) \
    $(LOCAL_PATH)/../ \
    external/stlport/stlport \
    external/protobuf/src \
    external \
    bionic
endif

LOCAL_CFLAGS := -DGOOGLE_PROTOBUF_NO_RTTI
LOCAL_STATIC_LIBRARIES := libprotobuf-cpp-2.3.0-lite liblzf
ifeq ($(TARGET_OS),gnu_linux)
LOCAL_SHARED_LIBRARIES := libcutils libutils liblog
else
LOCAL_SHARED_LIBRARIES := libcutils libutils liblog libstlport
endif

LOCAL_CFLAGS += -DLOG_TAG=\"libGLES_trace\"

ifeq ($(TARGET_OS),gnu_linux)
LOCAL_LDLIBS := -lpthread
else
# we need to access the private Bionic header <bionic_tls.h>
LOCAL_C_INCLUDES += bionic/libc/private
endif

LOCAL_MODULE:= libGLES_trace
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
