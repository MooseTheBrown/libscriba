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

// Common test functions used by different test suites

#include "company.h"
#include "poc.h"
#include "project.h"
#include "event.h"
#include "common_test.h"
#include <CUnit/CUnit.h>
#include <stdlib.h>
#include <time.h>

void test_company()
{
    scriba_list_t *companies = NULL;
    struct ScribaCompany *company = NULL;
    struct ScribaCompany *company1 = NULL;

    // add the first company and verify it's been added
    scriba_addCompany("Test company #1", "Test1 LLC", "Test Address 1",
                      "012345678901", "111", "test1@test1.com");
    companies = scriba_getAllCompanies();
    CU_ASSERT_FALSE(scriba_list_is_empty(companies));
    CU_ASSERT(scriba_list_is_empty(companies->next));
    company = scriba_getCompany(companies->id);
    CU_ASSERT_PTR_NOT_NULL(company);
    CU_ASSERT(scriba_id_compare(&(company->id), &(companies->id)));
    CU_ASSERT_STRING_EQUAL(company->name, "Test company #1");
    CU_ASSERT_STRING_EQUAL(company->jur_name, "Test1 LLC");
    CU_ASSERT_STRING_EQUAL(company->address, "Test Address 1");
    CU_ASSERT_STRING_EQUAL(company->inn, "012345678901");
    CU_ASSERT_STRING_EQUAL(company->phonenum, "111");
    CU_ASSERT_STRING_EQUAL(company->email, "test1@test1.com");
    CU_ASSERT(scriba_list_is_empty(company->poc_list));
    CU_ASSERT(scriba_list_is_empty(company->proj_list));
    CU_ASSERT(scriba_list_is_empty(company->event_list));

    scriba_list_delete(companies);
    scriba_freeCompanyData(company);
    company = NULL;

    // add the second company and verify it's been added
    scriba_addCompany("Test company #2", "Test2 LLC", "Test Address 2",
                      "987654321055", "222", "test2@test2.com");
    companies = scriba_getAllCompanies();
    CU_ASSERT_FALSE(scriba_list_is_empty(companies));
    CU_ASSERT_FALSE(scriba_list_is_empty(companies->next));
    CU_ASSERT(scriba_list_is_empty(companies->next->next));
    company = scriba_getCompany(companies->next->id);
    CU_ASSERT_PTR_NOT_NULL(company);
    CU_ASSERT(scriba_id_compare(&(company->id), &(companies->next->id)));
    CU_ASSERT_STRING_EQUAL(company->name, "Test company #2");
    CU_ASSERT_STRING_EQUAL(company->jur_name, "Test2 LLC");
    CU_ASSERT_STRING_EQUAL(company->address, "Test Address 2");
    CU_ASSERT_STRING_EQUAL(company->inn, "987654321055");
    CU_ASSERT_STRING_EQUAL(company->phonenum, "222");
    CU_ASSERT_STRING_EQUAL(company->email, "test2@test2.com");
    CU_ASSERT(scriba_list_is_empty(company->poc_list));
    CU_ASSERT(scriba_list_is_empty(company->proj_list));
    CU_ASSERT(scriba_list_is_empty(company->event_list));

    scriba_list_delete(companies);
    scriba_freeCompanyData(company);
    company = NULL;

    // verify company search functions
    companies = scriba_getCompaniesByName("Test company #1");
    CU_ASSERT_FALSE(scriba_list_is_empty(companies));
    CU_ASSERT(scriba_list_is_empty(companies->next));
    company = scriba_getCompany(companies->id);
    CU_ASSERT_PTR_NOT_NULL(company);
    CU_ASSERT(scriba_id_compare(&(company->id), &(companies->id)));
    CU_ASSERT_STRING_EQUAL(company->name, "Test company #1");

    scriba_list_delete(companies);
    scriba_freeCompanyData(company);
    company = NULL;

    companies = scriba_getCompaniesByJurName("Test2 LLC");
    CU_ASSERT_FALSE(scriba_list_is_empty(companies));
    CU_ASSERT(scriba_list_is_empty(companies->next));
    company = scriba_getCompany(companies->id);
    CU_ASSERT_PTR_NOT_NULL(company);
    CU_ASSERT(scriba_id_compare(&(company->id), &(companies->id)));
    CU_ASSERT_STRING_EQUAL(company->name, "Test company #2");

    scriba_list_delete(companies);
    scriba_freeCompanyData(company);
    company = NULL;

    companies = scriba_getCompaniesByAddress("Test Address 1");
    CU_ASSERT_FALSE(scriba_list_is_empty(companies));
    CU_ASSERT(scriba_list_is_empty(companies->next));
    company = scriba_getCompany(companies->id);
    CU_ASSERT_PTR_NOT_NULL(company);
    CU_ASSERT(scriba_id_compare(&(company->id), &(companies->id)));
    CU_ASSERT_STRING_EQUAL(company->name, "Test company #1");

    scriba_list_delete(companies);

    // create a copy of the first company's data structure
    company1 = scriba_copyCompany(company);
    CU_ASSERT_PTR_NOT_NULL(company1);
    CU_ASSERT_NOT_EQUAL(company1, company);
    CU_ASSERT(scriba_id_compare(&(company1->id), &(company->id)));
    CU_ASSERT_STRING_EQUAL(company->name, "Test company #1");
    CU_ASSERT_STRING_EQUAL(company->jur_name, "Test1 LLC");
    CU_ASSERT_STRING_EQUAL(company->address, "Test Address 1");
    CU_ASSERT_STRING_EQUAL(company->inn, "012345678901");
    CU_ASSERT_STRING_EQUAL(company->phonenum, "111");
    CU_ASSERT_STRING_EQUAL(company->email, "test1@test1.com");
    CU_ASSERT(scriba_list_is_empty(company->poc_list));
    CU_ASSERT(scriba_list_is_empty(company->proj_list));
    CU_ASSERT(scriba_list_is_empty(company->event_list));

    scriba_freeCompanyData(company);
    company = NULL;

    // modify the first company and verify it's been modified correctly
    free(company1->address);
    company1->address = (char *)malloc(strlen("New Test Address 1") + 1);
    strcpy(company1->address, "New Test Address 1");
    scriba_updateCompany(company1);

    company = scriba_getCompany(company1->id);
    CU_ASSERT_PTR_NOT_NULL(company);
    CU_ASSERT_STRING_EQUAL(company->name, "Test company #1");
    CU_ASSERT_STRING_EQUAL(company->jur_name, "Test1 LLC");
    CU_ASSERT_STRING_EQUAL(company->address, "New Test Address 1");
    CU_ASSERT_STRING_EQUAL(company->inn, "012345678901");
    CU_ASSERT_STRING_EQUAL(company->phonenum, "111");
    CU_ASSERT_STRING_EQUAL(company->email, "test1@test1.com");
    CU_ASSERT(scriba_list_is_empty(company->poc_list));
    CU_ASSERT(scriba_list_is_empty(company->proj_list));
    CU_ASSERT(scriba_list_is_empty(company->event_list));

    scriba_freeCompanyData(company);
    company = NULL;

    // remove the first company and verify that only the second is left
    scriba_removeCompany(company1->id);
    company = scriba_getCompany(company1->id);
    CU_ASSERT_PTR_NULL(company);
    companies = scriba_getAllCompanies();
    CU_ASSERT_FALSE(scriba_list_is_empty(companies));
    CU_ASSERT(scriba_list_is_empty(companies->next));
    company = scriba_getCompany(companies->id);
    CU_ASSERT_PTR_NOT_NULL(company);
    CU_ASSERT(scriba_id_compare(&(company->id), &(companies->id)));
    CU_ASSERT_STRING_EQUAL(company->name, "Test company #2");

    scriba_list_delete(companies);
    scriba_freeCompanyData(company);
    scriba_freeCompanyData(company1);

    clean_local_db();
}

