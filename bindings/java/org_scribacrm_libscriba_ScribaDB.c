/* 
 * Copyright (C) 2014 Mikhail Sapozhnikov
 *
 * This file is part of libscriba.
 *
 * libscriba is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libscriba is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libscriba. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "org_scribacrm_libscriba_ScribaDB.h"
#include "scriba.h"
#include "company.h"
#include "event.h"
#include "poc.h"
#include "project.h"
#include <string.h>
#include <stdlib.h>

// local utility functions

// string conversion routines
static char *java_string_to_utf8(JNIEnv *env, jclass this, jstring str);
static jstring utf8_to_java_string(JNIEnv *env, jclass this, const char *utf8);

// convert scriba list to an array of DataDescriptor objects
static jobjectArray scriba_list_to_data_descr_array(JNIEnv *env, jclass this, scriba_list_t *list);

// convert Java UUID object to scriba_id_t
static void UUID_to_scriba_id(JNIEnv *env, jobject uuid, scriba_id_t *id);

/* Java passes strings through JNI using "modified" UTF-8 encoding, which
   can lead to disaster if we just consider them normal UTF-8 strings. To
   work around this we manually convert all Java strings to UTF-8 byte arrays
   before passing them further to native code and vice versa */
static char *java_string_to_utf8(JNIEnv *env, jclass this, jstring str)
{
    jmethodID methodID = (*env)->GetStaticMethodID(env, this, "getUtf8FromString",
                                               "(Ljava/lang/String;)[B");
    jbyteArray byteArray = NULL;
    jsize arraySize = 0;
    char *result = NULL;
    jbyte *bytes = NULL;

    if (methodID == NULL)
    {
        goto error;
    }

    byteArray = (*env)->CallStaticObjectMethod(env, this, methodID, str);
    if (byteArray == NULL)
    {
        goto error;
    }

    bytes = (*env)->GetByteArrayElements(env, byteArray, NULL);
    if (bytes == NULL)
    {
        goto error;
    }

    arraySize = (*env)->GetArrayLength(env, byteArray);
    result = (char *)malloc(arraySize + 1);
    if (result == NULL)
    {
        goto error;
    }

    for (int i = 0; i < arraySize; i++)
    {
        result[i] = (char)bytes[i];
    }
    result[arraySize] = 0;

    (*env)->ReleaseByteArrayElements(env, byteArray, bytes, JNI_ABORT);
    return result;

error:
    if ((bytes != NULL) && (byteArray != NULL))
    {
        (*env)->ReleaseByteArrayElements(env, byteArray, bytes, JNI_ABORT);
    }
    if (result != NULL)
    {
        free(result);
    }

    return NULL;
}

static jstring utf8_to_java_string(JNIEnv *env, jclass this, const char *utf8)
{
    jmethodID methodID = (*env)->GetStaticMethodID(env, this, "getStringFromUtf8",
                                                   "([B)Ljava/lang/String;");
    jsize len = 0;
    jbyteArray byteArray = NULL;
    jbyte *bytes = NULL;
    jstring result = NULL;

    if (utf8 == NULL)
    {
        return NULL;
    }

    len = strlen(utf8);
    if (len == 0)
    {
        return NULL;
    }

    byteArray = (*env)->NewByteArray(env, len);
    if (byteArray == NULL)
    {
        return NULL;
    }

    bytes = (*env)->GetByteArrayElements(env, byteArray, NULL);
    for (int i = 0; i < len; i++)
    {
        bytes[i] = utf8[i];
    }
    (*env)->ReleaseByteArrayElements(env, byteArray, bytes, JNI_COMMIT);

    result = (*env)->CallStaticObjectMethod(env, this, methodID, byteArray);
    return result;
}

// convert scriba list to an array of DataDescriptor objects
static jobjectArray scriba_list_to_data_descr_array(JNIEnv *env, jclass this, scriba_list_t *list)
{
    jobjectArray java_array = NULL;
    jobject *data_descriptors = NULL;
    jmethodID constructor_id = NULL;
    jclass data_descr_class = NULL;
    int num_elements = 0;
    int i = 0;

    if (list == NULL)
    {
        goto exit;
    }

    data_descr_class = (*env)->FindClass(env, "org/scribacrm/libscriba/DataDescriptor");
    if (data_descr_class == NULL)
    {
        goto exit;
    }
    constructor_id = (*env)->GetMethodID(env,
                                      data_descr_class,
                                      "<init>", "(JLjava/lang/String;)V");
    if (constructor_id == NULL)
    {
        goto exit;
    }

    scriba_list_for_each(list, item)
    {
        num_elements++;
    }

    java_array = (*env)->NewObjectArray(env, num_elements, data_descr_class, NULL);
    if (java_array == NULL)
    {
        goto exit;
    }

    if (num_elements == 0)
    {
        goto exit;
    }

    data_descriptors = (jobject *)malloc(num_elements * sizeof(jobject));
    if (data_descriptors == NULL)
    {
        goto exit;
    }

    scriba_list_for_each(list, item)
    {
        jlong item_id = item->id;
        jstring item_text = utf8_to_java_string(env, this, item->text);

        data_descriptors[i] = (*env)->NewObject(env, data_descr_class, constructor_id,
                                             item_id, item_text);
        if (data_descriptors[i] == NULL)
        {
            goto exit;
        }

        i++;
    }

    for (i = 0; i < num_elements; i++)
    {
        (*env)->SetObjectArrayElement(env, java_array, i, data_descriptors[i]);
    }

exit:
    if (data_descriptors != NULL)
    {
        free(data_descriptors);
    }
    return java_array;
}

