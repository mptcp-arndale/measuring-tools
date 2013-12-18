# Copyright (C) 2008 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := smt-echod
LOCAL_MODULE_TAGS := eng
LOCAL_SRC_FILES := echod.c
LOCAL_CFLAGS := -std=c99
include $(BUILD_EXECUTABLE)

#####################################

include $(CLEAR_VARS)
LOCAL_MODULE := smt-stream_measurer
LOCAL_MODULE_TAGS := eng
LOCAL_SRC_FILES := stream_measurer.c
include $(BUILD_EXECUTABLE)

#####################################

include $(CLEAR_VARS)
LOCAL_MODULE := smt-trafficgen
LOCAL_MODULE_TAGS := eng
LOCAL_SRC_FILES := trafficgen.c
include $(BUILD_EXECUTABLE)
