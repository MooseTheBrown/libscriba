/*
 * Copyright (C) 2015 Mikhail Sapozhnikov
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
#include "serializer.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// maximum size of array, which is passed to Java directly
#define MAX_ARRAY_SIZE  50
// invalid next id, must match DataDescriptor.NONEXT constant in Java code
#define NONEXT -1

/* Data array part passes large arrays of objects between native code
 * and Java in small chunks to avoid JNI local reference table overflow
 */
typedef struct
{
    long id;
    scriba_list_t *data;    // the whole native list
    scriba_list_t *part;    // points to the first element not yet passed to Java
} data_array_part;

// list of all data array parts
struct
{
    data_array_part *list;
    long list_size;
    // list of free array part ids (and free locations within array part list)
    long *free_ids;
    long num_free_ids;
    long allocated_free_ids;
    jobject sync_obj;
} part_list;

// local utility functions

// create new data array part, returns new part id
static long add_data_array_part(JNIEnv *env, scriba_list_t *data, scriba_list_t *part);
// find data array part
static data_array_part *get_data_array_part(JNIEnv *env, long id);
// remove data array part
static void remove_data_array_part(JNIEnv *env, long id);

// string conversion routines
static char *java_string_to_utf8(JNIEnv *env, jclass this, jstring str);
static jstring utf8_to_java_string(JNIEnv *env, jclass this, const char *utf8);

// convert scriba list to an array of DataDescriptor objects
static jobjectArray scriba_list_to_data_descr_array(JNIEnv *env, jclass this, scriba_list_t *list);
// convert array of DataDescriptor objects to scriba list
static scriba_list_t *data_descr_array_to_scriba_list(JNIEnv *env, jclass this,
                                                      jobjectArray data_descr_array);

// convert Java UUID object to scriba_id_t
static void UUID_to_scriba_id(JNIEnv *env, jobject uuid, scriba_id_t *id);

static long add_data_array_part(JNIEnv *env, scriba_list_t *data, scriba_list_t *part)
{
    data_array_part *new_part = NULL;

    (*env)->MonitorEnter(env, part_list.sync_obj);

    if (part_list.list == NULL)
    {
        // this will be the first element
        part_list.list = (data_array_part *)malloc(sizeof (data_array_part));
        part_list.list_size = 1;
        new_part = &(part_list.list[0]);
        new_part->id = part_list.list_size - 1;
    }
    else
    {
        // is there a free space in already allocated memory?
        if (part_list.num_free_ids > 0)
        {
            long new_id = part_list.free_ids[part_list.num_free_ids - 1];
            part_list.num_free_ids--;
            new_part = &(part_list.list[new_id]);
            new_part->id = new_id;
        }
        else
        {
            // no, we have to increase the part list
            part_list.list_size++;
            part_list.list = (data_array_part *)realloc(part_list.list,
                                                        sizeof (data_array_part) * part_list.list_size);
            new_part = &(part_list.list[part_list.list_size - 1]);
            new_part->id = part_list.list_size - 1;
        }
    }

    // copy the whole list
    new_part->data = scriba_list_init();
    scriba_list_for_each(data, item)
    {
        scriba_list_add(new_part->data, item->id, item->text);
    }
    scriba_list_for_each(new_part->data, new_item)
    {
        if (scriba_id_compare(&(new_item->id), &(part->id)))
        {
            new_part->part = new_item;
        }
    }

    (*env)->MonitorExit(env, part_list.sync_obj);

    return new_part->id;
}

static data_array_part *get_data_array_part(JNIEnv *env, long id)
{
    data_array_part *part = NULL;

    (*env)->MonitorEnter(env, part_list.sync_obj);

    if (id < part_list.list_size)
    {
        part = &(part_list.list[id]);
    }

    (*env)->MonitorExit(env, part_list.sync_obj);

    return part;
}