void test_poc()
{
    scriba_list_t *people = NULL;
    scriba_list_t *companies = NULL;
    struct ScribaPoc *poc1 = NULL;
    struct ScribaPoc *poc2 = NULL;
    struct ScribaPoc *poc3 = NULL;
    struct ScribaPoc *poc4 = NULL;
    struct ScribaCompany *company = NULL;
    int count = 0;

    // add test companies
    scriba_addCompany("Test company #1", "Test1 LLC", "Test Address 1",
                      "12345", "111", "test1@test1.com");
    scriba_addCompany("Test company #2", "Test2 LLC", "Test Address 2",
                      "54321", "222", "test2@test2.com");

    companies = scriba_getAllCompanies();

    // add test people and verify that they are added properly
    scriba_addPOC("Mikhail", "Alekseevich", "Sapozhnikov",
                  "1111", "2222", "mikhail@test1.com",
                  "SW Engineer", companies->id);
    people = scriba_getPOCByName("Mikhail");
    CU_ASSERT_FALSE(scriba_list_is_empty(people));
    CU_ASSERT(scriba_list_is_empty(people->next));
    poc1 = scriba_getPOC(people->id);
    CU_ASSERT_PTR_NOT_NULL(poc1);
    CU_ASSERT(scriba_id_compare(&(poc1->id), &(people->id)));
    CU_ASSERT_STRING_EQUAL(poc1->firstname, "Mikhail");
    CU_ASSERT_STRING_EQUAL(poc1->secondname, "Alekseevich");
    CU_ASSERT_STRING_EQUAL(poc1->lastname, "Sapozhnikov");
    CU_ASSERT_STRING_EQUAL(poc1->mobilenum, "1111");
    CU_ASSERT_STRING_EQUAL(poc1->phonenum, "2222");
    CU_ASSERT_STRING_EQUAL(poc1->email, "mikhail@test1.com");
    CU_ASSERT_STRING_EQUAL(poc1->position, "SW Engineer");
    CU_ASSERT(scriba_id_compare(&(poc1->company_id), &(companies->id)));

    scriba_list_delete(people);

    scriba_addPOC("Alexey", "Vladimirovich", "Sapozhnikov",
                  "3333", "4444", "alexey@test2.com",
                  "Maritime Inspector", companies->next->id);
    people = scriba_getPOCByName("Alexey");
    CU_ASSERT_FALSE(scriba_list_is_empty(people));
    CU_ASSERT(scriba_list_is_empty(people->next));
    poc2 = scriba_getPOC(people->id);
    CU_ASSERT_PTR_NOT_NULL(poc2);
    CU_ASSERT(scriba_id_compare(&(poc2->id), &(people->id)));
    CU_ASSERT_STRING_EQUAL(poc2->firstname, "Alexey");
    CU_ASSERT_STRING_EQUAL(poc2->secondname, "Vladimirovich");
    CU_ASSERT_STRING_EQUAL(poc2->lastname, "Sapozhnikov");
    CU_ASSERT_STRING_EQUAL(poc2->mobilenum, "3333");
    CU_ASSERT_STRING_EQUAL(poc2->phonenum, "4444");
    CU_ASSERT_STRING_EQUAL(poc2->email, "alexey@test2.com");
    CU_ASSERT_STRING_EQUAL(poc2->position, "Maritime Inspector");
    CU_ASSERT(scriba_id_compare(&(poc2->company_id), &(companies->next->id)));

    scriba_list_delete(people);

    scriba_addPOC("Elena", "Yurievna", "Sapozhnikova",
                  "5555", "6666", "elena@test2.com",
                  "Accountant", companies->next->id);
    people = scriba_getPOCByName("Yurievna");
    CU_ASSERT_FALSE(scriba_list_is_empty(people));
    CU_ASSERT(scriba_list_is_empty(people->next));
    poc3 = scriba_getPOC(people->id);
    CU_ASSERT_PTR_NOT_NULL(poc3);
    CU_ASSERT(scriba_id_compare(&(poc3->id), &(people->id)));
    CU_ASSERT_STRING_EQUAL(poc3->firstname, "Elena");
    CU_ASSERT_STRING_EQUAL(poc3->secondname, "Yurievna");
    CU_ASSERT_STRING_EQUAL(poc3->lastname, "Sapozhnikova");
    CU_ASSERT_STRING_EQUAL(poc3->mobilenum, "5555");
    CU_ASSERT_STRING_EQUAL(poc3->phonenum, "6666");
    CU_ASSERT_STRING_EQUAL(poc3->email, "elena@test2.com");
    CU_ASSERT_STRING_EQUAL(poc3->position, "Accountant");
    CU_ASSERT(scriba_id_compare(&(poc3->company_id), &(companies->next->id)));

    scriba_list_delete(people);

    // check if all people are retrieved correctly
    people = scriba_getAllPeople();
    CU_ASSERT_FALSE(scriba_list_is_empty(people));
    scriba_list_for_each(people, person)
    {
        count++;
    }
    // there must be 3 persons
    CU_ASSERT_EQUAL(3, count);

    scriba_list_delete(people);

    people = scriba_getPOCByCompany(companies->next->id);
    CU_ASSERT_FALSE(scriba_list_is_empty(people));
    CU_ASSERT_FALSE(scriba_list_is_empty(people->next));
    CU_ASSERT(scriba_id_compare(&(people->id), &(poc2->id)));
    CU_ASSERT(scriba_id_compare(&(people->next->id), &(poc3->id)));

    scriba_list_delete(people);

    people = scriba_getPOCByPosition("SW Engineer");
    CU_ASSERT_FALSE(scriba_list_is_empty(people));
    CU_ASSERT(scriba_list_is_empty(people->next));
    CU_ASSERT(scriba_id_compare(&(people->id), &(poc1->id)));

    scriba_list_delete(people);

    people = scriba_getPOCByEmail("elena@test2.com");
    CU_ASSERT_FALSE(scriba_list_is_empty(people));
    CU_ASSERT(scriba_list_is_empty(people->next));
    CU_ASSERT(scriba_id_compare(&(people->id), &(poc3->id)));

    scriba_list_delete(people);

    // update the third POC
    free(poc3->firstname);
    free(poc3->secondname);
    free(poc3->position);
    free(poc3->email);
    poc3->firstname = (char *)malloc(strlen("Ksenia") + 1);
    memset(poc3->firstname, 0, strlen("Ksenia") + 1);
    strcpy(poc3->firstname, "Ksenia");
    poc3->secondname = (char *)malloc(strlen("Nikolayevna") + 1);
    memset(poc3->secondname, 0, strlen("Nikolayevna") + 1);
    strcpy(poc3->secondname, "Nikolayevna");
    poc3->position = (char *)malloc(strlen("Logistics specialist") + 1);
    memset(poc3->position, 0, strlen("Logistics specialist") + 1);
    strcpy(poc3->position, "Logistics specialist");
    poc3->email = (char *)malloc(strlen("ksenia@test2.com") + 1);
    memset(poc3->email, 0, strlen("ksenia@test2.com") + 1);
    strcpy(poc3->email, "ksenia@test2.com");
    scriba_updatePOC(poc3);
    // verify that it is updated correctly
    poc4 = scriba_getPOC(poc3->id);
    CU_ASSERT_PTR_NOT_NULL(poc4);
    CU_ASSERT_STRING_EQUAL(poc4->firstname, "Ksenia");
    CU_ASSERT_STRING_EQUAL(poc4->secondname, "Nikolayevna");
    CU_ASSERT_STRING_EQUAL(poc4->lastname, "Sapozhnikova");
    CU_ASSERT_STRING_EQUAL(poc4->mobilenum, "5555");
    CU_ASSERT_STRING_EQUAL(poc4->phonenum, "6666");
    CU_ASSERT_STRING_EQUAL(poc4->email, "ksenia@test2.com");
    CU_ASSERT_STRING_EQUAL(poc4->position, "Logistics specialist");
    CU_ASSERT(scriba_id_compare(&(poc4->company_id), &(companies->next->id)));

    scriba_freePOCData(poc3);
    poc3 = NULL;

    // copy POC data and verify
    poc3 = scriba_copyPOC(poc4);
    CU_ASSERT_PTR_NOT_NULL(poc3);
    CU_ASSERT_NOT_EQUAL(poc3, poc4);
    CU_ASSERT(scriba_id_compare(&(poc3->id), &(poc4->id)));
    CU_ASSERT_STRING_EQUAL(poc3->firstname, "Ksenia");
    CU_ASSERT_STRING_EQUAL(poc3->secondname, "Nikolayevna");
    CU_ASSERT_STRING_EQUAL(poc3->lastname, "Sapozhnikova");
    CU_ASSERT_STRING_EQUAL(poc3->mobilenum, "5555");
    CU_ASSERT_STRING_EQUAL(poc3->phonenum, "6666");
    CU_ASSERT_STRING_EQUAL(poc3->email, "ksenia@test2.com");
    CU_ASSERT_STRING_EQUAL(poc3->position, "Logistics specialist");
    CU_ASSERT(scriba_id_compare(&(poc3->company_id), &(companies->next->id)));

    // check companies' POC lists
    company = scriba_getCompany(companies->id);
    CU_ASSERT_FALSE(scriba_list_is_empty(company->poc_list));
    CU_ASSERT(scriba_list_is_empty(company->poc_list->next));
    CU_ASSERT(scriba_id_compare(&(company->poc_list->id), &(poc1->id)));
    scriba_freeCompanyData(company);

    company = scriba_getCompany(companies->next->id);
    CU_ASSERT_FALSE(scriba_list_is_empty(company->poc_list));
    CU_ASSERT_FALSE(scriba_list_is_empty(company->poc_list->next));
    CU_ASSERT(scriba_id_compare(&(company->poc_list->id), &(poc2->id)));
    CU_ASSERT(scriba_id_compare(&(company->poc_list->next->id), &(poc4->id)));
    scriba_freeCompanyData(company);

    // remove one poc and verify that it is indeed removed
    scriba_removePOC(poc1->id);
    people = NULL;
    people = scriba_getPOCByName("Mikhail");
    CU_ASSERT(scriba_list_is_empty(people));
    scriba_list_delete(people);

    scriba_list_delete(companies);
    scriba_freePOCData(poc1);
    scriba_freePOCData(poc2);
    scriba_freePOCData(poc3);
    scriba_freePOCData(poc4);

    clean_local_db();
}

