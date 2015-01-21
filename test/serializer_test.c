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

#include "serializer_test.h"
#include "common_test.h"
#include "sqlite_backend.h"
#include "serializer.h"
#include <stdlib.h>
#include <unistd.h>
#include <CUnit/CUnit.h>

#define TEST_DB_LOCATION "./serializer_test_sqlite_db"

static scriba_id_t company1_id;
static scriba_id_t company2_id;
static scriba_id_t poc1_id;
static scriba_id_t poc2_id;
static scriba_id_t project1_id;
static scriba_id_t project2_id;
static scriba_id_t event1_id;
static scriba_id_t event2_id;

// populate local DB with test data
static void create_test_data();
// verify data restored from buffer
static void verify_test_data();
// populate local DB with test data in Russian
static void create_ru_test_data();
// verify data in Russian restored from buffer
static void verify_ru_test_data();

int serializer_test_init()
{
    int ret = 0;

    scriba_id_zero_init(&company1_id);
    scriba_id_zero_init(&company2_id);
    scriba_id_zero_init(&poc1_id);
    scriba_id_zero_init(&poc2_id);
    scriba_id_zero_init(&project1_id);
    scriba_id_zero_init(&project2_id);
    scriba_id_zero_init(&event1_id);
    scriba_id_zero_init(&event2_id);

    struct ScribaDB db;
    db.name = SCRIBA_SQLITE_BACKEND_NAME;
    db.type = SCRIBA_DB_BUILTIN;
    db.location = NULL;

    struct ScribaDBParam param;
    param.key = SCRIBA_SQLITE_DB_LOCATION_PARAM;
    param.value = TEST_DB_LOCATION;

    struct ScribaDBParamList paramList;
    paramList.param = &param;
    paramList.next = NULL;

    if (scriba_init(&db, &paramList) != SCRIBA_INIT_SUCCESS)
    {
        ret = 1;
    }

    return ret;
}

int serializer_test_cleanup()
{
    scriba_cleanup();
    unlink(TEST_DB_LOCATION);

    return 0;
}

// general serializer test, verifies that serializer is able to save and
// restore data without corrupting it
void test_serializer()
{
    // populate local DB
    create_test_data();

    // export all entries to buffer
    scriba_list_t *companies = scriba_getAllCompanies();
    scriba_list_t *people = scriba_getAllPeople();
    scriba_list_t *projects = scriba_getAllProjects();
    scriba_list_t *events = scriba_getAllEvents();

    void *buf = NULL;
    unsigned long buflen = 0;
    buf = scriba_serialize(companies, events, people, projects, &buflen);

    scriba_list_delete(companies);
    scriba_list_delete(people);
    scriba_list_delete(projects);
    scriba_list_delete(events);

    // delete all local data
    clean_local_db();

    // restore data from the buffer
    enum ScribaMergeStatus status = scriba_deserialize(buf, buflen, SCRIBA_MERGE_REMOTE_OVERRIDE);
    CU_ASSERT_EQUAL(status, SCRIBA_MERGE_OK);

    // verify it
    verify_test_data();
    free(buf);

    // delete all local data
    clean_local_db();
}

// test serialization/deserialization of data in Russian
void test_serializer_ru()
{
    // populate local DB
    create_ru_test_data();

    // export all entries to buffer
    scriba_list_t *companies = scriba_getAllCompanies();
    scriba_list_t *people = scriba_getAllPeople();
    scriba_list_t *projects = scriba_getAllProjects();
    scriba_list_t *events = scriba_getAllEvents();

    void *buf = NULL;
    unsigned long buflen = 0;
    buf = scriba_serialize(companies, events, people, projects, &buflen);

    scriba_list_delete(companies);
    scriba_list_delete(people);
    scriba_list_delete(projects);
    scriba_list_delete(events);

    // delete all local data
    clean_local_db();

    // restore data from the buffer
    enum ScribaMergeStatus status = scriba_deserialize(buf, buflen, SCRIBA_MERGE_REMOTE_OVERRIDE);
    CU_ASSERT_EQUAL(status, SCRIBA_MERGE_OK);

    // verify it
    verify_ru_test_data();
    free(buf);

    // delete all local data
    clean_local_db();
}