static void remove_data_array_part(JNIEnv *env, long id)
{
    (*env)->MonitorEnter(env, part_list.sync_obj);

    if (id < part_list.list_size)
    {
        data_array_part *part = &(part_list.list[id]);
        scriba_list_delete(part->data);
        part->data = NULL;
        part->part = NULL;
        // don't shrink the list, just mark given id as free
        if (part_list.free_ids == NULL)
        {
            // allocate space for free ids array
            part_list.free_ids = (long *)malloc(sizeof (long) * 10);
            part_list.allocated_free_ids = 10;
            part_list.num_free_ids = 1;
            part_list.free_ids[0] = id;
        }
        else
        {
            if (part_list.num_free_ids == part_list.allocated_free_ids)
            {
                // increase free ids array
                part_list.allocated_free_ids += 10;
                part_list.free_ids = (long *)realloc(part_list.free_ids,
                                                     part_list.allocated_free_ids);
            }
            part_list.num_free_ids++;
            part_list.free_ids[part_list.num_free_ids - 1] = id;
        }
    }

    (*env)->MonitorExit(env, part_list.sync_obj);
}

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
    (*env)->ReleaseByteArrayElements(env, byteArray, bytes, 0);

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
    char part = 0;
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
                                      "<init>", "(JJLjava/lang/String;)V");
    if (constructor_id == NULL)
    {
        goto exit;
    }

    scriba_list_for_each(list, item)
    {
        num_elements++;
    }

    if (num_elements == 0)
    {
        goto exit;
    }

    if (num_elements > MAX_ARRAY_SIZE)
    {
        num_elements = MAX_ARRAY_SIZE;
        part = 1;
    }

    java_array = (*env)->NewObjectArray(env, num_elements, data_descr_class, NULL);
    if (java_array == NULL)
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
        if ((i == (MAX_ARRAY_SIZE - 1)) && (part == 1))
        {
            long part_id = add_data_array_part(env, list, item);
            jmethodID part_constructor_id = (*env)->GetMethodID(env,
                                                                data_descr_class,
                                                                "<init>", "(J)V");
            if (part_constructor_id == NULL)
            {
                goto exit;
            }

            data_descriptors[i] = (*env)->NewObject(env, data_descr_class,
                                                    part_constructor_id, (jlong)part_id);
            if (data_descriptors[i] == NULL)
            {
                goto exit;
            }

            break;
        }
        else
        {
            jlong item_id_high = (jlong)(item->id._high);
            jlong item_id_low = (jlong)(item->id._low);
            jstring item_text = utf8_to_java_string(env, this, item->text);

            data_descriptors[i] = (*env)->NewObject(env, data_descr_class, constructor_id,
                                                    item_id_high, item_id_low, item_text);
            if (data_descriptors[i] == NULL)
            {
                goto exit;
            }

            i++;
        }
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