void test_project()
{
    scriba_list_t *companies = NULL;
    scriba_list_t *projects = NULL;
    struct ScribaProject *project1 = NULL;
    struct ScribaProject *project2 = NULL;
    struct ScribaProject *project3 = NULL;
    struct ScribaCompany *company = NULL;

    // add test company
    scriba_addCompany("Test company #1", "Test1 LLC", "Test Address 1",
                      "12345", "111", "test1@test1.com");

    companies = scriba_getAllCompanies();

    // add test project and verify that it's been added correctly
    scriba_addProject("Test Project #1", "100 bottles of whisky",
                      companies->id, PROJECT_STATE_CONTRACT_SIGNED,
                      SCRIBA_CURRENCY_RUB, 1000);

    projects = scriba_getAllProjects();
    CU_ASSERT_FALSE(scriba_list_is_empty(projects));
    CU_ASSERT(scriba_list_is_empty(projects->next));

    project1 = scriba_getProject(projects->id);
    CU_ASSERT_PTR_NOT_NULL(project1);
    CU_ASSERT(scriba_id_compare(&(project1->id), &(projects->id)));
    CU_ASSERT_STRING_EQUAL(project1->title, "Test Project #1");
    CU_ASSERT_STRING_EQUAL(project1->descr, "100 bottles of whisky");
    CU_ASSERT(scriba_id_compare(&(project1->company_id), &(companies->id)));
    CU_ASSERT_EQUAL(project1->state, PROJECT_STATE_CONTRACT_SIGNED);
    CU_ASSERT_EQUAL(project1->currency, SCRIBA_CURRENCY_RUB);
    CU_ASSERT_EQUAL(project1->cost, 1000);

    scriba_list_delete(projects);

    // add another project
    scriba_addProject("Test Project #2", "1000 bottles of milk",
                      companies->id, PROJECT_STATE_REJECTED,
                      SCRIBA_CURRENCY_USD, 300);

    projects = scriba_getProjectsByCompany(companies->id);
    CU_ASSERT_FALSE(scriba_list_is_empty(projects));
    CU_ASSERT_FALSE(scriba_list_is_empty(projects->next));

    project2 = scriba_getProject(projects->next->id);
    CU_ASSERT_PTR_NOT_NULL(project2);

    CU_ASSERT(scriba_id_compare(&(projects->id), &(project1->id)));
    CU_ASSERT(scriba_id_compare(&(projects->next->id), &(project2->id)));

    scriba_list_delete(projects);

    projects = scriba_getProjectsByState(PROJECT_STATE_REJECTED);
    CU_ASSERT_FALSE(scriba_list_is_empty(projects));
    CU_ASSERT(scriba_list_is_empty(projects->next));
    CU_ASSERT(scriba_id_compare(&(projects->id), &(project2->id)));

    scriba_list_delete(projects);

    // update project and verify
    projects = scriba_getAllProjects();
    project2->state = PROJECT_STATE_OFFER;
    scriba_updateProject(project2);
    scriba_freeProjectData(project2);
    project2 = scriba_getProject(projects->next->id);
    CU_ASSERT_PTR_NOT_NULL(project2);
    CU_ASSERT(scriba_id_compare(&(project2->id), &(projects->next->id)));
    CU_ASSERT_STRING_EQUAL(project2->title, "Test Project #2");
    CU_ASSERT_STRING_EQUAL(project2->descr, "1000 bottles of milk");
    CU_ASSERT(scriba_id_compare(&(project2->company_id), &(companies->id)));
    CU_ASSERT_EQUAL(project2->state, PROJECT_STATE_OFFER);

    scriba_list_delete(projects);

    // copy project data and verify
    project3 = scriba_copyProject(project1);
    CU_ASSERT_PTR_NOT_NULL(project3);
    CU_ASSERT_NOT_EQUAL(project3, project1);
    CU_ASSERT(scriba_id_compare(&(project3->id), &(project1->id)));
    CU_ASSERT_STRING_EQUAL(project3->title, "Test Project #1");
    CU_ASSERT_STRING_EQUAL(project3->descr, "100 bottles of whisky");
    CU_ASSERT(scriba_id_compare(&(project3->company_id), &(companies->id)));
    CU_ASSERT_EQUAL(project3->state, PROJECT_STATE_CONTRACT_SIGNED);
    CU_ASSERT_EQUAL(project3->currency, SCRIBA_CURRENCY_RUB);
    CU_ASSERT_EQUAL(project3->cost, 1000);
    scriba_freeProjectData(project3);
    project3 = NULL;

    // check company project list
    company = scriba_getCompany(companies->id);
    CU_ASSERT_FALSE(scriba_list_is_empty(company->proj_list));
    CU_ASSERT_FALSE(scriba_list_is_empty(company->proj_list->next));
    CU_ASSERT(scriba_id_compare(&(company->proj_list->id), &(project1->id)));
    CU_ASSERT(scriba_id_compare(&(company->proj_list->next->id), &(project2->id)));
    scriba_freeCompanyData(company);

    // remove the first project and verify
    scriba_removeProject(project1->id);
    project3 = scriba_getProject(project1->id);
    CU_ASSERT_PTR_NULL(project3);

    scriba_list_delete(companies);

    scriba_freeProjectData(project1);
    scriba_freeProjectData(project2);

    clean_local_db();
}

