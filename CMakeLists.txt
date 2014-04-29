cmake_minimum_required (VERSION 2.6)

project (libscriba)

set (CMAKE_BUILD_TYPE RelWithDebInfo)
set (CMAKE_C_FLAGS_RELWITHDEBINFO "-std=c99 -ggdb")

option (BUILD_UNIT_TESTS "generate targets to build unit tests" ON)
option (BUILD_JAVA_BINDINGS "generate targets to build Java bindings" ON)

# main library

# library front-end sources
set (LIBSCRIBA_SRC ${libscriba_SOURCE_DIR}/scriba.c
                   ${libscriba_SOURCE_DIR}/types.c
                   ${libscriba_SOURCE_DIR}/company.c
                   ${libscriba_SOURCE_DIR}/event.c
                   ${libscriba_SOURCE_DIR}/poc.c
                   ${libscriba_SOURCE_DIR}/project.c)

# sqlite backend sources
list(APPEND LIBSCRIBA_SRC ${libscriba_SOURCE_DIR}/sqlite-backend/sqlite3.c
                          ${libscriba_SOURCE_DIR}/sqlite-backend/sqlite_backend.c)

include_directories (${libscriba_SOURCE_DIR}/include)
include_directories (${libscriba_SOURCE_DIR}/sqlite-backend)

# libraries linked with libscriba
set(LIBSCRIBA_REQUIRED_LIBS)

# sqlite backend requires pthread
find_library (PTHREAD_LIB NAMES pthread)
if (${PTHREAD_LIB} EQUAL "PTHREAD_LIB-NOTFOUND")
    message(FATAL_ERROR "Couldn't find pthread library")
endif (${PTHREAD_LIB} EQUAL "PTHREAD_LIB-NOTFOUND")
list (APPEND LIBSCRIBA_REQUIRED_LIBS ${PTHREAD_LIB})

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

# unit tests
if (BUILD_UNIT_TESTS)
    # main library tests
    include_directories (${libscriba_SOURCE_DIR})
    set (LIBSCRIBA_UT_SRC ${libscriba_SOURCE_DIR}/test/main.c
                          ${libscriba_SOURCE_DIR}/test/mock_backend.c
                          ${libscriba_SOURCE_DIR}/test/frontend_test.c
                          ${libscriba_SOURCE_DIR}/test/sqlite_backend_test.c)

    find_library (CUNIT_LIB NAMES cunit)
    if (${CUNIT_LIB} EQUAL "CUNIT_LIB-NOTFOUND")
        message (FATAL_ERROR "Couldn't find cunit library")
    endif()

    add_executable (libscriba-test EXCLUDE_FROM_ALL ${LIBSCRIBA_UT_SRC})
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