// test remote override merge strategy
void test_serializer_remote_override()
{
    // populate local DB
    create_test_data();

    // export all entries to buffer
    scriba_list_t *companies = scriba_getAllCompanies();
    scriba_list_t *people = scriba_getAllPeople();
    scriba_list_t *projects = scriba_getAllProjects();
    scriba_list_t *events = scriba_getAllEvents();

    void *buf = NULL;
    unsigned long buflen = 0;
    buf = scriba_serialize(companies, events, people, projects, &buflen);

    // modify local data
    struct ScribaCompany *company = scriba_getCompany(companies->id);
    free(company->name);
    int len = strlen("Modified company name");
    company->name = (char *)malloc(len + 1);
    memset(company->name, 0, len + 1);
    strncpy(company->name, "Modified company name", len);
    scriba_updateCompany(company);
    scriba_freeCompanyData(company);

    // restore data from the buffer
    enum ScribaMergeStatus status = scriba_deserialize(buf, buflen, SCRIBA_MERGE_REMOTE_OVERRIDE);
    CU_ASSERT_EQUAL(status, SCRIBA_MERGE_OK);

    // verify that local modifications have been overriden by data from the buffer
    verify_test_data();
    free(buf);

    scriba_list_delete(companies);
    scriba_list_delete(people);
    scriba_list_delete(projects);
    scriba_list_delete(events);
}

// test local override merge strategy
void test_serializer_local_override()
{
    // populate local DB
    create_test_data();

    // export all entries to buffer
    scriba_list_t *companies = scriba_getAllCompanies();
    scriba_list_t *people = scriba_getAllPeople();
    scriba_list_t *projects = scriba_getAllProjects();
    scriba_list_t *events = scriba_getAllEvents();

    void *buf = NULL;
    unsigned long buflen = 0;
    buf = scriba_serialize(companies, events, people, projects, &buflen);

    scriba_list_delete(companies);
    scriba_list_delete(people);
    scriba_list_delete(projects);
    scriba_list_delete(events);

    // modify local data
    struct ScribaProject *project = scriba_getProject(project2_id);
    project->state = PROJECT_STATE_REJECTED;
    scriba_updateProject(project);
    scriba_freeProjectData(project);

    // restore data from the buffer
    enum ScribaMergeStatus status = scriba_deserialize(buf, buflen, SCRIBA_MERGE_LOCAL_OVERRIDE);
    CU_ASSERT_EQUAL(status, SCRIBA_MERGE_OK);
    
    // verify that project 2 modifications are kept after deserialization
    project = scriba_getProject(project2_id);
    CU_ASSERT_EQUAL(project->state, PROJECT_STATE_REJECTED);
    scriba_freeProjectData(project);

    free(buf);
}

// populate local DB with test data
static void create_test_data()
{
    scriba_id_create(&company1_id);
    scriba_id_create(&company2_id);
    scriba_id_create(&poc1_id);
    scriba_id_create(&poc2_id);
    scriba_id_create(&project1_id);
    scriba_id_create(&project2_id);
    scriba_id_create(&event1_id);
    scriba_id_create(&event2_id);

    scriba_addCompanyWithID(company1_id, "TestCompany1", "TestCompany1 LLC", "SomeAddress1",
                            scriba_inn_from_string("0123456789"), "111",
                            "testcompany@test1.com");
    scriba_addCompanyWithID(company2_id, "TestCompany2", "TestCompany2 LLC", "SomeAddress2",
                            scriba_inn_from_string("9876543210"), "222",
                            "testcompany@test2.com");

    scriba_addPOCWithID(poc1_id, "Moose", "Moosevich", "Moose", "333", "3333",
                        "moose@test1.com", "regular moose", company1_id);
    scriba_addPOCWithID(poc2_id, "Gustav", "Gustavovich", "Moose", "444", "4444",
                        "gustav@test2.com", "another regular moose", company2_id);


    scriba_addProjectWithID(project1_id, "TestProject1", "test project1", company1_id,
                            PROJECT_STATE_PAYMENT);
    scriba_addProjectWithID(project2_id, "TestProject2", "test project2", company2_id,
                            PROJECT_STATE_CLIENT_INFORMED);

    scriba_addEventWithID(event1_id, "Test event1", company1_id, poc1_id, project1_id,
                          EVENT_TYPE_CALL, "missed", (scriba_time_t)0, EVENT_STATE_CANCELLED);
    scriba_addEventWithID(event2_id, "Test event2", company2_id, poc2_id, project2_id,
                          EVENT_TYPE_TASK, "overdue", (scriba_time_t)0, EVENT_STATE_COMPLETED);
}


