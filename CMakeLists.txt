# 
#  Copyright (C) 2015 Mikhail Sapozhnikov
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

cmake_minimum_required (VERSION 2.6)

project (libscriba)

set (CMAKE_BUILD_TYPE RelWithDebInfo)
set (CMAKE_C_FLAGS_RELWITHDEBINFO "-std=c11 -ggdb")
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-std=c++11 -ggdb")

option (BUILD_UNIT_TESTS "generate targets to build unit tests" ON)
option (BUILD_TEST_GENERATOR "generate targets to build test database generator" ON)
option (BUILD_JAVA_BINDINGS "generate targets to build Java bindings" ON)

# main library

# library front-end sources
set (LIBSCRIBA_SRC ${libscriba_SOURCE_DIR}/scriba.c
                   ${libscriba_SOURCE_DIR}/types.c
                   ${libscriba_SOURCE_DIR}/company.c
                   ${libscriba_SOURCE_DIR}/event.c
                   ${libscriba_SOURCE_DIR}/poc.c
                   ${libscriba_SOURCE_DIR}/project.c
                   ${libscriba_SOURCE_DIR}/serializer.cpp)

# sqlite backend sources
list(APPEND LIBSCRIBA_SRC ${libscriba_SOURCE_DIR}/sqlite-backend/sqlite3.c
                          ${libscriba_SOURCE_DIR}/sqlite-backend/sqlite_backend.c)

include_directories (${libscriba_SOURCE_DIR}/include)
include_directories (${libscriba_SOURCE_DIR}/sqlite-backend)
include_directories (${libscriba_SOURCE_DIR})

# libraries linked with libscriba
set(LIBSCRIBA_REQUIRED_LIBS)

# sqlite backend requires pthread
find_library (PTHREAD_LIB NAMES pthread)
if (${PTHREAD_LIB} EQUAL "PTHREAD_LIB-NOTFOUND")
    message(FATAL_ERROR "Couldn't find pthread library")
endif (${PTHREAD_LIB} EQUAL "PTHREAD_LIB-NOTFOUND")
list (APPEND LIBSCRIBA_REQUIRED_LIBS ${PTHREAD_LIB})

# sqlite for scriba is built with ICU support
find_library (ICU_LIB NAMES icui18n)
if (${ICU_LIB} EQUAL "ICU_LIB-NOTFOUND")
    message(FATAL_ERROR "Couldn't find ICU library")
endif (${ICU_LIB} EQUAL "ICU_LIB-NOTFOUND")
list (APPEND LIBSCRIBA_REQUIRED_LIBS ${ICU_LIB})

add_library (scriba SHARED ${LIBSCRIBA_SRC})
target_link_libraries (scriba ${LIBSCRIBA_REQUIRED_LIBS})

# Java bindings
if (BUILD_JAVA_BINDINGS)
    find_package(JNI REQUIRED)
    include_directories(${JNI_INCLUDE_DIRS})
    set (JAVA_BINDINGS_DIR ${libscriba_SOURCE_DIR}/bindings/java)
    set (JAVA_SRC_DIR org/scribacrm/libscriba)
    set (LIBSCRIBA_JAVA_SRC ${JAVA_BINDINGS_DIR}/org_scribacrm_libscriba_ScribaDB.c)
    add_library (scriba-java SHARED ${LIBSCRIBA_JAVA_SRC})
    target_link_libraries (scriba-java scriba)
    add_custom_target (libscriba.jar ALL
                       COMMAND javac ${JAVA_SRC_DIR}/*.java
                       COMMAND jar cf libscriba.jar ${JAVA_SRC_DIR}/*.class
                       COMMAND rm -f ${JAVA_SRC_DIR}/*.class
                       COMMAND mv libscriba.jar ${libscriba_BINARY_DIR}
                       WORKING_DIRECTORY ${JAVA_BINDINGS_DIR})
endif (BUILD_JAVA_BINDINGS)

# test generator
if (BUILD_TEST_GENERATOR)
    add_executable (scriba_test_generator ${libscriba_SOURCE_DIR}/test/test_db_generator.c)
    target_link_libraries (scriba_test_generator scriba)
endif (BUILD_TEST_GENERATOR)

# unit tests
if (BUILD_UNIT_TESTS)
    # main library tests
    set (LIBSCRIBA_UT_SRC ${libscriba_SOURCE_DIR}/test/main.c
                          ${libscriba_SOURCE_DIR}/test/common_test.c
                          ${libscriba_SOURCE_DIR}/test/mock_backend.c
                          ${libscriba_SOURCE_DIR}/test/frontend_test.c
                          ${libscriba_SOURCE_DIR}/test/sqlite_backend_test.c
                          ${libscriba_SOURCE_DIR}/test/serializer_test.c)

    find_library (CUNIT_LIB NAMES cunit)
    if (${CUNIT_LIB} EQUAL "CUNIT_LIB-NOTFOUND")
        message (FATAL_ERROR "Couldn't find cunit library")
    endif()

    add_executable (libscriba-test ${LIBSCRIBA_UT_SRC})
    target_link_libraries (libscriba-test scriba ${CUNIT_LIB})

    # Java bindings tests
    if (BUILD_JAVA_BINDINGS)
        # check junit4 availability
        if (NOT EXISTS ${JUNIT4_JAR_PATH})
            message (FATAL_ERROR "JUNIT4_JAR_PATH is either not specified or points to non-existing file. Correct it or disable building Java bindings unit tests by turning either BUILD_UNIT_TESTS or BUILD_JAVA_BINDINGS off")
        endif (NOT EXISTS ${JUNIT4_JAR_PATH})

        set (JAVA_TEST_DIR ${JAVA_SRC_DIR}/test)
        set (JAVA_CLASSPATH ".:${JUNIT4_JAR_PATH}:${libscriba_BINARY_DIR}/libscriba.jar")
        add_custom_target (libscriba-java-test ALL
                           COMMAND javac -cp ${JAVA_CLASSPATH} ${JAVA_TEST_DIR}/*.java
                           COMMAND jar cf libscriba-java-test.jar ${JAVA_TEST_DIR}/*.class
                           COMMAND rm -f ${JAVA_TEST_DIR}/*.class
                           COMMAND rm -f ${JAVA_SRC_DIR}/*.class
                           COMMAND mv libscriba-java-test.jar ${libscriba_BINARY_DIR}
                           WORKING_DIRECTORY ${JAVA_BINDINGS_DIR})
        add_dependencies (libscriba-java-test libscriba.jar)
        configure_file (${JAVA_BINDINGS_DIR}/${JAVA_TEST_DIR}/java_bindings_test.sh.in
                        ${libscriba_BINARY_DIR}/java_bindings_test.sh)
    endif (BUILD_JAVA_BINDINGS)
endif (BUILD_UNIT_TESTS)