void test_event()
{
    scriba_list_t *events = NULL;
    scriba_list_t *companies = NULL;
    scriba_list_t *people1 = NULL;
    scriba_list_t *people2 = NULL;
    scriba_list_t *projects = NULL;
    struct ScribaEvent *event1 = NULL;
    struct ScribaEvent *event2 = NULL;
    struct ScribaEvent *event3 = NULL;
    struct ScribaEvent *event4 = NULL;
    struct ScribaCompany *company = NULL;
    int count = 0;
    scriba_time_t cur_time = time(NULL);

    // add test data - companies, people, projects
    scriba_addCompany("Test company #1", "Test1 LLC", "Test Address 1",
                      "12345", "111", "test1@test1.com");
    scriba_addCompany("Test company #2", "Test2 LLC", "Test Address 2",
                      "54321", "222", "test2@test2.com");

    companies = scriba_getAllCompanies();

    scriba_addPOC("Mikhail", "Alekseevich", "Sapozhnikov",
                  "1111", "2222", "mikhail@test1.com",
                  "SW Engineer", companies->id);
    scriba_addPOC("Andrey", "Gennadyevich", "Pilyaev",
                  "333", "444", "andrey@test2.com",
                  "Salesperson", companies->next->id);

    people1 = scriba_getPOCByName("Sapozhnikov");
    people2 = scriba_getPOCByName("Pilyaev");

    scriba_addProject("Test Project #1", "100 bottles of whisky",
                      companies->id, PROJECT_STATE_CONTRACT_SIGNED,
                      SCRIBA_CURRENCY_RUB, 1000);
    scriba_addProject("Test Project #2", "100 bottles of rum",
                      companies->id, PROJECT_STATE_CONTRACT_SIGNED,
                      SCRIBA_CURRENCY_EUR, 1000);

    projects = scriba_getAllProjects();

    scriba_addEvent("Test event #1", companies->id, people1->id,
                    projects->id, EVENT_TYPE_MEETING, "Cancelled",
                    cur_time, EVENT_STATE_CANCELLED);
    scriba_addEvent("Test event #2", companies->next->id, people2->id,
                    projects->next->id, EVENT_TYPE_CALL, "Missed",
                    cur_time + 100, EVENT_STATE_COMPLETED);

    events = scriba_getAllEvents();
    CU_ASSERT_FALSE(scriba_list_is_empty(events));
    scriba_list_for_each(events, event)
    {
        count++;
    }
    CU_ASSERT_EQUAL(2, count);

    scriba_list_delete(events);

    events = scriba_getEventsByCompany(companies->id);
    CU_ASSERT_FALSE(scriba_list_is_empty(events));
    CU_ASSERT(scriba_list_is_empty(events->next));
    event1 = scriba_getEvent(events->id);
    CU_ASSERT_PTR_NOT_NULL(event1);
    CU_ASSERT(scriba_id_compare(&(event1->id), &(events->id)));
    CU_ASSERT_STRING_EQUAL(event1->descr, "Test event #1");
    CU_ASSERT(scriba_id_compare(&(event1->company_id), &(companies->id)));
    CU_ASSERT(scriba_id_compare(&(event1->poc_id), &(people1->id)));
    CU_ASSERT(scriba_id_compare(&(event1->project_id), &(projects->id)));
    CU_ASSERT_EQUAL(event1->type, EVENT_TYPE_MEETING);
    CU_ASSERT_STRING_EQUAL(event1->outcome, "Cancelled");
    CU_ASSERT_EQUAL(event1->timestamp, cur_time);
    CU_ASSERT_EQUAL(event1->state, EVENT_STATE_CANCELLED);

    scriba_list_delete(events);

    events = scriba_getEventsByPOC(people2->id);
    CU_ASSERT_FALSE(scriba_list_is_empty(events));
    CU_ASSERT(scriba_list_is_empty(events->next));
    event2 = scriba_getEvent(events->id);
    CU_ASSERT_PTR_NOT_NULL(event2);
    CU_ASSERT(scriba_id_compare(&(event2->id), &(events->id)));
    CU_ASSERT_STRING_EQUAL(event2->descr, "Test event #2");
    CU_ASSERT(scriba_id_compare(&(event2->company_id), &(companies->next->id)));
    CU_ASSERT(scriba_id_compare(&(event2->poc_id), &(people2->id)));
    CU_ASSERT(scriba_id_compare(&(event2->project_id), &(projects->next->id)));
    CU_ASSERT_EQUAL(event2->type, EVENT_TYPE_CALL);
    CU_ASSERT_STRING_EQUAL(event2->outcome, "Missed");
    CU_ASSERT_EQUAL(event2->timestamp, cur_time + 100);
    CU_ASSERT_EQUAL(event2->state, EVENT_STATE_COMPLETED);

    scriba_list_delete(events);
    scriba_freeEventData(event2);
    event2 = NULL;

    events = scriba_getEventsByProject(projects->next->id);
    CU_ASSERT_FALSE(scriba_list_is_empty(events));
    CU_ASSERT(scriba_list_is_empty(events->next));
    event2 = scriba_getEvent(events->id);
    CU_ASSERT_PTR_NOT_NULL(event2);
    CU_ASSERT(scriba_id_compare(&(event2->id), &(events->id)));
    CU_ASSERT_STRING_EQUAL(event2->descr, "Test event #2");
    CU_ASSERT(scriba_id_compare(&(event2->company_id), &(companies->next->id)));
    CU_ASSERT(scriba_id_compare(&(event2->poc_id), &(people2->id)));
    CU_ASSERT(scriba_id_compare(&(event2->project_id), &(projects->next->id)));
    CU_ASSERT_EQUAL(event2->type, EVENT_TYPE_CALL);
    CU_ASSERT_STRING_EQUAL(event2->outcome, "Missed");
    CU_ASSERT_EQUAL(event2->timestamp, cur_time + 100);
    CU_ASSERT_EQUAL(event2->state, EVENT_STATE_COMPLETED);

    scriba_list_delete(events);
    scriba_freeEventData(event2);
    event2 = NULL;

    events = scriba_getEventsByState(EVENT_STATE_COMPLETED);
    CU_ASSERT_FALSE(scriba_list_is_empty(events));
    CU_ASSERT(scriba_list_is_empty(events->next));
    event2 = scriba_getEvent(events->id);
    CU_ASSERT_PTR_NOT_NULL(event2);
    CU_ASSERT(scriba_id_compare(&(event2->id), &(events->id)));
    CU_ASSERT_STRING_EQUAL(event2->descr, "Test event #2");
    CU_ASSERT(scriba_id_compare(&(event2->company_id), &(companies->next->id)));
    CU_ASSERT(scriba_id_compare(&(event2->poc_id), &(people2->id)));
    CU_ASSERT(scriba_id_compare(&(event2->project_id), &(projects->next->id)));
    CU_ASSERT_EQUAL(event2->type, EVENT_TYPE_CALL);
    CU_ASSERT_STRING_EQUAL(event2->outcome, "Missed");
    CU_ASSERT_EQUAL(event2->timestamp, cur_time + 100);
    CU_ASSERT_EQUAL(event2->state, EVENT_STATE_COMPLETED);

    // update the second event and verify
    event2->type = EVENT_TYPE_TASK;
    event2->state = EVENT_STATE_SCHEDULED;
    scriba_updateEvent(event2);
    event3 = scriba_getEvent(events->id);
    CU_ASSERT_PTR_NOT_NULL(event3);
    CU_ASSERT(scriba_id_compare(&(event3->id), &(events->id)));
    CU_ASSERT_STRING_EQUAL(event3->descr, "Test event #2");
    CU_ASSERT(scriba_id_compare(&(event3->company_id), &(companies->next->id)));
    CU_ASSERT(scriba_id_compare(&(event3->poc_id), &(people2->id)));
    CU_ASSERT(scriba_id_compare(&(event3->project_id), &(projects->next->id)));
    CU_ASSERT_EQUAL(event3->type, EVENT_TYPE_TASK);
    CU_ASSERT_STRING_EQUAL(event3->outcome, "Missed");
    CU_ASSERT_EQUAL(event3->timestamp, cur_time + 100);
    CU_ASSERT_EQUAL(event3->state, EVENT_STATE_SCHEDULED);

    // copy event data and verify
    event4 = scriba_copyEvent(event3);
    CU_ASSERT_PTR_NOT_NULL(event4);
    CU_ASSERT_NOT_EQUAL(event4, event3);
    CU_ASSERT(scriba_id_compare(&(event4->id), &(event3->id)));
    CU_ASSERT_STRING_EQUAL(event4->descr, "Test event #2");
    CU_ASSERT(scriba_id_compare(&(event4->company_id), &(companies->next->id)));
    CU_ASSERT(scriba_id_compare(&(event4->poc_id), &(people2->id)));
    CU_ASSERT(scriba_id_compare(&(event4->project_id), &(projects->next->id)));
    CU_ASSERT_EQUAL(event4->type, EVENT_TYPE_TASK);
    CU_ASSERT_STRING_EQUAL(event4->outcome, "Missed");
    CU_ASSERT_EQUAL(event4->timestamp, cur_time + 100);
    CU_ASSERT_EQUAL(event4->state, EVENT_STATE_SCHEDULED);
    scriba_freeEventData(event4);
    event4 = NULL;

    scriba_list_delete(events);
    events = NULL;

    // remove event and verify
    scriba_removeEvent(event1->id);
    event4 = scriba_getEvent(event1->id);
    CU_ASSERT_PTR_NULL(event4);

    // check company event list
    company = scriba_getCompany(companies->next->id);
    CU_ASSERT_FALSE(scriba_list_is_empty(company->event_list));
    CU_ASSERT(scriba_list_is_empty(company->event_list->next));
    CU_ASSERT(scriba_id_compare(&(company->event_list->id), &(event2->id)));

    scriba_list_delete(people1);
    scriba_list_delete(people2);
    scriba_list_delete(projects);
    scriba_list_delete(companies);
    scriba_freeCompanyData(company);
    scriba_freeEventData(event1);
    scriba_freeEventData(event2);
    scriba_freeEventData(event3);

    clean_local_db();
}