// verify data restored from buffer
static void verify_test_data()
{
    struct ScribaCompany *company1 = scriba_getCompany(company1_id);
    CU_ASSERT_PTR_NOT_NULL(company1);
    CU_ASSERT(scriba_id_compare(&company1_id, &(company1->id)));
    CU_ASSERT_STRING_EQUAL(company1->name, "TestCompany1");
    CU_ASSERT_STRING_EQUAL(company1->jur_name, "TestCompany1 LLC");
    CU_ASSERT_STRING_EQUAL(company1->address, "SomeAddress1");
    scriba_inn_t inn1 = scriba_inn_from_string("0123456789");
    CU_ASSERT(scriba_inn_is_equal(&(company1->inn), &inn1));
    CU_ASSERT_STRING_EQUAL(company1->phonenum, "111");
    CU_ASSERT_STRING_EQUAL(company1->email, "testcompany@test1.com");
    scriba_freeCompanyData(company1);

    struct ScribaCompany *company2 = scriba_getCompany(company2_id);
    CU_ASSERT_PTR_NOT_NULL(company2);
    CU_ASSERT(scriba_id_compare(&company2_id, &(company2->id)));
    CU_ASSERT_STRING_EQUAL(company2->name, "TestCompany2");
    CU_ASSERT_STRING_EQUAL(company2->jur_name, "TestCompany2 LLC");
    CU_ASSERT_STRING_EQUAL(company2->address, "SomeAddress2");
    scriba_inn_t inn2 = scriba_inn_from_string("9876543210");
    CU_ASSERT(scriba_inn_is_equal(&(company2->inn), &inn2));
    CU_ASSERT_STRING_EQUAL(company2->phonenum, "222");
    CU_ASSERT_STRING_EQUAL(company2->email, "testcompany@test2.com");
    scriba_freeCompanyData(company2);

    struct ScribaPoc *poc1 = scriba_getPOC(poc1_id);
    CU_ASSERT_PTR_NOT_NULL(poc1);
    CU_ASSERT(scriba_id_compare(&(poc1->id), &poc1_id));
    CU_ASSERT_STRING_EQUAL(poc1->firstname, "Moose");
    CU_ASSERT_STRING_EQUAL(poc1->secondname, "Moosevich");
    CU_ASSERT_STRING_EQUAL(poc1->lastname, "Moose");
    CU_ASSERT_STRING_EQUAL(poc1->mobilenum, "333");
    CU_ASSERT_STRING_EQUAL(poc1->phonenum, "3333");
    CU_ASSERT_STRING_EQUAL(poc1->email, "moose@test1.com");
    CU_ASSERT_STRING_EQUAL(poc1->position, "regular moose");
    CU_ASSERT(scriba_id_compare(&(poc1->company_id), &company1_id));
    scriba_freePOCData(poc1);

    struct ScribaPoc *poc2 = scriba_getPOC(poc2_id);
    CU_ASSERT_PTR_NOT_NULL(poc2);
    CU_ASSERT(scriba_id_compare(&(poc2->id), &poc2_id));
    CU_ASSERT_STRING_EQUAL(poc2->firstname, "Gustav");
    CU_ASSERT_STRING_EQUAL(poc2->secondname, "Gustavovich");
    CU_ASSERT_STRING_EQUAL(poc2->lastname, "Moose");
    CU_ASSERT_STRING_EQUAL(poc2->mobilenum, "444");
    CU_ASSERT_STRING_EQUAL(poc2->phonenum, "4444");
    CU_ASSERT_STRING_EQUAL(poc2->email, "gustav@test2.com");
    CU_ASSERT_STRING_EQUAL(poc2->position, "another regular moose");
    CU_ASSERT(scriba_id_compare(&(poc2->company_id), &company2_id));
    scriba_freePOCData(poc2);

    struct ScribaProject *project1 = scriba_getProject(project1_id);
    CU_ASSERT_PTR_NOT_NULL(project1);
    CU_ASSERT(scriba_id_compare(&(project1->id), &project1_id));
    CU_ASSERT_STRING_EQUAL(project1->title, "TestProject1");
    CU_ASSERT_STRING_EQUAL(project1->descr, "test project1");
    CU_ASSERT(scriba_id_compare(&(project1->company_id), &company1_id));
    CU_ASSERT_EQUAL(project1->state, PROJECT_STATE_PAYMENT);
    scriba_freeProjectData(project1);

    struct ScribaProject *project2 = scriba_getProject(project2_id);
    CU_ASSERT_PTR_NOT_NULL(project2);
    CU_ASSERT(scriba_id_compare(&(project2->id), &project2_id));
    CU_ASSERT_STRING_EQUAL(project2->title, "TestProject2");
    CU_ASSERT_STRING_EQUAL(project2->descr, "test project2");
    CU_ASSERT(scriba_id_compare(&(project2->company_id), &company2_id));
    CU_ASSERT_EQUAL(project2->state, PROJECT_STATE_CLIENT_INFORMED);
    scriba_freeProjectData(project2);

    struct ScribaEvent *event1 = scriba_getEvent(event1_id);
    CU_ASSERT_PTR_NOT_NULL(event1);
    CU_ASSERT(scriba_id_compare(&(event1->id), &event1_id));
    CU_ASSERT_STRING_EQUAL(event1->descr, "Test event1");
    CU_ASSERT(scriba_id_compare(&(event1->company_id), &company1_id));
    CU_ASSERT(scriba_id_compare(&(event1->poc_id), &poc1_id));
    CU_ASSERT(scriba_id_compare(&(event1->project_id), &project1_id));
    CU_ASSERT_EQUAL(event1->type, EVENT_TYPE_CALL);
    CU_ASSERT_STRING_EQUAL(event1->outcome, "missed");
    CU_ASSERT_EQUAL(event1->timestamp, (scriba_time_t)0);
    CU_ASSERT_EQUAL(event1->state, EVENT_STATE_CANCELLED);
    scriba_freeEventData(event1);

    struct ScribaEvent *event2 = scriba_getEvent(event2_id);
    CU_ASSERT_PTR_NOT_NULL(event2);
    CU_ASSERT(scriba_id_compare(&(event2->id), &event2_id));
    CU_ASSERT_STRING_EQUAL(event2->descr, "Test event2");
    CU_ASSERT(scriba_id_compare(&(event2->company_id), &company2_id));
    CU_ASSERT(scriba_id_compare(&(event2->poc_id), &poc2_id));
    CU_ASSERT(scriba_id_compare(&(event2->project_id), &project2_id));
    CU_ASSERT_EQUAL(event2->type, EVENT_TYPE_TASK);
    CU_ASSERT_STRING_EQUAL(event2->outcome, "overdue");
    CU_ASSERT_EQUAL(event2->timestamp, (scriba_time_t)0);
    CU_ASSERT_EQUAL(event2->state, EVENT_STATE_COMPLETED);
    scriba_freeEventData(event2);
}