// convert Java UUID object to scriba_id_t
static void UUID_to_scriba_id(JNIEnv *env, jobject uuid, scriba_id_t *id)
{
    jclass uuid_class = NULL;
    jmethodID uuid_get_high_id = NULL;
    jmethodID uuid_get_low_id = NULL;
    jlong id_high = 0;
    jlong id_low = 0;

    if ((uuid == NULL) || (id == NULL))
    {
        return;
    }

    uuid_class = (*env)->GetObjectClass(env, uuid);
    if (uuid_class == NULL)
    {
        return;
    }

    uuid_get_high_id = (*env)->GetMethodID(env,
                                           uuid_class,
                                           "getMostSignificantBits",
                                           "(V)J");
    uuid_get_low_id = (*env)->GetMethodID(env,
                                          uuid_class,
                                          "getLeastSignificantBits",
                                          "(V)J");

    if ((uuid_get_high_id == NULL) || (uuid_get_low_id == NULL))
    {
        return;
    }

    id_high = (*env)->CallLongMethod(env, uuid, uuid_get_high_id);
    id_low = (*env)->CallLongMethod(env, uuid, uuid_get_low_id);

    id->_high = (unsigned long long)id_high;
    id->_low = (unsigned long long id_low);
}



// implementation of native functions from ScribaDB class

JNIEXPORT jint JNICALL Java_org_scribacrm_libscriba_ScribaDB_init(JNIEnv *env,
                                                                  jclass this,
                                                                  jobject dbDescr,
                                                                  jobjectArray dbParams)
{
    jint ret = 0;
    struct ScribaDB db;
    jfieldID fieldID = NULL;
    jclass class = NULL;
    jstring dbNameString = NULL;
    jbyte dbType = 0;
    struct ScribaDBParamList *pl = NULL;
    jsize plSize = 0;

    memset(&db, 0, sizeof (struct ScribaDB));

    class = (*env)->GetObjectClass(env, dbDescr);
    if (class == NULL)
    {
        ret = SCRIBA_INIT_MAX_ERR + 1;
        goto exit;
    }

    // get db name string
    fieldID = (*env)->GetFieldID(env, class, "name", "Ljava/lang/String;");
    if (fieldID == NULL)
    {
        ret = SCRIBA_INIT_MAX_ERR + 1;
        goto exit;
    }
    dbNameString = (*env)->GetObjectField(env, dbDescr, fieldID);
    if (dbNameString == NULL)
    {
        ret = SCRIBA_INIT_MAX_ERR + 1;
        goto exit;
    }
    db.name = java_string_to_utf8(env, this, dbNameString);

    // get db type value
    fieldID = (*env)->GetFieldID(env, class, "type", "B");
    if (fieldID == NULL)
    {
        ret = SCRIBA_INIT_MAX_ERR + 1;
        goto exit;
    }
    dbType = (*env)->GetByteField(env, dbDescr, fieldID);
    if (dbType == 0)
    {
        // built-in
        db.type = SCRIBA_DB_BUILTIN;
    }
    else if (dbType == 1)
    {
        // external
        db.type = SCRIBA_DB_EXT;
    }

    (*env)->DeleteLocalRef(env, dbNameString);

    // populate param list
    plSize = (*env)->GetArrayLength(env, dbParams);
    if (plSize != 0)
    {
        struct ScribaDBParamList **cur_param = &pl;
        int i = 0;

        for (; i < plSize; i++, cur_param = &((*cur_param)->next))
        {
            jobject cur_java_param = (*env)->GetObjectArrayElement(env, dbParams, i);
            jstring java_param_key;
            jstring java_param_value;

            if (cur_java_param == NULL)
            {
                ret = SCRIBA_INIT_MAX_ERR + 1;
                goto exit;
            }

            class = (*env)->GetObjectClass(env, cur_java_param);
            if (class == NULL)
            {
                ret = SCRIBA_INIT_MAX_ERR + 1;
                goto exit;
            }
            fieldID = (*env)->GetFieldID(env, class, "key", "Ljava/lang/String;");
            java_param_key = (*env)->GetObjectField(env, cur_java_param, fieldID);
            fieldID = (*env)->GetFieldID(env, class, "value", "Ljava/lang/String;");
            java_param_value = (*env)->GetObjectField(env, cur_java_param, fieldID);
            if ((java_param_key == NULL) || (java_param_value == NULL))
            {
                ret = SCRIBA_INIT_MAX_ERR + 1;
                goto exit;
            }

            (*cur_param) = (struct ScribaDBParamList *)malloc(sizeof (struct ScribaDBParamList));
            if ((*cur_param) == NULL)
            {
                ret = SCRIBA_INIT_MAX_ERR + 1;
                goto exit;
            }
            (*cur_param)->next = NULL;
            (*cur_param)->param = (struct ScribaDBParam *)malloc(sizeof (struct ScribaDBParam));
            if ((*cur_param)->param == NULL)
            {
                ret = SCRIBA_INIT_MAX_ERR + 1;
                goto exit;
            }
            (*cur_param)->param->key = java_string_to_utf8(env, this, java_param_key);
            (*cur_param)->param->value = java_string_to_utf8(env, this, java_param_value);
        }
    }

    // call library init
    ret = scriba_init(&db, pl);

exit:
    if (db.name != NULL)
    {
        free(db.name);
    }
    if (pl != NULL)
    {
        struct ScribaDBParamList *cur_param = pl;

        do
        {
            struct ScribaDBParamList *to_delete = cur_param;

            if (cur_param->param != NULL)
            {
                if (cur_param->param->key != NULL)
                {
                    free(cur_param->param->key);
                }
                if (cur_param->param->value != NULL)
                {
                    free(cur_param->param->value);
                }

                free(cur_param->param);
            }

            cur_param = cur_param->next;
            free(to_delete);
        } while (cur_param != NULL);
    }
    return ret;
}