void test_create_with_id()
{
    // create company with given ID
    scriba_id_t company_id;

    scriba_id_create(&company_id);
    scriba_addCompanyWithID(company_id, "TestCompany", "TestCompany LLC", "SomeAddress",
                            "0123456789", "333-22-11",
                            "testcompany@test.com");
    struct ScribaCompany *company = scriba_getCompany(company_id);
    CU_ASSERT_PTR_NOT_NULL(company);
    CU_ASSERT(scriba_id_compare(&company_id, &(company->id)));
    CU_ASSERT_STRING_EQUAL(company->name, "TestCompany");

    scriba_freeCompanyData(company);

    // create POC with given ID
    scriba_id_t poc_id;
    scriba_id_create(&poc_id);
    scriba_addPOCWithID(poc_id, "Moose", "Moosevich", "Moose", "4567896312", "1112233",
                        "moose@test.com", "regular moose", company_id);
    struct ScribaPoc *poc = scriba_getPOC(poc_id);
    CU_ASSERT_PTR_NOT_NULL(poc);
    CU_ASSERT(scriba_id_compare(&poc_id, &(poc->id)));
    CU_ASSERT_STRING_EQUAL(poc->position, "regular moose");
    scriba_freePOCData(poc);

    // create project with given ID
    scriba_id_t project_id;
    scriba_id_create(&project_id);
    scriba_addProjectWithID(project_id, "TestProject", "test project", company_id,
                            PROJECT_STATE_OFFER, SCRIBA_CURRENCY_RUB, 1000);
    struct ScribaProject *project = scriba_getProject(project_id);
    CU_ASSERT_PTR_NOT_NULL(project);
    CU_ASSERT(scriba_id_compare(&project_id, &(project->id)));
    CU_ASSERT_EQUAL(project->state, PROJECT_STATE_OFFER);
    scriba_freeProjectData(project);

    // create event with given ID
    scriba_id_t event_id;
    scriba_id_create(&event_id);
    scriba_addEventWithID(event_id, "Test event", company_id, poc_id, project_id,
                          EVENT_TYPE_CALL, "missed", (scriba_time_t)0, EVENT_STATE_CANCELLED);
    struct ScribaEvent *event = scriba_getEvent(event_id);
    CU_ASSERT_PTR_NOT_NULL(event);
    CU_ASSERT(scriba_id_compare(&event_id, &(event->id)));
    CU_ASSERT_EQUAL(event->type, EVENT_TYPE_CALL);
    scriba_freeEventData(event);

    clean_local_db();
}

