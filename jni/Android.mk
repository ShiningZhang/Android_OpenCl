LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := opencl
LOCAL_SRC_FILES := OpenCL_phone.cpp imageprocess.cpp main.cpp OpenCL_RBG2Gray.cpp readbmp.cpp time_cal.cpp
LOCAL_C_INCLUDES := include
include $(BUILD_EXECUTABLE)