JNIEXPORT void JNICALL Java_org_scribacrm_libscriba_ScribaDB_cleanup(JNIEnv *env, jclass this)
{
    scriba_cleanup();
}

JNIEXPORT jobject JNICALL Java_org_scribacrm_libscriba_ScribaDB_getCompany(JNIEnv *env,
                                                                           jclass this,
                                                                           jobject id)
{
    jobject java_company = NULL;
    jclass company_class = NULL;
    struct ScribaCompany *company = NULL;
    jstring name = NULL;
    jstring jur_name = NULL;
    jstring address = NULL;
    jstring inn = NULL;
    jstring phonenum = NULL;
    jstring email = NULL;
    jobjectArray poc_list = NULL;
    jobjectArray proj_list = NULL;
    jobjectArray event_list = NULL;
    char *inn_string = NULL;
    jmethodID company_ctor_id = NULL;

    company = scriba_getCompany((scriba_id_t)id);

    if (company == NULL)
    {
        goto exit;
    }

    company_class = (*env)->FindClass(env, "org/scribacrm/libscriba/Company");
    if (company_class == NULL)
    {
        goto exit;
    }

    name = utf8_to_java_string(env, this, company->name);
    jur_name = utf8_to_java_string(env, this, company->jur_name);
    address = utf8_to_java_string(env, this, company->address);
    phonenum = utf8_to_java_string(env, this, company->phonenum);
    email = utf8_to_java_string(env, this, company->email);
    inn_string = scriba_inn_to_string(&(company->inn));
    if (inn_string != NULL)
    {
        inn = utf8_to_java_string(env, this, inn_string);
    }
    poc_list = scriba_list_to_data_descr_array(env, this, company->poc_list);
    proj_list = scriba_list_to_data_descr_array(env, this, company->proj_list);
    event_list = scriba_list_to_data_descr_array(env, this, company->event_list);

    company_ctor_id = (*env)->GetMethodID(env,
                                          company_class,
                                          "<init>",
                                          "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;"
                                          "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
                                          "[Lorg/scribacrm/libscriba/DataDescriptor;"
                                          "[Lorg/scribacrm/libscriba/DataDescriptor;"
                                          "[Lorg/scribacrm/libscriba/DataDescriptor;)V");
    if (company_ctor_id == NULL)
    {
        goto exit;
    }

    java_company = (*env)->NewObject(env, company_class, company_ctor_id,
                                  id, name, jur_name, address, inn, phonenum, email,
                                  poc_list, proj_list, event_list);

exit:
    if (inn_string != NULL)
    {
        free(inn_string);
    }
    if (company != NULL)
    {
        scriba_freeCompanyData(company);
    }

    return java_company;
}

JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getAllCompanies(JNIEnv *env,
                                                                                     jclass this)
{
    jobjectArray company_list = NULL;
    scriba_list_t *companies = scriba_getAllCompanies();

    company_list = scriba_list_to_data_descr_array(env, this, companies);

    scriba_list_delete(companies);

    return company_list;
}

JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getCompaniesByName(JNIEnv *env,
                                                                                        jclass this,
                                                                                        jstring java_name)
{
    jobjectArray company_list = NULL;
    char *name = java_string_to_utf8(env, this, java_name);
    scriba_list_t *companies = scriba_getCompaniesByName(name);

    company_list = scriba_list_to_data_descr_array(env, this, companies);

    if (name != NULL)
    {
        free(name);
    }

    scriba_list_delete(companies);

    return company_list;
}

JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getCompaniesByJurName(JNIEnv *env,
                                                                                           jclass this,
                                                                                           jstring java_jur_name)
{
    jobjectArray company_list = NULL;
    char *jur_name = java_string_to_utf8(env, this, java_jur_name);
    scriba_list_t *companies = scriba_getCompaniesByJurName(jur_name);

    company_list = scriba_list_to_data_descr_array(env, this, companies);

    if (jur_name != NULL)
    {
        free(jur_name);
    }

    scriba_list_delete(companies);

    return company_list;
}

JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getCompaniesByAddress(JNIEnv *env,
                                                                                           jclass this,
                                                                                           jstring java_address)
{
    jobjectArray company_list = NULL;
    char *address = java_string_to_utf8(env, this, java_address);
    scriba_list_t *companies = scriba_getCompaniesByAddress(address);

    company_list = scriba_list_to_data_descr_array(env, this, companies);

    if (address != NULL)
    {
        free(address);
    }

    scriba_list_delete(companies);

    return company_list;
}