void test_company_search()
{
    scriba_id_t company1_id;
    scriba_id_t company2_id;
    scriba_id_t company3_id;
    scriba_list_t *companies = NULL;

    scriba_id_create(&company1_id);
    scriba_id_create(&company2_id);
    scriba_id_create(&company3_id);

    scriba_addCompanyWithID(company1_id, "Test Company 1", "SomethingUnique",
                            "unknown", "0123456789", "555",
                            "test@test.com");
    scriba_addCompanyWithID(company2_id, "Test Company 2", "Test2 LLC",
                            "addr2", "0123456789", "555",
                            "test@test.com");
    scriba_addCompanyWithID(company3_id, "Different_company", "Test3 LLC",
                            "addr3", "0123456789", "555",
                            "test@test.com");

    companies = scriba_getCompaniesByName("Test");
    // "Test" should give us name match for company1 and company2
    CU_ASSERT_FALSE(scriba_list_is_empty(companies));
    CU_ASSERT_PTR_NOT_NULL(companies->next);
    CU_ASSERT(scriba_id_compare(&company1_id, &(companies->id)));
    CU_ASSERT(scriba_id_compare(&company2_id, &(companies->next->id)));

    scriba_list_delete(companies);
    companies = NULL;

    companies = scriba_getCompaniesByJurName("LLC");
    // "LLC" should give us juridicial name match for company2 and company3
    CU_ASSERT_FALSE(scriba_list_is_empty(companies));
    CU_ASSERT_PTR_NOT_NULL(companies->next);
    CU_ASSERT(scriba_id_compare(&company2_id, &(companies->id)));
    CU_ASSERT(scriba_id_compare(&company3_id, &(companies->next->id)));

    scriba_list_delete(companies);
    companies = NULL;

    companies = scriba_getCompaniesByName("company");
    // search should be case insensitive, so this should give all three companies
    CU_ASSERT_FALSE(scriba_list_is_empty(companies));
    CU_ASSERT_PTR_NOT_NULL(companies->next);
    CU_ASSERT_PTR_NOT_NULL(companies->next->next);
    CU_ASSERT(scriba_id_compare(&company1_id, &(companies->id)));
    CU_ASSERT(scriba_id_compare(&company2_id, &(companies->next->id)));
    CU_ASSERT(scriba_id_compare(&company3_id, &(companies->next->next->id)));

    scriba_list_delete(companies);
    companies = NULL;

    companies = scriba_getCompaniesByName("Nonexistent");
    CU_ASSERT(scriba_list_is_empty(companies));

    scriba_list_delete(companies);
    companies = NULL;

    companies = scriba_getCompaniesByAddress("addr");
    CU_ASSERT_FALSE(scriba_list_is_empty(companies));
    CU_ASSERT_PTR_NOT_NULL(companies->next);
    CU_ASSERT(scriba_id_compare(&company2_id, &(companies->id)));
    CU_ASSERT(scriba_id_compare(&company3_id, &(companies->next->id)));

    scriba_list_delete(companies);
    companies = NULL;

    clean_local_db();
}

void test_ru_company_search()
{
    scriba_id_t company1_id;
    scriba_id_t company2_id;

    scriba_list_t *companies = NULL;

    scriba_id_create(&company1_id);
    scriba_id_create(&company2_id);

    scriba_addCompanyWithID(company1_id, "Компания№1", "ООО Номер один",
                            "неизвестно", "0123456789", "555",
                            "test1@test.com");
    scriba_addCompanyWithID(company2_id, "другая компания", "ОАО рога и копыта",
                            "адрес", "0123456789", "555",
                            "test2@test.com");

    companies = scriba_getCompaniesByName("комп");
    CU_ASSERT_FALSE(scriba_list_is_empty(companies));
    CU_ASSERT_PTR_NOT_NULL(companies->next);
    CU_ASSERT(scriba_id_compare(&company1_id, &(companies->id)));
    CU_ASSERT(scriba_id_compare(&company2_id, &(companies->next->id)));

    scriba_list_delete(companies);
    companies = NULL;

    companies = scriba_getCompaniesByJurName("ОДИН");
    CU_ASSERT_FALSE(scriba_list_is_empty(companies));
    CU_ASSERT_PTR_NULL(companies->next);
    CU_ASSERT(scriba_id_compare(&company1_id, &(companies->id)));

    scriba_list_delete(companies);
    companies = NULL;

    companies = scriba_getCompaniesByAddress("Адр");
    CU_ASSERT_FALSE(scriba_list_is_empty(companies));
    CU_ASSERT_PTR_NULL(companies->next);
    CU_ASSERT(scriba_id_compare(&company2_id, &(companies->id)));

    scriba_list_delete(companies);
    companies = NULL;

    clean_local_db();
}

