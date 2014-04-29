/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class org_scribacrm_libscriba_ScribaDB */

#ifndef _Included_org_scribacrm_libscriba_ScribaDB
#define _Included_org_scribacrm_libscriba_ScribaDB
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    init
 * Signature: (Lorg/scribacrm/libscriba/ScribaDB/DBDescr;[Lorg/scribacrm/libscriba/ScribaDB/DBParam;)I
 */
JNIEXPORT jint JNICALL Java_org_scribacrm_libscriba_ScribaDB_init
  (JNIEnv *, jclass, jobject, jobjectArray);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    cleanup
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_scribacrm_libscriba_ScribaDB_cleanup
  (JNIEnv *, jclass);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    getCompany
 * Signature: (J)Lorg/scribacrm/libscriba/Company;
 */
JNIEXPORT jobject JNICALL Java_org_scribacrm_libscriba_ScribaDB_getCompany
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    getAllCompanies
 * Signature: ()[Lorg/scribacrm/libscriba/DataDescriptor;
 */
JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getAllCompanies
  (JNIEnv *, jclass);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    getCompaniesByName
 * Signature: (Ljava/lang/String;)[Lorg/scribacrm/libscriba/DataDescriptor;
 */
JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getCompaniesByName
  (JNIEnv *, jclass, jstring);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    getCompaniesByJurName
 * Signature: (Ljava/lang/String;)[Lorg/scribacrm/libscriba/DataDescriptor;
 */
JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getCompaniesByJurName
  (JNIEnv *, jclass, jstring);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    getCompaniesByAddress
 * Signature: (Ljava/lang/String;)[Lorg/scribacrm/libscriba/DataDescriptor;
 */
JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getCompaniesByAddress
  (JNIEnv *, jclass, jstring);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    addCompany
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_scribacrm_libscriba_ScribaDB_addCompany
  (JNIEnv *, jclass, jstring, jstring, jstring, jstring, jstring, jstring);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    updateCompany
 * Signature: (Lorg/scribacrm/libscriba/Company;)V
 */
JNIEXPORT void JNICALL Java_org_scribacrm_libscriba_ScribaDB_updateCompany
  (JNIEnv *, jclass, jobject);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    removeCompany
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_org_scribacrm_libscriba_ScribaDB_removeCompany
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    getPoc
 * Signature: (J)Lorg/scribacrm/libscriba/POC;
 */
JNIEXPORT jobject JNICALL Java_org_scribacrm_libscriba_ScribaDB_getPoc
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    getAllPeople
 * Signature: ()[Lorg/scribacrm/libscriba/DataDescriptor;
 */
JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getAllPeople
  (JNIEnv *, jclass);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    getPOCByName
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)[Lorg/scribacrm/libscriba/DataDescriptor;
 */
JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getPOCByName
  (JNIEnv *, jclass, jstring, jstring, jstring);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    getPOCByCompany
 * Signature: (J)[Lorg/scribacrm/libscriba/DataDescriptor;
 */
JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getPOCByCompany
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    getPOCByPosition
 * Signature: (Ljava/lang/String;)[Lorg/scribacrm/libscriba/DataDescriptor;
 */
JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getPOCByPosition
  (JNIEnv *, jclass, jstring);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    getPOCByEmail
 * Signature: (Ljava/lang/String;)[Lorg/scribacrm/libscriba/DataDescriptor;
 */
JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getPOCByEmail
  (JNIEnv *, jclass, jstring);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    addPOC
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;J)V
 */
JNIEXPORT void JNICALL Java_org_scribacrm_libscriba_ScribaDB_addPOC
  (JNIEnv *, jclass, jstring, jstring, jstring, jstring, jstring, jstring, jstring, jlong);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    updatePOC
 * Signature: (Lorg/scribacrm/libscriba/POC;)V
 */
JNIEXPORT void JNICALL Java_org_scribacrm_libscriba_ScribaDB_updatePOC
  (JNIEnv *, jclass, jobject);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    removePOC
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_org_scribacrm_libscriba_ScribaDB_removePOC
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    getProject
 * Signature: (J)Lorg/scribacrm/libscriba/Project;
 */
JNIEXPORT jobject JNICALL Java_org_scribacrm_libscriba_ScribaDB_getProject
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    getAllProjects
 * Signature: ()[Lorg/scribacrm/libscriba/DataDescriptor;
 */
JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getAllProjects
  (JNIEnv *, jclass);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    getProjectsByCompany
 * Signature: (J)[Lorg/scribacrm/libscriba/DataDescriptor;
 */
JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getProjectsByCompany
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    getProjectsByState
 * Signature: (B)[Lorg/scribacrm/libscriba/DataDescriptor;
 */
JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getProjectsByState
  (JNIEnv *, jclass, jbyte);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    addProject
 * Signature: (Ljava/lang/String;Ljava/lang/String;JB)V
 */
JNIEXPORT void JNICALL Java_org_scribacrm_libscriba_ScribaDB_addProject
  (JNIEnv *, jclass, jstring, jstring, jlong, jbyte);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    updateProject
 * Signature: (Lorg/scribacrm/libscriba/Project;)V
 */
JNIEXPORT void JNICALL Java_org_scribacrm_libscriba_ScribaDB_updateProject
  (JNIEnv *, jclass, jobject);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    removeProject
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_org_scribacrm_libscriba_ScribaDB_removeProject
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    getEvent
 * Signature: (J)Lorg/scribacrm/libscriba/Event;
 */
JNIEXPORT jobject JNICALL Java_org_scribacrm_libscriba_ScribaDB_getEvent
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    getAllEvents
 * Signature: ()[Lorg/scribacrm/libscriba/DataDescriptor;
 */
JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getAllEvents
  (JNIEnv *, jclass);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    getEventsByCompany
 * Signature: (J)[Lorg/scribacrm/libscriba/DataDescriptor;
 */
JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getEventsByCompany
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    getEventsByPOC
 * Signature: (J)[Lorg/scribacrm/libscriba/DataDescriptor;
 */
JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getEventsByPOC
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    getEventsByProject
 * Signature: (J)[Lorg/scribacrm/libscriba/DataDescriptor;
 */
JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getEventsByProject
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    addEvent
 * Signature: (Ljava/lang/String;JJJBLjava/lang/String;JB)V
 */
JNIEXPORT void JNICALL Java_org_scribacrm_libscriba_ScribaDB_addEvent
  (JNIEnv *, jclass, jstring, jlong, jlong, jlong, jbyte, jstring, jlong, jbyte);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    updateEvent
 * Signature: (Lorg/scribacrm/libscriba/Event;)V
 */
JNIEXPORT void JNICALL Java_org_scribacrm_libscriba_ScribaDB_updateEvent
  (JNIEnv *, jclass, jobject);

/*
 * Class:     org_scribacrm_libscriba_ScribaDB
 * Method:    removeEvent
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_org_scribacrm_libscriba_ScribaDB_removeEvent
  (JNIEnv *, jclass, jlong);

#ifdef __cplusplus
}
#endif
#endif
