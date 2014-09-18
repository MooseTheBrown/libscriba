#!/bin/sh

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

usage()
{
    echo "Usage: $0 <android_project_dir> [clear]"
    exit 1
}

if [ -z $1 ]; then
    usage
fi

ANDROID_PROJECT_DIR=$1

SOURCE_DIR=`pwd`

ANDROID_MK_FILE=$SOURCE_DIR/Android.mk

LIBRARY_INCLUDE_DIR=$SOURCE_DIR/include
LIBRARY_INCLUDE_FILES=`ls $LIBRARY_INCLUDE_DIR/*.h`
FLATBUFFERS_INCLUDE_DIR=$SOURCE_DIR/include/flatbuffers
FLATBUFFERS_INCLUDE_FILES=`ls $FLATBUFFERS_INCLUDE_DIR/*.h`

LIBRARY_FRONTEND_FILES=`ls *.c *.cpp *.h`

LIBRARY_BACKEND_DIR=$SOURCE_DIR/sqlite-backend
LIBRARY_BACKEND_FILES=`ls $LIBRARY_BACKEND_DIR/*`

JAVA_BINDINGS_DIR=$SOURCE_DIR/bindings/java
LIBRARY_JNI_FILES=`ls $JAVA_BINDINGS_DIR/*.c $JAVA_BINDINGS_DIR/*.h`
JAVA_SRC_DIR=$JAVA_BINDINGS_DIR/org/scribacrm/libscriba
JAVA_SRC_FILES=`ls $JAVA_SRC_DIR/*.java`

# target directories
NATIVE_TARGET_DIR=$ANDROID_PROJECT_DIR/jni
FLATBUFFERS_TARGET_DIR=$NATIVE_TARGET_DIR/flatbuffers
JAVA_TARGET_DIR=$ANDROID_PROJECT_DIR/src/org/scribacrm/libscriba

copy_files()
{
    echo "Copying files..."

    # copy sources

    mkdir $NATIVE_TARGET_DIR
    for header in $LIBRARY_INCLUDE_FILES; do
        cp $header $NATIVE_TARGET_DIR/
    done

    mkdir $FLATBUFFERS_TARGET_DIR
    for header in $FLATBUFFERS_INCLUDE_FILES; do
        cp $header $FLATBUFFERS_TARGET_DIR/
    done

    for file in $LIBRARY_FRONTEND_FILES; do
        cp $file $NATIVE_TARGET_DIR/
    done

    for file in $LIBRARY_BACKEND_FILES; do
        cp $file $NATIVE_TARGET_DIR/
    done

    # copy Android.mk
    cp $ANDROID_MK_FILE $NATIVE_TARGET_DIR/

    # copy JNI sources
    for file in $LIBRARY_JNI_FILES; do
        cp $file $NATIVE_TARGET_DIR/
    done

    # copy Java sources
    mkdir -p $JAVA_TARGET_DIR
    for file in $JAVA_SRC_FILES; do
        cp $file $JAVA_TARGET_DIR/
    done

    echo "Done"
}

remove_files()
{
    rm -rf $NATIVE_TARGET_DIR
    rm -rf $JAVA_TARGET_DIR
}

if [ -z $2 ]; then
    copy_files
else
    if [ "$2" = "clear" ]; then
        remove_files
    else
        usage
    fi
fi