// populate local DB with test data in Russian
static void create_ru_test_data()
{
    scriba_id_create(&company1_id);
    scriba_id_create(&company2_id);
    scriba_id_create(&poc1_id);
    scriba_id_create(&poc2_id);
    scriba_id_create(&project1_id);
    scriba_id_create(&project2_id);
    scriba_id_create(&event1_id);
    scriba_id_create(&event2_id);

    scriba_addCompanyWithID(company1_id, "Компания1", "ООО к1", "адрес1",
                            scriba_inn_from_string("0123456789"), "111",
                            "testcompany@test1.com");
    scriba_addCompanyWithID(company2_id, "Компания2", "ООО к2", "адрес2",
                            scriba_inn_from_string("9876543210"), "222",
                            "testcompany@test2.com");

    scriba_addPOCWithID(poc1_id, "Лось", "Лосевич", "Лосев", "333", "3333",
                        "moose@test1.com", "просто лось", company1_id);
    scriba_addPOCWithID(poc2_id, "Густав", "Густавович", "Лосев", "444", "4444",
                        "gustav@test2.com", "тоже просто лось", company2_id);


    scriba_addProjectWithID(project1_id, "Проект1", "тестовый проект 1", company1_id,
                            PROJECT_STATE_PAYMENT);
    scriba_addProjectWithID(project2_id, "Проект2", "тестовый проект 2", company2_id,
                            PROJECT_STATE_CLIENT_INFORMED);

    scriba_addEventWithID(event1_id, "Событие1", company1_id, poc1_id, project1_id,
                          EVENT_TYPE_CALL, "отменено", (scriba_time_t)0, EVENT_STATE_CANCELLED);
    scriba_addEventWithID(event2_id, "Событие2", company2_id, poc2_id, project2_id,
                          EVENT_TYPE_TASK, "пропущено", (scriba_time_t)0, EVENT_STATE_COMPLETED);
}