JNIEXPORT void JNICALL Java_org_scribacrm_libscriba_ScribaDB_addCompany(JNIEnv *env,
                                                                        jclass this,
                                                                        jstring java_name,
                                                                        jstring java_jur_name,
                                                                        jstring java_address,
                                                                        jstring java_inn,
                                                                        jstring java_phonenum,
                                                                        jstring java_email)
{
    char *name = java_string_to_utf8(env, this, java_name);
    char *jur_name = java_string_to_utf8(env, this, java_jur_name);
    char *address = java_string_to_utf8(env, this, java_address);
    char *phonenum = java_string_to_utf8(env, this, java_phonenum);
    char *email = java_string_to_utf8(env, this, java_email);
    char *inn_string = java_string_to_utf8(env, this, java_inn);
    scriba_inn_t inn = scriba_inn_from_string(inn_string);

    scriba_addCompany(name, jur_name, address, inn, phonenum, email);

    if (name != NULL)
    {
        free(name);
    }
    if (jur_name != NULL)
    {
        free(jur_name);
    }
    if (address != NULL)
    {
        free(address);
    }
    if (phonenum != NULL)
    {
        free(phonenum);
    }
    if (inn_string != NULL)
    {
        free(inn_string);
    }
    if (email != NULL)
    {
        free(email);
    }
}

JNIEXPORT void JNICALL Java_org_scribacrm_libscriba_ScribaDB_updateCompany(JNIEnv *env,
                                                                           jclass this,
                                                                           jobject java_company)
{
    jclass company_class = NULL;
    struct ScribaCompany *company = NULL;
    jfieldID fieldID = NULL;
    char *inn_string = NULL;

    if (java_company == NULL)
    {
        return;
    }

    company = (struct ScribaCompany *)malloc(sizeof (struct ScribaCompany));
    if (company == NULL)
    {
        goto exit;
    }
    memset((void *)company, 0, sizeof(struct ScribaCompany));

    company_class = (*env)->GetObjectClass(env, java_company);
    if (company_class == NULL)
    {
        goto exit;
    }

    // get fields of Java Company object one by one and copy them
    // to the native ScribaCompany structure

    fieldID = (*env)->GetFieldID(env, company_class, "id", "J");
    if (fieldID == NULL)
    {
        goto exit;
    }
    company->id = (scriba_id_t)((*env)->GetLongField(env, java_company, fieldID));

    fieldID = (*env)->GetFieldID(env, company_class, "name", "Ljava/lang/String;");
    if (fieldID == NULL)
    {
        goto exit;
    }
    company->name = java_string_to_utf8(env, this,
                                        (jstring)((*env)->GetObjectField(env, java_company, fieldID)));

    fieldID = (*env)->GetFieldID(env, company_class, "jur_name", "Ljava/lang/String;");
    if (fieldID == NULL)
    {
        goto exit;
    }
    company->jur_name = java_string_to_utf8(env, this,
                                            (jstring)((*env)->GetObjectField(env, java_company, fieldID)));

    fieldID = (*env)->GetFieldID(env, company_class, "address", "Ljava/lang/String;");
    if (fieldID == NULL)
    {
        goto exit;
    }
    company->address = java_string_to_utf8(env, this,
                                           (jstring)((*env)->GetObjectField(env, java_company, fieldID)));

    fieldID = (*env)->GetFieldID(env, company_class, "inn", "Ljava/lang/String;");
    if (fieldID == NULL)
    {
        goto exit;
    }
    inn_string = java_string_to_utf8(env, this,
                                     (jstring)((*env)->GetObjectField(env, java_company, fieldID)));
    company->inn = scriba_inn_from_string(inn_string);

    fieldID = (*env)->GetFieldID(env, company_class, "phonenum", "Ljava/lang/String;");
    if (fieldID == NULL)
    {
        goto exit;
    }
    company->phonenum = java_string_to_utf8(env, this,
                                            (jstring)((*env)->GetObjectField(env, java_company, fieldID)));

    fieldID = (*env)->GetFieldID(env, company_class, "email", "Ljava/lang/String;");
    if (fieldID == NULL)
    {
        goto exit;
    }
    company->email = java_string_to_utf8(env, this,
                                         (jstring)((*env)->GetObjectField(env, java_company, fieldID)));

    scriba_updateCompany(company); 

exit:
    if (company != NULL)
    {
        scriba_freeCompanyData(company);
    }
    if (inn_string != NULL)
    {
        free(inn_string);
    }
}

JNIEXPORT void JNICALL Java_org_scribacrm_libscriba_ScribaDB_removeCompany(JNIEnv *env,
                                                                           jclass this,
                                                                           jlong id)
{
    scriba_removeCompany((scriba_id_t)id);
}

JNIEXPORT jobject JNICALL Java_org_scribacrm_libscriba_ScribaDB_getPoc(JNIEnv *env,
                                                                       jclass this,
                                                                       jlong id)
{
    struct ScribaPoc *poc = NULL;
    jobject java_poc = NULL;
    jclass poc_class = NULL;
    jmethodID poc_ctor_id = NULL;
    jstring firstname = NULL;
    jstring secondname = NULL;
    jstring lastname = NULL;
    jstring mobilenum = NULL;
    jstring phonenum = NULL;
    jstring email = NULL;
    jstring position = NULL;
    jlong company_id = 0;

    poc = scriba_getPOC((scriba_id_t)id);
    if (poc == NULL)
    {
        goto exit;
    }

    poc_class = (*env)->FindClass(env, "org/scribacrm/libscriba/POC");
    if (poc_class == NULL)
    {
        goto exit;
    }

    poc_ctor_id = (*env)->GetMethodID(env, poc_class, "<init>",
                                   "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;"
                                   "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
                                   "Ljava/lang/String;J)V");
    if (poc_ctor_id == NULL)
    {
        goto exit;
    }

    // copy data from ScribaPoc structure to java variables
    firstname = utf8_to_java_string(env, this, poc->firstname);
    secondname = utf8_to_java_string(env, this, poc->secondname);
    lastname = utf8_to_java_string(env, this, poc->lastname);
    mobilenum = utf8_to_java_string(env, this, poc->mobilenum);
    phonenum = utf8_to_java_string(env, this, poc->phonenum);
    email = utf8_to_java_string(env, this, poc->email);
    position = utf8_to_java_string(env, this, poc->position);
    company_id = (jlong)(poc->company_id);

    // create java POC object
    java_poc = (*env)->NewObject(env, poc_class, poc_ctor_id,
                              id, firstname, secondname, lastname, mobilenum,
                              phonenum, email, position, company_id);

exit:
    if (poc != NULL)
    {
        scriba_freePOCData(poc);
    }

    return java_poc;
}

JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getAllPeople(JNIEnv *env, jclass this)
{
    jobjectArray java_poc_list = NULL;
    scriba_list_t *poc_list = scriba_getAllPeople();

    java_poc_list = scriba_list_to_data_descr_array(env, this, poc_list);
    scriba_list_delete(poc_list);

    return java_poc_list;
}

JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getPOCByName(JNIEnv *env,
                                                                                  jclass this,
                                                                                  jstring firstname,
                                                                                  jstring secondname,
                                                                                  jstring lastname)
{
    jobjectArray java_poc_list = NULL;
    scriba_list_t *poc_list = NULL;
    char *native_firstname = java_string_to_utf8(env, this, firstname);
    char *native_secondname = java_string_to_utf8(env, this, secondname);
    char *native_lastname = java_string_to_utf8(env, this, lastname);

    poc_list = scriba_getPOCByName(native_firstname, native_secondname, native_lastname);

    java_poc_list = scriba_list_to_data_descr_array(env, this, poc_list);
    
exit:
    if (native_firstname != NULL)
    {
        free(native_firstname);
    }
    if (native_secondname != NULL)
    {
        free(native_secondname);
    }
    if (native_lastname != NULL)
    {
        free(native_lastname);
    }
    scriba_list_delete(poc_list);
    return java_poc_list;
}

JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getPOCByCompany(JNIEnv *env,
                                                                                     jclass this,
                                                                                     jlong company_id)
{
    scriba_list_t *poc_list = scriba_getPOCByCompany((scriba_id_t)company_id);
    jobjectArray java_poc_list = scriba_list_to_data_descr_array(env, this, poc_list);

    scriba_list_delete(poc_list);
    return java_poc_list;
}

JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getPOCByPosition(JNIEnv *env,
                                                                                      jclass this,
                                                                                      jstring position)
{
    char *native_position = java_string_to_utf8(env, this, position);
    scriba_list_t *poc_list = scriba_getPOCByPosition(native_position);
    jobjectArray java_poc_list = scriba_list_to_data_descr_array(env, this, poc_list);

    if (native_position != NULL)
    {
        free(native_position);
    }
    scriba_list_delete(poc_list);

    return java_poc_list;
}

JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getPOCByEmail(JNIEnv *env,
                                                                                   jclass this,
                                                                                   jstring email)
{
    char *native_email = java_string_to_utf8(env, this, email);
    scriba_list_t *poc_list = scriba_getPOCByEmail(native_email);
    jobjectArray java_poc_list = scriba_list_to_data_descr_array(env, this, poc_list);

    if (native_email != NULL)
    {
        free(native_email);
    }
    scriba_list_delete(poc_list);

    return java_poc_list;
}

JNIEXPORT void JNICALL Java_org_scribacrm_libscriba_ScribaDB_addPOC(JNIEnv *env,
                                                                    jclass this,
                                                                    jstring firstname,
                                                                    jstring secondname,
                                                                    jstring lastname,
                                                                    jstring mobilenum,
                                                                    jstring phonenum,
                                                                    jstring email,
                                                                    jstring position,
                                                                    jlong company_id)
{
    char *native_firstname = java_string_to_utf8(env, this, firstname);
    char *native_secondname = java_string_to_utf8(env, this, secondname);
    char *native_lastname = java_string_to_utf8(env, this, lastname);
    char *native_mobilenum = java_string_to_utf8(env, this, mobilenum);
    char *native_phonenum = java_string_to_utf8(env, this, phonenum);
    char *native_email = java_string_to_utf8(env, this, email);
    char *native_position = java_string_to_utf8(env, this, position);

    scriba_addPOC(native_firstname, native_secondname, native_lastname,
                  native_mobilenum, native_phonenum, native_email,
                  native_position, (scriba_id_t)company_id);

    if (native_firstname != NULL)
    {
        free(native_firstname);
    }
    if (native_secondname != NULL)
    {
        free(native_secondname);
    }
    if (native_lastname != NULL)
    {
        free(native_lastname);
    }
    if (native_mobilenum != NULL)
    {
        free(native_mobilenum);
    }
    if (native_phonenum != NULL)
    {
        free(native_phonenum);
    }
    if (native_email != NULL)
    {
        free(native_email);
    }
    if (native_position != NULL)
    {
        free(native_position);
    }
}

