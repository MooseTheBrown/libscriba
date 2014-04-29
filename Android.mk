LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := scriba-java
LOCAL_SRC_FILES := company.c event.c org_scribacrm_libscriba_ScribaDB.c poc.c project.c scriba.c sqlite3.c sqlite_backend.c types.c
LOCAL_CFLAGS := -std=c99

include $(BUILD_SHARED_LIBRARY)
