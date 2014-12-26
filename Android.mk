# 
#  Copyright (C) 2014 Mikhail Sapozhnikov
#
#  This file is part of libscriba.
#
#  libscriba is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Lesser General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  libscriba is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public License
#  along with libscriba. If not, see <http://www.gnu.org/licenses/>.
#
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := icu-data
LOCAL_SRC_FILES := libicudata.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := icu-i18n
LOCAL_SRC_FILES := libicui18n.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := icu-io
LOCAL_SRC_FILES := libicuio.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := icu-le
LOCAL_SRC_FILES := libicule.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := icu-lx
LOCAL_SRC_FILES := libiculx.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := icu-tu
LOCAL_SRC_FILES := libicutu.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := icu-uc
LOCAL_SRC_FILES := libicuuc.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := scriba-java
LOCAL_SRC_FILES := company.c event.c org_scribacrm_libscriba_ScribaDB.c poc.c project.c scriba.c sqlite3.c sqlite_backend.c types.c serializer.cpp
LOCAL_CFLAGS := -std=c99
LOCAL_CPPFLAGS := -std=c++11
LOCAL_C_INCLUDES := $(LOCAL_PATH)/layout $(LOCAL_PATH)/unicode
LOCAL_STATIC_LIBRARIES := icu-i18n icu=io icu-le icu-lx icu-tu icu-uc icu-data
include $(BUILD_SHARED_LIBRARY)