JNIEXPORT void JNICALL Java_org_scribacrm_libscriba_ScribaDB_updatePOC(JNIEnv *env,
                                                                       jclass this,
                                                                       jobject java_poc)
{
    struct ScribaPoc *poc = NULL;
    jclass poc_class = NULL;
    jfieldID fieldID = NULL;

    if (java_poc == NULL)
    {
        return;
    }

    poc = (struct ScribaPoc*)malloc(sizeof (struct ScribaPoc));
    if (poc == NULL)
    {
        goto exit;
    }
    memset((void *)poc, 0, sizeof (struct ScribaPoc));

    poc_class = (*env)->GetObjectClass(env, java_poc);
    if (poc_class == NULL)
    {
        goto exit;
    }

    // get fields of Java POC object one by one and copy their values to ScribaPoc structure
    
    fieldID = (*env)->GetFieldID(env, poc_class, "id", "J");
    if (fieldID == NULL)
    {
        goto exit;
    }
    poc->id = (scriba_id_t)((*env)->GetLongField(env, java_poc, fieldID));

    fieldID = (*env)->GetFieldID(env, poc_class, "firstname", "Ljava/lang/String;");
    if (fieldID == NULL)
    {
        goto exit;
    }
    poc->firstname = java_string_to_utf8(env, this,
                                         (jstring)((*env)->GetObjectField(env, java_poc, fieldID)));

    fieldID = (*env)->GetFieldID(env, poc_class, "secondname", "Ljava/lang/String;");
    if (fieldID == NULL)
    {
        goto exit;
    }
    poc->secondname = java_string_to_utf8(env, this,
                                          (jstring)((*env)->GetObjectField(env, java_poc, fieldID)));

    fieldID = (*env)->GetFieldID(env, poc_class, "lastname", "Ljava/lang/String;");
    if (fieldID == NULL)
    {
        goto exit;
    }
    poc->lastname = java_string_to_utf8(env, this,
                                        (jstring)((*env)->GetObjectField(env, java_poc, fieldID)));

    fieldID = (*env)->GetFieldID(env, poc_class, "mobilenum", "Ljava/lang/String;");
    if (fieldID == NULL)
    {
        goto exit;
    }
    poc->mobilenum = java_string_to_utf8(env, this,
                                         (jstring)((*env)->GetObjectField(env, java_poc, fieldID)));

    fieldID = (*env)->GetFieldID(env, poc_class, "phonenum", "Ljava/lang/String;");
    if (fieldID == NULL)
    {
        goto exit;
    }
    poc->phonenum = java_string_to_utf8(env, this,
                                        (jstring)((*env)->GetObjectField(env, java_poc, fieldID)));

    fieldID = (*env)->GetFieldID(env, poc_class, "email", "Ljava/lang/String;");
    if (fieldID == NULL)
    {
        goto exit;
    }
    poc->email = java_string_to_utf8(env, this,
                                     (jstring)((*env)->GetObjectField(env, java_poc, fieldID)));

    fieldID = (*env)->GetFieldID(env, poc_class, "position", "Ljava/lang/String;");
    if (fieldID == NULL)
    {
        goto exit;
    }
    poc->position = java_string_to_utf8(env, this,
                                        (jstring)((*env)->GetObjectField(env, java_poc, fieldID)));

    fieldID = (*env)->GetFieldID(env, poc_class, "company_id", "J");
    if (fieldID == NULL)
    {
        goto exit;
    }
    poc->company_id = (scriba_id_t)((*env)->GetLongField(env, java_poc, fieldID));

    scriba_updatePOC(poc);

exit:
    if (poc != NULL)
    {
        scriba_freePOCData(poc);
    }
}

JNIEXPORT void JNICALL Java_org_scribacrm_libscriba_ScribaDB_removePOC(JNIEnv *env,
                                                                       jclass this,
                                                                       jlong id)
{
    scriba_removePOC((scriba_id_t)id);
}

JNIEXPORT jobject JNICALL Java_org_scribacrm_libscriba_ScribaDB_getProject(JNIEnv *env,
                                                                           jclass this,
                                                                           jlong id)
{
    jobject java_project = NULL;
    jclass project_class = NULL;
    jmethodID project_ctor_id = NULL;
    struct ScribaProject *project = scriba_getProject((scriba_id_t)id);
    jstring java_title = NULL;
    jstring java_descr = NULL;

    if (project == NULL)
    {
        goto exit;
    }

    project_class = (*env)->FindClass(env, "org/scribacrm/libscriba/Project");
    if (project_class == NULL)
    {
        goto exit;
    }

    project_ctor_id = (*env)->GetMethodID(env, project_class, "<init>",
                                       "(JLjava/lang/String;Ljava/lang/String;JB)V");
    if (project_ctor_id == NULL)
    {
        goto exit;
    }

    java_title = utf8_to_java_string(env, this, project->title);
    java_descr = utf8_to_java_string(env, this, project->descr);

    java_project = (*env)->NewObject(env, project_class, project_ctor_id,
                                     id, java_title, java_descr,
                                     (jlong)(project->company_id), (jbyte)(project->state));

exit:
    if (project != NULL)
    {
        scriba_freeProjectData(project);
    }
    return java_project;
}

JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getAllProjects(JNIEnv *env,
                                                                                    jclass this)
{
    jobjectArray java_projects = NULL;
    scriba_list_t *projects = scriba_getAllProjects();

    if (projects == NULL)
    {
        return NULL;
    }

    java_projects = scriba_list_to_data_descr_array(env, this, projects);
    scriba_list_delete(projects);
    return java_projects;
}

JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getProjectsByCompany(JNIEnv *env,
                                                                                          jclass this,
                                                                                          jlong id)
{
    jobjectArray java_projects = NULL;
    scriba_list_t *projects = scriba_getProjectsByCompany((scriba_id_t)id);

    if (projects == NULL)
    {
        return NULL;
    }

    java_projects = scriba_list_to_data_descr_array(env, this, projects);
    scriba_list_delete(projects);
    return java_projects;
}

JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getProjectsByState(JNIEnv *env,
                                                                                        jclass this,
                                                                                        jbyte state)
{
    jobjectArray java_projects = NULL;
    scriba_list_t *projects = scriba_getProjectsByState((enum ScribaProjectState)state);

    if (projects == NULL)
    {
        return NULL;
    }

    java_projects = scriba_list_to_data_descr_array(env, this, projects);
    scriba_list_delete(projects);
    return java_projects;
}

JNIEXPORT void JNICALL Java_org_scribacrm_libscriba_ScribaDB_addProject(JNIEnv *env,
                                                                        jclass this,
                                                                        jstring java_title,
                                                                        jstring java_descr,
                                                                        jlong company_id,
                                                                        jbyte state)
{
    char *native_title = java_string_to_utf8(env, this, java_title);
    char *native_descr = java_string_to_utf8(env, this, java_descr);

    scriba_addProject(native_title, native_descr, (scriba_id_t)company_id,
                      (enum ScribaProjectState)state);

    if (native_title != NULL)
    {
        free(native_title);
    }
    if (native_descr != NULL)
    {
        free(native_descr);
    }
}

JNIEXPORT void JNICALL Java_org_scribacrm_libscriba_ScribaDB_updateProject(JNIEnv *env,
                                                                           jclass this,
                                                                           jobject java_project)
{
    struct ScribaProject *project = NULL;
    jclass project_class = NULL;
    jfieldID fieldID = NULL;

    if (java_project == NULL)
    {
        goto exit;
    }

    project = (struct ScribaProject *)malloc(sizeof (struct ScribaProject));
    if (project == NULL)
    {
        goto exit;
    }
    memset(project, 0, sizeof (struct ScribaProject));

    project_class = (*env)->GetObjectClass(env, java_project);
    if (project_class == NULL)
    {
        goto exit;
    }

    // get fields of Project Java object and copy their values to ScribaProject structure
    fieldID = (*env)->GetFieldID(env, project_class, "id", "J");
    if (fieldID == NULL)
    {
        goto exit;
    }
    project->id = (scriba_id_t)((*env)->GetLongField(env, java_project, fieldID));

    fieldID = (*env)->GetFieldID(env, project_class, "title", "Ljava/lang/String;");
    if (fieldID == NULL)
    {
        goto exit;
    }
    project->title = java_string_to_utf8(env, this,
                                         (jstring)((*env)->GetObjectField(env, java_project, fieldID)));

    fieldID = (*env)->GetFieldID(env, project_class, "descr", "Ljava/lang/String;");
    if (fieldID == NULL)
    {
        goto exit;
    }
    project->descr = java_string_to_utf8(env, this,
                                         (jstring)((*env)->GetObjectField(env, java_project, fieldID)));

    fieldID = (*env)->GetFieldID(env, project_class, "company_id", "J");
    if (fieldID == NULL)
    {
        goto exit;
    }
    project->company_id = (scriba_id_t)((*env)->GetLongField(env, java_project, fieldID));

    fieldID = (*env)->GetFieldID(env, project_class, "state", "B");
    if (fieldID == NULL)
    {
        goto exit;
    }
    project->state = (enum ScribaProjectState)((*env)->GetByteField(env, java_project, fieldID));

    scriba_updateProject(project);

exit:
    if (project != NULL)
    {
        scriba_freeProjectData(project);
    }
}

JNIEXPORT void JNICALL Java_org_scribacrm_libscriba_ScribaDB_removeProject(JNIEnv *env,
                                                                           jclass this,
                                                                           jlong id)
{
    scriba_removeProject((scriba_id_t)id);
}

JNIEXPORT jobject JNICALL Java_org_scribacrm_libscriba_ScribaDB_getEvent(JNIEnv *env,
                                                                         jclass this,
                                                                         jlong id)
{
    jobject java_event = NULL;
    jclass event_class = NULL;
    jmethodID event_ctor_id = NULL;
    jstring java_descr = NULL;
    jstring java_outcome = NULL;
    struct ScribaEvent *event = scriba_getEvent((scriba_id_t)id);

    if (event == NULL)
    {
        goto exit;
    }

    event_class = (*env)->FindClass(env, "org/scribacrm/libscriba/Event");
    if (event_class == NULL)
    {
        goto exit;
    }

    event_ctor_id = (*env)->GetMethodID(env, event_class, "<init>",
                                     "(JLjava/lang/String;JJJBLjava/lang/String;JB)V");
    if (event_ctor_id == NULL)
    {
        goto exit;
    }

    java_descr = utf8_to_java_string(env, this, event->descr);
    java_outcome = utf8_to_java_string(env, this, event->outcome);

    java_event = (*env)->NewObject(env, event_class, event_ctor_id,
                                id, java_descr, (jlong)(event->company_id),
                                (jlong)(event->poc_id), (jlong)(event->project_id),
                                (jbyte)(event->type), java_outcome, (jlong)(event->timestamp),
                                (jbyte)(event->state));

exit:
    if (event != NULL)
    {
        scriba_freeEventData(event);
    }
    return java_event;
}

JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getAllEvents(JNIEnv *env, jclass this)
{
    jobjectArray java_events = NULL;
    scriba_list_t *events = scriba_getAllEvents();

    java_events = scriba_list_to_data_descr_array(env, this, events);
    scriba_list_delete(events);

    return java_events;
}

JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getEventsByCompany(JNIEnv *env,
                                                                                        jclass this,
                                                                                        jlong id)
{
    jobjectArray java_events = NULL;
    scriba_list_t *events = scriba_getEventsByCompany((scriba_id_t)id);

    if (events == NULL)
    {
        return NULL;
    }

    java_events = scriba_list_to_data_descr_array(env, this, events);
    scriba_list_delete(events);
    return java_events;
}

JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getEventsByPOC(JNIEnv *env,
                                                                                    jclass this,
                                                                                    jlong id)
{
    jobjectArray java_events = NULL;
    scriba_list_t *events = scriba_getEventsByPOC((scriba_id_t)id);

    if (events == NULL)
    {
        return NULL;
    }

    java_events = scriba_list_to_data_descr_array(env, this, events);
    scriba_list_delete(events);
    return java_events;
}

JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getEventsByProject(JNIEnv *env,
                                                                                        jclass this,
                                                                                        jlong id)
{
    jobjectArray java_events = NULL;
    scriba_list_t *events = scriba_getEventsByProject((scriba_id_t)id);

    if (events == NULL)
    {
        return NULL;
    }

    java_events = scriba_list_to_data_descr_array(env, this, events);
    scriba_list_delete(events);
    return java_events;
}

JNIEXPORT void JNICALL Java_org_scribacrm_libscriba_ScribaDB_addEvent(JNIEnv *env,
                                                                      jclass this,
                                                                      jstring descr,
                                                                      jlong company_id,
                                                                      jlong poc_id,
                                                                      jlong project_id,
                                                                      jbyte type,
                                                                      jstring outcome,
                                                                      jlong timestamp,
                                                                      jbyte state)
{
    char *native_descr = java_string_to_utf8(env, this, descr);
    char *native_outcome = java_string_to_utf8(env, this, outcome);

    scriba_addEvent(native_descr, (scriba_id_t)company_id, (scriba_id_t)poc_id,
                    (scriba_id_t)project_id, (enum ScribaEventType)type, native_outcome,
                    (scriba_time_t)timestamp, (enum ScribaEventState)state);

    if (native_descr != NULL)
    {
        free(native_descr);
    }
    if (native_outcome != NULL)
    {
        free(native_outcome);
    }
}

JNIEXPORT void JNICALL Java_org_scribacrm_libscriba_ScribaDB_updateEvent(JNIEnv *env,
                                                                         jclass this,
                                                                         jobject java_event)
{
    jclass event_class = NULL;
    jfieldID fieldID = NULL;
    struct ScribaEvent *event = NULL;

    if (java_event == NULL)
    {
        goto exit;
    }

    event = (struct ScribaEvent *)malloc(sizeof (struct ScribaEvent));
    if (event == NULL)
    {
        goto exit;
    }
    memset(event, 0, sizeof (struct ScribaEvent));

    event_class = (*env)->GetObjectClass(env, java_event);
    if (event_class == NULL)
    {
        goto exit;
    }

    // get fields of Event Java object and copy them to ScribaEvent structure
    
    fieldID = (*env)->GetFieldID(env, event_class, "id", "J");
    if (fieldID == NULL)
    {
        goto exit;
    }
    event->id = (scriba_id_t)((*env)->GetLongField(env, java_event, fieldID));

    fieldID = (*env)->GetFieldID(env, event_class, "descr", "Ljava/lang/String;");
    if (fieldID == NULL)
    {
        goto exit;
    }
    event->descr = java_string_to_utf8(env, this,
                                       (jstring)((*env)->GetObjectField(env, java_event, fieldID)));

    fieldID = (*env)->GetFieldID(env, event_class, "company_id", "J");
    if (fieldID == NULL)
    {
        goto exit;
    }
    event->company_id = (scriba_id_t)((*env)->GetLongField(env, java_event, fieldID));

    fieldID = (*env)->GetFieldID(env, event_class, "poc_id", "J");
    if (fieldID == NULL)
    {
        goto exit;
    }
    event->poc_id = (scriba_id_t)((*env)->GetLongField(env, java_event, fieldID));

    fieldID = (*env)->GetFieldID(env, event_class, "project_id", "J");
    if (fieldID == NULL)
    {
        goto exit;
    }
    event->project_id = (scriba_id_t)((*env)->GetLongField(env, java_event, fieldID));

    fieldID = (*env)->GetFieldID(env, event_class, "type", "B");
    if (fieldID == NULL)
    {
        goto exit;
    }
    event->type = (enum ScribaEventType)((*env)->GetByteField(env, java_event, fieldID));

    fieldID = (*env)->GetFieldID(env, event_class, "outcome", "Ljava/lang/String;");
    if (fieldID == NULL)
    {
        goto exit;
    }
    event->outcome = java_string_to_utf8(env, this,
                                         (jstring)((*env)->GetObjectField(env, java_event, fieldID)));

    fieldID = (*env)->GetFieldID(env, event_class, "timestamp", "J");
    if (fieldID == NULL)
    {
        goto exit;
    }
    event->timestamp = (scriba_time_t)((*env)->GetLongField(env, java_event, fieldID));

    fieldID = (*env)->GetFieldID(env, event_class, "state", "B");
    if (fieldID == NULL)
    {
        goto exit;
    }
    event->state = (enum ScribaEventState)((*env)->GetByteField(env, java_event, fieldID));

    scriba_updateEvent(event);

exit:
    if (event != NULL)
    {
        scriba_freeEventData(event);
    }
}

JNIEXPORT void JNICALL Java_org_scribacrm_libscriba_ScribaDB_removeEvent(JNIEnv *env,
                                                                         jclass this,
                                                                         jlong id)
{
    scriba_removeEvent((scriba_id_t)id);
}