void test_event_search()
{
    scriba_id_t event1_id;
    scriba_id_t event2_id;
    scriba_id_t event3_id;
    scriba_id_t company_id;
    scriba_id_t poc_id;
    scriba_id_t project_id;
    scriba_list_t *events = NULL;

    scriba_id_create(&event1_id);
    scriba_id_create(&event2_id);
    scriba_id_create(&event3_id);
    scriba_id_create(&company_id);
    scriba_id_create(&poc_id);
    scriba_id_create(&project_id);

    scriba_addEventWithID(event1_id, "Eating chocolate", company_id, poc_id,
                          project_id, EVENT_TYPE_TASK, "Chocolate is gone",
                          0, EVENT_STATE_COMPLETED);
    scriba_addEventWithID(event2_id, "Buying more chocolate", company_id, poc_id,
                          project_id, EVENT_TYPE_TASK, "",
                          0, EVENT_STATE_SCHEDULED);
    scriba_addEventWithID(event3_id, "Calling doctor", company_id, poc_id,
                          project_id, EVENT_TYPE_CALL, "",
                          0, EVENT_STATE_SCHEDULED);

    events = scriba_getEventsByDescr("choc");
    CU_ASSERT_FALSE(scriba_list_is_empty(events));
    CU_ASSERT_PTR_NOT_NULL(events->next);
    CU_ASSERT(scriba_id_compare(&event1_id, &(events->id)));
    CU_ASSERT(scriba_id_compare(&event2_id, &(events->next->id)));

    scriba_list_delete(events);
    events = NULL;

    events = scriba_getEventsByDescr("call");
    CU_ASSERT_FALSE(scriba_list_is_empty(events));
    CU_ASSERT_PTR_NULL(events->next);
    CU_ASSERT(scriba_id_compare(&event3_id, &(events->id)));

    scriba_list_delete(events);
    events = NULL;

    clean_local_db();
}

void test_ru_event_search()
{
    scriba_id_t event1_id;
    scriba_id_t event2_id;
    scriba_id_t event3_id;
    scriba_id_t company_id;
    scriba_id_t poc_id;
    scriba_id_t project_id;
    scriba_list_t *events = NULL;

    scriba_id_create(&event1_id);
    scriba_id_create(&event2_id);
    scriba_id_create(&event3_id);
    scriba_id_create(&company_id);
    scriba_id_create(&poc_id);
    scriba_id_create(&project_id);

    scriba_addEventWithID(event1_id, "Поедание шоколада", company_id, poc_id,
                          project_id, EVENT_TYPE_TASK, "Шоколад закончился",
                          0, EVENT_STATE_COMPLETED);
    scriba_addEventWithID(event2_id, "Покупка шоколада", company_id, poc_id,
                          project_id, EVENT_TYPE_TASK, "",
                          0, EVENT_STATE_SCHEDULED);
    scriba_addEventWithID(event3_id, "Звонок доктору", company_id, poc_id,
                          project_id, EVENT_TYPE_CALL, "",
                          0, EVENT_STATE_SCHEDULED);

    events = scriba_getEventsByDescr("шок");
    CU_ASSERT_FALSE(scriba_list_is_empty(events));
    CU_ASSERT_PTR_NOT_NULL(events->next);
    CU_ASSERT(scriba_id_compare(&event1_id, &(events->id)));
    CU_ASSERT(scriba_id_compare(&event2_id, &(events->next->id)));

    scriba_list_delete(events);
    events = NULL;

    events = scriba_getEventsByDescr("звон");
    CU_ASSERT_FALSE(scriba_list_is_empty(events));
    CU_ASSERT_PTR_NULL(events->next);
    CU_ASSERT(scriba_id_compare(&event3_id, &(events->id)));

    scriba_list_delete(events);
    events = NULL;

    clean_local_db();
}

void test_poc_search()
{
    scriba_id_t poc1_id;
    scriba_id_t poc2_id;
    scriba_id_t poc3_id;
    scriba_id_t company_id;
    scriba_list_t *people = NULL;

    scriba_id_create(&poc1_id);
    scriba_id_create(&poc2_id);
    scriba_id_create(&poc3_id);
    scriba_id_create(&company_id);

    scriba_addPOCWithID(poc1_id, "Mikhail", "Alekseevich", "Sapozhnikov",
                        "", "", "mas@test.org", "testpos", company_id);
    scriba_addPOCWithID(poc2_id, "Kseniia", "Nikolayevna", "Sapozhnikova",
                        "", "", "ks@test.org", "test", company_id);
    scriba_addPOCWithID(poc3_id, "Andrey", "Gennadyevich", "Pilyaev",
                        "", "", "", "", company_id);

    people = scriba_getPOCByName("sapozh");
    CU_ASSERT_FALSE(scriba_list_is_empty(people));
    CU_ASSERT_PTR_NOT_NULL(people->next);
    CU_ASSERT(scriba_id_compare(&poc1_id, &(people->id)));
    CU_ASSERT(scriba_id_compare(&poc2_id, &(people->next->id)));

    scriba_list_delete(people);
    people = NULL;

    people = scriba_getPOCByName("pil");
    CU_ASSERT_FALSE(scriba_list_is_empty(people));
    CU_ASSERT_PTR_NULL(people->next);
    CU_ASSERT(scriba_id_compare(&poc3_id, &(people->id)));

    scriba_list_delete(people);
    people = NULL;

    people = scriba_getPOCByName("PIL");
    CU_ASSERT_FALSE(scriba_list_is_empty(people));
    CU_ASSERT_PTR_NULL(people->next);
    CU_ASSERT(scriba_id_compare(&poc3_id, &(people->id)));

    scriba_list_delete(people);
    people = NULL;

    people = scriba_getPOCByName("NoSuchPerson");
    CU_ASSERT(scriba_list_is_empty(people));

    scriba_list_delete(people);
    people = NULL;

    people = scriba_getPOCByEmail(".org");
    CU_ASSERT_FALSE(scriba_list_is_empty(people));
    CU_ASSERT_PTR_NOT_NULL(people->next);
    CU_ASSERT(scriba_id_compare(&poc1_id, &(people->id)));
    CU_ASSERT(scriba_id_compare(&poc2_id, &(people->next->id)));

    scriba_list_delete(people);
    people = NULL;

    people = scriba_getPOCByPosition("TEST");
    CU_ASSERT_FALSE(scriba_list_is_empty(people));
    CU_ASSERT_PTR_NOT_NULL(people->next);
    CU_ASSERT(scriba_id_compare(&poc1_id, &(people->id)));
    CU_ASSERT(scriba_id_compare(&poc2_id, &(people->next->id)));

    scriba_list_delete(people);
    people = NULL;

    clean_local_db();
}