// verify data in Russian restored from buffer
static void verify_ru_test_data()
{
    struct ScribaCompany *company1 = scriba_getCompany(company1_id);
    CU_ASSERT_PTR_NOT_NULL(company1);
    CU_ASSERT(scriba_id_compare(&company1_id, &(company1->id)));
    CU_ASSERT_STRING_EQUAL(company1->name, "Компания1");
    CU_ASSERT_STRING_EQUAL(company1->jur_name, "ООО к1");
    CU_ASSERT_STRING_EQUAL(company1->address, "адрес1");
    scriba_inn_t inn1 = scriba_inn_from_string("0123456789");
    CU_ASSERT(scriba_inn_is_equal(&(company1->inn), &inn1));
    CU_ASSERT_STRING_EQUAL(company1->phonenum, "111");
    CU_ASSERT_STRING_EQUAL(company1->email, "testcompany@test1.com");
    scriba_freeCompanyData(company1);

    struct ScribaCompany *company2 = scriba_getCompany(company2_id);
    CU_ASSERT_PTR_NOT_NULL(company2);
    CU_ASSERT(scriba_id_compare(&company2_id, &(company2->id)));
    CU_ASSERT_STRING_EQUAL(company2->name, "Компания2");
    CU_ASSERT_STRING_EQUAL(company2->jur_name, "ООО к2");
    CU_ASSERT_STRING_EQUAL(company2->address, "адрес2");
    scriba_inn_t inn2 = scriba_inn_from_string("9876543210");
    CU_ASSERT(scriba_inn_is_equal(&(company2->inn), &inn2));
    CU_ASSERT_STRING_EQUAL(company2->phonenum, "222");
    CU_ASSERT_STRING_EQUAL(company2->email, "testcompany@test2.com");
    scriba_freeCompanyData(company2);

    struct ScribaPoc *poc1 = scriba_getPOC(poc1_id);
    CU_ASSERT_PTR_NOT_NULL(poc1);
    CU_ASSERT(scriba_id_compare(&(poc1->id), &poc1_id));
    CU_ASSERT_STRING_EQUAL(poc1->firstname, "Лось");
    CU_ASSERT_STRING_EQUAL(poc1->secondname, "Лосевич");
    CU_ASSERT_STRING_EQUAL(poc1->lastname, "Лосев");
    CU_ASSERT_STRING_EQUAL(poc1->mobilenum, "333");
    CU_ASSERT_STRING_EQUAL(poc1->phonenum, "3333");
    CU_ASSERT_STRING_EQUAL(poc1->email, "moose@test1.com");
    CU_ASSERT_STRING_EQUAL(poc1->position, "просто лось");
    CU_ASSERT(scriba_id_compare(&(poc1->company_id), &company1_id));
    scriba_freePOCData(poc1);

    struct ScribaPoc *poc2 = scriba_getPOC(poc2_id);
    CU_ASSERT_PTR_NOT_NULL(poc2);
    CU_ASSERT(scriba_id_compare(&(poc2->id), &poc2_id));
    CU_ASSERT_STRING_EQUAL(poc2->firstname, "Густав");
    CU_ASSERT_STRING_EQUAL(poc2->secondname, "Густавович");
    CU_ASSERT_STRING_EQUAL(poc2->lastname, "Лосев");
    CU_ASSERT_STRING_EQUAL(poc2->mobilenum, "444");
    CU_ASSERT_STRING_EQUAL(poc2->phonenum, "4444");
    CU_ASSERT_STRING_EQUAL(poc2->email, "gustav@test2.com");
    CU_ASSERT_STRING_EQUAL(poc2->position, "тоже просто лось");
    CU_ASSERT(scriba_id_compare(&(poc2->company_id), &company2_id));
    scriba_freePOCData(poc2);

    struct ScribaProject *project1 = scriba_getProject(project1_id);
    CU_ASSERT_PTR_NOT_NULL(project1);
    CU_ASSERT(scriba_id_compare(&(project1->id), &project1_id));
    CU_ASSERT_STRING_EQUAL(project1->title, "Проект1");
    CU_ASSERT_STRING_EQUAL(project1->descr, "тестовый проект 1");
    CU_ASSERT(scriba_id_compare(&(project1->company_id), &company1_id));
    CU_ASSERT_EQUAL(project1->state, PROJECT_STATE_PAYMENT);
    scriba_freeProjectData(project1);

    struct ScribaProject *project2 = scriba_getProject(project2_id);
    CU_ASSERT_PTR_NOT_NULL(project2);
    CU_ASSERT(scriba_id_compare(&(project2->id), &project2_id));
    CU_ASSERT_STRING_EQUAL(project2->title, "Проект2");
    CU_ASSERT_STRING_EQUAL(project2->descr, "тестовый проект 2");
    CU_ASSERT(scriba_id_compare(&(project2->company_id), &company2_id));
    CU_ASSERT_EQUAL(project2->state, PROJECT_STATE_CLIENT_INFORMED);
    scriba_freeProjectData(project2);

    struct ScribaEvent *event1 = scriba_getEvent(event1_id);
    CU_ASSERT_PTR_NOT_NULL(event1);
    CU_ASSERT(scriba_id_compare(&(event1->id), &event1_id));
    CU_ASSERT_STRING_EQUAL(event1->descr, "Событие1");
    CU_ASSERT(scriba_id_compare(&(event1->company_id), &company1_id));
    CU_ASSERT(scriba_id_compare(&(event1->poc_id), &poc1_id));
    CU_ASSERT(scriba_id_compare(&(event1->project_id), &project1_id));
    CU_ASSERT_EQUAL(event1->type, EVENT_TYPE_CALL);
    CU_ASSERT_STRING_EQUAL(event1->outcome, "отменено");
    CU_ASSERT_EQUAL(event1->timestamp, (scriba_time_t)0);
    CU_ASSERT_EQUAL(event1->state, EVENT_STATE_CANCELLED);
    scriba_freeEventData(event1);

    struct ScribaEvent *event2 = scriba_getEvent(event2_id);
    CU_ASSERT_PTR_NOT_NULL(event2);
    CU_ASSERT(scriba_id_compare(&(event2->id), &event2_id));
    CU_ASSERT_STRING_EQUAL(event2->descr, "Событие2");
    CU_ASSERT(scriba_id_compare(&(event2->company_id), &company2_id));
    CU_ASSERT(scriba_id_compare(&(event2->poc_id), &poc2_id));
    CU_ASSERT(scriba_id_compare(&(event2->project_id), &project2_id));
    CU_ASSERT_EQUAL(event2->type, EVENT_TYPE_TASK);
    CU_ASSERT_STRING_EQUAL(event2->outcome, "пропущено");
    CU_ASSERT_EQUAL(event2->timestamp, (scriba_time_t)0);
    CU_ASSERT_EQUAL(event2->state, EVENT_STATE_COMPLETED);
    scriba_freeEventData(event2);
}