// convert array of DataDescriptor objects to scriba list
static scriba_list_t *data_descr_array_to_scriba_list(JNIEnv *env, jclass this,
                                                      jobjectArray data_descr_array)
{
    jsize array_length;
    scriba_list_t *scriba_list = scriba_list_init();
    jfieldID uuid_field_id;
    jfieldID next_field_id;
    jclass data_descr_class;

    data_descr_class = (*env)->FindClass(env, "org/scribacrm/libscriba/DataDescriptor");
    if (data_descr_class == NULL)
    {
        goto exit;
    }

    uuid_field_id = (*env)->GetFieldID(env,
                                       data_descr_class,
                                       "id",
                                       "Ljava/util/UUID;");
    if (uuid_field_id == NULL)
    {
        goto exit;
    }

    next_field_id = (*env)->GetFieldID(env,
                                       data_descr_class,
                                       "nextId",
                                       "J");
    if (next_field_id == NULL)
    {
        goto exit;
    }

    array_length = (*env)->GetArrayLength(env, data_descr_array);
    for (jsize i = 0; i < array_length; i++)
    {
        jobject data_descr = (*env)->GetObjectArrayElement(env, data_descr_array, i);
        jlong nextId = (*env)->GetLongField(env, data_descr, next_field_id);
        if (nextId != NONEXT)
        {
            data_array_part *part = get_data_array_part(env, nextId);
            if (part == NULL)
            {
                goto exit;
            }
            // copy the remaining part to the new list
            scriba_list_for_each(part->part, item)
            {
                scriba_list_add(scriba_list, item->id, NULL);
            }

            (*env)->DeleteLocalRef(env, data_descr);
            break;
            // don't remove data array part here, because Java code still
            // owns data descriptor array
        }
        else
        {
            jobject uuid = (*env)->GetObjectField(env, data_descr, uuid_field_id);
            scriba_id_t scriba_id;
            UUID_to_scriba_id(env, uuid, &scriba_id);
            scriba_list_add(scriba_list, scriba_id, NULL);
            /* For now we don't need to copy text description of each entry here
               because this function is currently used only by serializer, and it
               needs only ids in the lists. */
            (*env)->DeleteLocalRef(env, uuid);
            (*env)->DeleteLocalRef(env, data_descr);
        }
    }

exit:
    return scriba_list;
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
                                           "()J");
    uuid_get_low_id = (*env)->GetMethodID(env,
                                          uuid_class,
                                          "getLeastSignificantBits",
                                          "()J");

    if ((uuid_get_high_id == NULL) || (uuid_get_low_id == NULL))
    {
        return;
    }

    id_high = (*env)->CallLongMethod(env, uuid, uuid_get_high_id);
    id_low = (*env)->CallLongMethod(env, uuid, uuid_get_low_id);

    id->_high = (unsigned long long)id_high;
    id->_low = (unsigned long long)id_low;
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
    jclass syncObjClass = NULL;
    jmethodID syncObjConstructor = NULL;
    jobject localSyncObj = NULL;

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

    // initialize part_list
    part_list.list = NULL;
    part_list.list_size = 0;
    part_list.free_ids = NULL;
    part_list.num_free_ids = 0;
    part_list.allocated_free_ids = 0;
    syncObjClass = (*env)->FindClass(env, "java/lang/Integer");
    syncObjConstructor = (*env)->GetMethodID(env,
                                             syncObjClass,
                                             "<init>", "(I)V");
    localSyncObj = (*env)->NewObject(env, syncObjClass, syncObjConstructor, 1);
    part_list.sync_obj = (*env)->NewGlobalRef(env, localSyncObj);

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
    // part_list cleanup
    if (part_list.list != NULL)
    {
        for (long i = 0; i < part_list.list_size; i++)
        {
            data_array_part *part = &(part_list.list[i]);
            if (part->data != NULL)
            {
                scriba_list_delete(part->data);
            }
        }
        free(part_list.list);
    }
    part_list.list = NULL;
    part_list.list_size = 0;
    if (part_list.free_ids != NULL)
    {
        free(part_list.free_ids);
    }
    part_list.free_ids = 0;
    part_list.num_free_ids = 0;
    part_list.allocated_free_ids = 0;
    (*env)->DeleteGlobalRef(env, part_list.sync_obj);

    // library cleanup
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
    jmethodID company_ctor_id = NULL;
    scriba_id_t company_id;

    UUID_to_scriba_id(env, id, &company_id);
    company = scriba_getCompany(company_id);

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
    inn = utf8_to_java_string(env, this, company->inn);
    poc_list = scriba_list_to_data_descr_array(env, this, company->poc_list);
    proj_list = scriba_list_to_data_descr_array(env, this, company->proj_list);
    event_list = scriba_list_to_data_descr_array(env, this, company->event_list);

    company_ctor_id = (*env)->GetMethodID(env,
                                          company_class,
                                          "<init>",
                                          "(JJLjava/lang/String;"
                                          "Ljava/lang/String;Ljava/lang/String;"
                                          "Ljava/lang/String;Ljava/lang/String;"
                                          "Ljava/lang/String;"
                                          "[Lorg/scribacrm/libscriba/DataDescriptor;"
                                          "[Lorg/scribacrm/libscriba/DataDescriptor;"
                                          "[Lorg/scribacrm/libscriba/DataDescriptor;)V");
    if (company_ctor_id == NULL)
    {
        goto exit;
    }

    java_company = (*env)->NewObject(env, company_class, company_ctor_id,
                                     (jlong)company_id._high,
                                     (jlong)company_id._low,
                                     name, jur_name, address, inn, phonenum, email,
                                     poc_list, proj_list, event_list);

exit:
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
    char *inn = java_string_to_utf8(env, this, java_inn);

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
    if (inn != NULL)
    {
        free(inn);
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
    jobject company_id = NULL;

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

    fieldID = (*env)->GetFieldID(env, company_class, "id", "Ljava/util/UUID;");
    if (fieldID == NULL)
    {
        goto exit;
    }
    company_id = (*env)->GetObjectField(env, java_company, fieldID);
    UUID_to_scriba_id(env, company_id, &(company->id));

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
    company->inn = java_string_to_utf8(env, this,
                                       (jstring)((*env)->GetObjectField(env,
                                        java_company, fieldID)));

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
}