void test_ru_poc_search()
{
    scriba_id_t poc1_id;
    scriba_id_t poc2_id;
    scriba_id_t poc3_id;
    scriba_id_t company_id;
    scriba_list_t *people = NULL;

    scriba_id_create(&poc1_id);
    scriba_id_create(&poc2_id);
    scriba_id_create(&poc3_id);
    scriba_id_create(&company_id);

    scriba_addPOCWithID(poc1_id, "Михаил", "Алексеевич", "Сапожников",
                        "", "", "", "тестовая_должность", company_id);
    scriba_addPOCWithID(poc2_id, "Ксения", "Николаевна", "Сапожникова",
                        "", "", "", "тест", company_id);
    scriba_addPOCWithID(poc3_id, "Андрей", "Геннадиевич", "Пиляев",
                        "", "", "", "", company_id);

    people = scriba_getPOCByName("сап");
    CU_ASSERT_FALSE(scriba_list_is_empty(people));
    CU_ASSERT_PTR_NOT_NULL(people->next);
    CU_ASSERT(scriba_id_compare(&poc1_id, &(people->id)));
    CU_ASSERT(scriba_id_compare(&poc2_id, &(people->next->id)));

    scriba_list_delete(people);
    people = NULL;

    people = scriba_getPOCByName("анд");
    CU_ASSERT_FALSE(scriba_list_is_empty(people));
    CU_ASSERT_PTR_NULL(people->next);
    CU_ASSERT(scriba_id_compare(&poc3_id, &(people->id)));

    scriba_list_delete(people);
    people = NULL;

    people = scriba_getPOCByName("ОЖН");
    CU_ASSERT_FALSE(scriba_list_is_empty(people));
    CU_ASSERT_PTR_NOT_NULL(people->next);
    CU_ASSERT(scriba_id_compare(&poc1_id, &(people->id)));
    CU_ASSERT(scriba_id_compare(&poc2_id, &(people->next->id)));

    scriba_list_delete(people);
    people = NULL;

    people = scriba_getPOCByName("неттаких");
    CU_ASSERT(scriba_list_is_empty(people));

    scriba_list_delete(people);
    people = NULL;

    people = scriba_getPOCByPosition("ТЕСТ");
    CU_ASSERT_FALSE(scriba_list_is_empty(people));
    CU_ASSERT_PTR_NOT_NULL(people->next);
    CU_ASSERT(scriba_id_compare(&poc1_id, &(people->id)));
    CU_ASSERT(scriba_id_compare(&poc2_id, &(people->next->id)));

    scriba_list_delete(people);
    people = NULL;

    clean_local_db();
}

void test_project_search()
{
    scriba_id_t proj1_id;
    scriba_id_t proj2_id;
    scriba_id_t proj3_id;
    scriba_id_t company_id;
    scriba_list_t *projects = NULL;

    scriba_id_create(&proj1_id);
    scriba_id_create(&proj2_id);
    scriba_id_create(&proj3_id);
    scriba_id_create(&company_id);

    scriba_addProjectWithID(proj1_id, "Shipbuilding", "",
                            company_id, PROJECT_STATE_INITIAL,
                            SCRIBA_CURRENCY_RUB, 10000);
    scriba_addProjectWithID(proj2_id, "Looking at ships", "",
                            company_id, PROJECT_STATE_INITIAL,
                            SCRIBA_CURRENCY_USD, 500);
    scriba_addProjectWithID(proj3_id, "Fishing", "",
                            company_id, PROJECT_STATE_INITIAL,
                            SCRIBA_CURRENCY_EUR, 100);

    projects = scriba_getProjectsByTitle("ship");
    CU_ASSERT_FALSE(scriba_list_is_empty(projects));
    CU_ASSERT_PTR_NOT_NULL(projects->next);
    CU_ASSERT(scriba_id_compare(&proj1_id, &(projects->id)));
    CU_ASSERT(scriba_id_compare(&proj2_id, &(projects->next->id)));

    scriba_list_delete(projects);
    projects = NULL;

    projects = scriba_getProjectsByTitle("FISH");
    CU_ASSERT_FALSE(scriba_list_is_empty(projects));
    CU_ASSERT_PTR_NULL(projects->next);
    CU_ASSERT(scriba_id_compare(&proj3_id, &(projects->id)));

    scriba_list_delete(projects);
    projects = NULL;

    projects = scriba_getProjectsByTitle("ing");
    CU_ASSERT_FALSE(scriba_list_is_empty(projects));
    CU_ASSERT_PTR_NOT_NULL(projects->next);
    CU_ASSERT_PTR_NOT_NULL(projects->next->next);
    CU_ASSERT(scriba_id_compare(&proj1_id, &(projects->id)));
    CU_ASSERT(scriba_id_compare(&proj2_id, &(projects->next->id)));
    CU_ASSERT(scriba_id_compare(&proj3_id, &(projects->next->next->id)));

    scriba_list_delete(projects);
    projects = NULL;

    clean_local_db();
}

void test_ru_project_search()
{
    scriba_id_t proj1_id;
    scriba_id_t proj2_id;
    scriba_id_t proj3_id;
    scriba_id_t company_id;
    scriba_list_t *projects = NULL;

    scriba_id_create(&proj1_id);
    scriba_id_create(&proj2_id);
    scriba_id_create(&proj3_id);
    scriba_id_create(&company_id);

    scriba_addProjectWithID(proj1_id, "Кораблестроение", "",
                            company_id, PROJECT_STATE_INITIAL,
                            SCRIBA_CURRENCY_RUB, 1000);
    scriba_addProjectWithID(proj2_id, "Глазеем на корабли", "",
                            company_id, PROJECT_STATE_INITIAL,
                            SCRIBA_CURRENCY_RUB, 1000);
    scriba_addProjectWithID(proj3_id, "Рыбалка", "",
                            company_id, PROJECT_STATE_INITIAL,
                            SCRIBA_CURRENCY_RUB, 1000);

    projects = scriba_getProjectsByTitle("корабл");
    CU_ASSERT_FALSE(scriba_list_is_empty(projects));
    CU_ASSERT_PTR_NOT_NULL(projects->next);
    CU_ASSERT(scriba_id_compare(&proj1_id, &(projects->id)));
    CU_ASSERT(scriba_id_compare(&proj2_id, &(projects->next->id)));

    scriba_list_delete(projects);
    projects = NULL;

    projects = scriba_getProjectsByTitle("РЫБ");
    CU_ASSERT_FALSE(scriba_list_is_empty(projects));
    CU_ASSERT_PTR_NULL(projects->next);
    CU_ASSERT(scriba_id_compare(&proj3_id, &(projects->id)));

    scriba_list_delete(projects);
    projects = NULL;

    clean_local_db();
}

// remove all data from the local DB
void clean_local_db()
{
    scriba_list_t *companies = scriba_getAllCompanies();
    scriba_list_for_each(companies, company)
    {
        scriba_removeCompany(company->id);
    }
    scriba_list_delete(companies);

    scriba_list_t *people = scriba_getAllPeople();
    scriba_list_for_each(people, poc)
    {
        scriba_removePOC(poc->id);
    }
    scriba_list_delete(people);

    scriba_list_t *projects = scriba_getAllProjects();
    scriba_list_for_each(projects, project)
    {
        scriba_removeProject(project->id);
    }
    scriba_list_delete(projects);

    scriba_list_t *events = scriba_getAllEvents();
    scriba_list_for_each(events, event)
    {
        scriba_removeEvent(event->id);
    }
    scriba_list_delete(events);
}
