LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := FlowerEvolver
LOCAL_SRC_FILES := lib/$(TARGET_ARCH_ABI)/libFlowerEvolver-4.so
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include

prebuilt_path := $(call local-prebuilt-path,$(LOCAL_SRC_FILES))
prebuilt := $(strip $(wildcard $(prebuilt_path)))

ifdef prebuilt
    include $(PREBUILT_SHARED_LIBRARY)
endif

include $(CLEAR_VARS)

LOCAL_MODULE := FlowerEvolver-static
LOCAL_SRC_FILES := lib/$(TARGET_ARCH_ABI)/libFlowerEvolver-4-s.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include

prebuilt_path := $(call local-prebuilt-path,$(LOCAL_SRC_FILES))
prebuilt := $(strip $(wildcard $(prebuilt_path)))

ifdef prebuilt
    include $(PREBUILT_STATIC_LIBRARY)
endif