JNIEXPORT void JNICALL Java_org_scribacrm_libscriba_ScribaDB_removeCompany(JNIEnv *env,
                                                                           jclass this,
                                                                           jobject id)
{
    scriba_id_t company_id;

    UUID_to_scriba_id(env, id, &company_id);
    scriba_removeCompany(company_id);
}

JNIEXPORT jobject JNICALL Java_org_scribacrm_libscriba_ScribaDB_getPoc(JNIEnv *env,
                                                                       jclass this,
                                                                       jobject id)
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
    scriba_id_t poc_id;

    UUID_to_scriba_id(env, id, &poc_id);
    poc = scriba_getPOC(poc_id);
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
                                      "(JJLjava/lang/String;Ljava/lang/String;Ljava/lang/String;"
                                      "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
                                      "Ljava/lang/String;JJ)V");
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

    // create java POC object
    java_poc = (*env)->NewObject(env, poc_class, poc_ctor_id,
                                 poc_id._high, poc_id._low,
                                 firstname, secondname, lastname, mobilenum,
                                 phonenum, email, position,
                                 (jlong)(poc->company_id._high),
                                 (jlong)(poc->company_id._low));

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
                                                                                  jstring name)
{
    jobjectArray java_poc_list = NULL;
    scriba_list_t *poc_list = NULL;
    char *native_name = java_string_to_utf8(env, this, name);

    poc_list = scriba_getPOCByName(native_name);

    java_poc_list = scriba_list_to_data_descr_array(env, this, poc_list);
    
exit:
    if (native_name != NULL)
    {
        free(native_name);
    }
    scriba_list_delete(poc_list);
    return java_poc_list;
}

JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getPOCByCompany(JNIEnv *env,
                                                                                     jclass this,
                                                                                     jobject company_id)
{
    scriba_id_t native_company_id;
    scriba_list_t *poc_list = NULL;

    UUID_to_scriba_id(env, company_id, &native_company_id);
    poc_list = scriba_getPOCByCompany(native_company_id);
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
                                                                    jobject company_id)
{
    char *native_firstname = java_string_to_utf8(env, this, firstname);
    char *native_secondname = java_string_to_utf8(env, this, secondname);
    char *native_lastname = java_string_to_utf8(env, this, lastname);
    char *native_mobilenum = java_string_to_utf8(env, this, mobilenum);
    char *native_phonenum = java_string_to_utf8(env, this, phonenum);
    char *native_email = java_string_to_utf8(env, this, email);
    char *native_position = java_string_to_utf8(env, this, position);
    scriba_id_t native_company_id;

    UUID_to_scriba_id(env, company_id, &native_company_id);
    scriba_addPOC(native_firstname, native_secondname, native_lastname,
                  native_mobilenum, native_phonenum, native_email,
                  native_position, native_company_id);

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
    jobject poc_id = NULL;
    jobject company_id = NULL;

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
    
    fieldID = (*env)->GetFieldID(env, poc_class, "id", "Ljava/util/UUID;");
    if (fieldID == NULL)
    {
        goto exit;
    }
    poc_id = (*env)->GetObjectField(env, java_poc, fieldID);
    UUID_to_scriba_id(env, poc_id, &(poc->id));

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

    fieldID = (*env)->GetFieldID(env, poc_class, "company_id", "Ljava/util/UUID;");
    if (fieldID == NULL)
    {
        goto exit;
    }
    company_id = (*env)->GetObjectField(env, java_poc, fieldID);
    UUID_to_scriba_id(env, company_id, &(poc->company_id));

    scriba_updatePOC(poc);

exit:
    if (poc != NULL)
    {
        scriba_freePOCData(poc);
    }
}

JNIEXPORT void JNICALL Java_org_scribacrm_libscriba_ScribaDB_removePOC(JNIEnv *env,
                                                                       jclass this,
                                                                       jobject id)
{
    scriba_id_t poc_id;

    UUID_to_scriba_id(env, id, &poc_id);
    scriba_removePOC(poc_id);
}

JNIEXPORT jobject JNICALL Java_org_scribacrm_libscriba_ScribaDB_getProject(JNIEnv *env,
                                                                           jclass this,
                                                                           jobject id)
{
    jobject java_project = NULL;
    jclass project_class = NULL;
    jmethodID project_ctor_id = NULL;
    struct ScribaProject *project = NULL;
    jstring java_title = NULL;
    jstring java_descr = NULL;
    scriba_id_t project_id;

    UUID_to_scriba_id(env, id, &project_id);
    project = scriba_getProject(project_id);
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
                                          "(JJLjava/lang/String;Ljava/lang/String;JJB)V");
    if (project_ctor_id == NULL)
    {
        goto exit;
    }

    java_title = utf8_to_java_string(env, this, project->title);
    java_descr = utf8_to_java_string(env, this, project->descr);

    java_project = (*env)->NewObject(env, project_class, project_ctor_id,
                                     (jlong)(project_id._high), (jlong)(project_id._low),
                                     java_title, java_descr,
                                     (jlong)(project->company_id._high),
                                     (jlong)(project->company_id._low),
                                     (jbyte)(project->state));

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

JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getProjectsByTitle(JNIEnv *env,
                                                                                        jclass this,
                                                                                        jstring title)
{
    jobjectArray java_projects = NULL;
    scriba_list_t *projects = NULL;
    char *native_title = java_string_to_utf8(env, this, title);

    projects = scriba_getProjectsByTitle(native_title);
    if (projects == NULL)
    {
        goto exit;
    }

    java_projects = scriba_list_to_data_descr_array(env, this, projects);

exit:
    if (native_title != NULL)
    {
        free(native_title);
    }
    scriba_list_delete(projects);
    return java_projects;
}

JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getProjectsByCompany(JNIEnv *env,
                                                                                          jclass this,
                                                                                          jobject id)
{
    jobjectArray java_projects = NULL;
    scriba_id_t native_project_id;
    scriba_list_t *projects = NULL;

    UUID_to_scriba_id(env, id, &native_project_id);
    projects = scriba_getProjectsByCompany(native_project_id);
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
                                                                        jobject company_id,
                                                                        jbyte state)
{
    char *native_title = java_string_to_utf8(env, this, java_title);
    char *native_descr = java_string_to_utf8(env, this, java_descr);
    scriba_id_t native_company_id;

    UUID_to_scriba_id(env, company_id, &native_company_id);
    scriba_addProject(native_title, native_descr, native_company_id,
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
    jobject project_id = NULL;
    jobject company_id = NULL;

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
    fieldID = (*env)->GetFieldID(env, project_class, "id", "Ljava/util/UUID;");
    if (fieldID == NULL)
    {
        goto exit;
    }
    project_id = (*env)->GetObjectField(env, java_project, fieldID);
    UUID_to_scriba_id(env, project_id, &(project->id));

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

    fieldID = (*env)->GetFieldID(env, project_class, "company_id", "Ljava/util/UUID;");
    if (fieldID == NULL)
    {
        goto exit;
    }
    company_id = (*env)->GetObjectField(env, java_project, fieldID);
    UUID_to_scriba_id(env, company_id, &(project->company_id));

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
                                                                           jobject id)
{
    scriba_id_t project_id;

    UUID_to_scriba_id(env, id, &project_id);
    scriba_removeProject(project_id);
}

JNIEXPORT jobject JNICALL Java_org_scribacrm_libscriba_ScribaDB_getEvent(JNIEnv *env,
                                                                         jclass this,
                                                                         jobject id)
{
    jobject java_event = NULL;
    jclass event_class = NULL;
    jmethodID event_ctor_id = NULL;
    jstring java_descr = NULL;
    jstring java_outcome = NULL;
    struct ScribaEvent *event = NULL;
    scriba_id_t event_id;

    UUID_to_scriba_id(env, id, &event_id);
    event = scriba_getEvent(event_id);
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
                                        "(JJLjava/lang/String;JJJJJJBLjava/lang/String;JB)V");
    if (event_ctor_id == NULL)
    {
        goto exit;
    }

    java_descr = utf8_to_java_string(env, this, event->descr);
    java_outcome = utf8_to_java_string(env, this, event->outcome);

    java_event = (*env)->NewObject(env, event_class, event_ctor_id,
                                   (jlong)(event_id._high), (jlong)(event_id._low),
                                   java_descr,
                                   (jlong)(event->company_id._high),
                                   (jlong)(event->company_id._low),
                                   (jlong)(event->poc_id._high),
                                   (jlong)(event->poc_id._low),
                                   (jlong)(event->project_id._high),
                                   (jlong)(event->project_id._low),
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

JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getEventsByDescr(JNIEnv *env,
                                                                                      jclass this,
                                                                                      jstring descr)
{
    jobjectArray java_events = NULL;
    scriba_list_t *events = NULL;
    char *native_descr = java_string_to_utf8(env, this, descr);

    events = scriba_getEventsByDescr(native_descr);
    if (events == NULL)
    {
        goto exit;
    }

    java_events = scriba_list_to_data_descr_array(env, this, events);

exit:
    if (native_descr != NULL)
    {
        free(native_descr);
    }
    scriba_list_delete(events);
    return java_events;
}

JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getEventsByCompany(JNIEnv *env,
                                                                                        jclass this,
                                                                                        jobject id)
{
    jobjectArray java_events = NULL;
    scriba_id_t company_id;
    scriba_list_t *events = NULL;

    UUID_to_scriba_id(env, id, &company_id);
    events = scriba_getEventsByCompany(company_id);
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
                                                                                    jobject id)
{
    jobjectArray java_events = NULL;
    scriba_id_t poc_id;
    scriba_list_t *events = NULL;

    UUID_to_scriba_id(env, id, &poc_id);
    events = scriba_getEventsByPOC(poc_id);
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
                                                                                        jobject id)
{
    jobjectArray java_events = NULL;
    scriba_id_t project_id;
    scriba_list_t *events = NULL;

    UUID_to_scriba_id(env, id, &project_id);
    events = scriba_getEventsByProject(project_id);
    if (events == NULL)
    {
        return NULL;
    }

    java_events = scriba_list_to_data_descr_array(env, this, events);
    scriba_list_delete(events);
    return java_events;
}

JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_getEventsByState(JNIEnv *env,
                                                                                      jclass this,
                                                                                      jbyte state)
{
    jobjectArray java_events = NULL;
    scriba_list_t *events = NULL;

    events = scriba_getEventsByState((enum ScribaEventState)state);
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
                                                                      jobject company_id,
                                                                      jobject poc_id,
                                                                      jobject project_id,
                                                                      jbyte type,
                                                                      jstring outcome,
                                                                      jlong timestamp,
                                                                      jbyte state)
{
    char *native_descr = java_string_to_utf8(env, this, descr);
    char *native_outcome = java_string_to_utf8(env, this, outcome);
    scriba_id_t native_company_id;
    scriba_id_t native_poc_id;
    scriba_id_t native_project_id;

    UUID_to_scriba_id(env, company_id, &native_company_id);
    UUID_to_scriba_id(env, poc_id, &native_poc_id);
    UUID_to_scriba_id(env, project_id, &native_project_id);
    scriba_addEvent(native_descr, native_company_id, native_poc_id,
                    native_project_id, (enum ScribaEventType)type, native_outcome,
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
    jobject event_id = NULL;
    jobject company_id = NULL;
    jobject poc_id = NULL;
    jobject project_id = NULL;

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
    
    fieldID = (*env)->GetFieldID(env, event_class, "id", "Ljava/util/UUID;");
    if (fieldID == NULL)
    {
        goto exit;
    }
    event_id = (*env)->GetObjectField(env, java_event, fieldID);
    UUID_to_scriba_id(env, event_id, &(event->id));

    fieldID = (*env)->GetFieldID(env, event_class, "descr", "Ljava/lang/String;");
    if (fieldID == NULL)
    {
        goto exit;
    }
    event->descr = java_string_to_utf8(env, this,
                                       (jstring)((*env)->GetObjectField(env, java_event, fieldID)));

    fieldID = (*env)->GetFieldID(env, event_class, "company_id", "Ljava/util/UUID;");
    if (fieldID == NULL)
    {
        goto exit;
    }
    company_id = (*env)->GetObjectField(env, java_event, fieldID);
    UUID_to_scriba_id(env, company_id, &(event->company_id));

    fieldID = (*env)->GetFieldID(env, event_class, "poc_id", "Ljava/util/UUID;");
    if (fieldID == NULL)
    {
        goto exit;
    }
    poc_id = (*env)->GetObjectField(env, java_event, fieldID);
    UUID_to_scriba_id(env, poc_id, &(event->poc_id));

    fieldID = (*env)->GetFieldID(env, event_class, "project_id", "Ljava/util/UUID;");
    if (fieldID == NULL)
    {
        goto exit;
    }
    project_id = (*env)->GetObjectField(env, java_event, fieldID);
    UUID_to_scriba_id(env, project_id, &(event->project_id));

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
                                                                         jobject id)
{
    scriba_id_t event_id;

    UUID_to_scriba_id(env, id, &event_id);
    scriba_removeEvent(event_id);
}

JNIEXPORT jbyteArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_serialize(JNIEnv *env,
                                                                             jclass this,
                                                                             jobjectArray companies,
                                                                             jobjectArray events,
                                                                             jobjectArray people,
                                                                             jobjectArray projects)
{
    scriba_list_t *native_companies = data_descr_array_to_scriba_list(env, this, companies);
    scriba_list_t *native_events = data_descr_array_to_scriba_list(env, this, events);
    scriba_list_t *native_people = data_descr_array_to_scriba_list(env, this, people);
    scriba_list_t *native_projects = data_descr_array_to_scriba_list(env, this, projects);
    void *buf = NULL;
    unsigned long buflen = 0;
    jbyteArray java_array = NULL;

    buf = scriba_serialize(native_companies,
                           native_events,
                           native_people,
                           native_projects,
                           &buflen);
    if (buf == NULL)
    {
        goto exit;
    }

    java_array = (*env)->NewByteArray(env, buflen);
    if (java_array == NULL)
    {
        goto exit;
    }

    (*env)->SetByteArrayRegion(env, java_array, 0, buflen, (jbyte *)buf);

exit:
    if (native_companies != NULL)
    {
        scriba_list_delete(native_companies);
    }
    if (native_events != NULL)
    {
        scriba_list_delete(native_events);
    }
    if (native_people != NULL)
    {
        scriba_list_delete(native_people);
    }
    if (native_projects != NULL)
    {
        scriba_list_delete(native_projects);
    }
    if (buf != NULL)
    {
        free(buf);
    }

    return java_array;
}

JNIEXPORT jbyte JNICALL Java_org_scribacrm_libscriba_ScribaDB_deserialize(JNIEnv *env,
                                                                          jclass this,
                                                                          jbyteArray buf,
                                                                          jbyte mergeStrategy)
{
    jbyte status = 0;
    enum ScribaMergeStatus native_status;
    enum ScribaMergeStrategy native_strategy;
    jbyte *native_buf = (*env)->GetByteArrayElements(env, buf, NULL);
    jsize buflen = (*env)->GetArrayLength(env, buf);

    switch (mergeStrategy)
    {
    case 0:
        native_strategy = SCRIBA_MERGE_LOCAL_OVERRIDE;
        break;
    case 1:
        native_strategy = SCRIBA_MERGE_REMOTE_OVERRIDE;
        break;
    case 2:
        native_strategy = SCRIBA_MERGE_MANUAL;
        break;
    default:
        native_strategy = SCRIBA_MERGE_LOCAL_OVERRIDE;
        break;
    }

    native_status = scriba_deserialize((void *)native_buf,
                                       (unsigned long)buflen,
                                       native_strategy);

    if (native_status == SCRIBA_MERGE_OK)
    {
        status = 0;
    }
    else
    {
        status = 1;
    }

exit:
    if (native_buf != NULL)
    {
        (*env)->ReleaseByteArrayElements(env, buf, native_buf, JNI_ABORT);
    }

    return status;
}

JNIEXPORT jobjectArray JNICALL Java_org_scribacrm_libscriba_ScribaDB_next(JNIEnv *env,
                                                                          jclass this,
                                                                          jlong nextId)
{
    jobjectArray array = NULL;
    data_array_part *part = get_data_array_part(env, (long)nextId);
    if (part == NULL)
    {
        return NULL;
    }

    array = scriba_list_to_data_descr_array(env, this, part->part);
    // Array part is being transferred to Java, so remove it.
    // scriba_list_to_data_descr_array() may have already created new part
    remove_data_array_part(env, part->id);

    return array;
}
