#include "frontend_test.h"
#include "mock_backend.h"
#include <CUnit/CUnit.h>
#include <stdlib.h>
#include <time.h>

int frontend_test_init()
{
    int ret = 0;

    struct ScribaInternalDB *mockDB = NULL;

    mockDB = mock_backend_init();
    scriba_addInternalDB(mockDB);

    struct ScribaDB db;
    db.name = mockDB->name;
    db.type = SCRIBA_DB_BUILTIN;
    db.location = NULL;
    
    if (scriba_init(&db, NULL) != SCRIBA_INIT_SUCCESS)
    {
        ret = 1;
    }

    return ret;
}

int frontend_test_cleanup()
{
    scriba_cleanup();

    return 0;
}

void test_company()
{
    scriba_list_t *companies = NULL;
    struct ScribaCompany *company = NULL;
    struct ScribaCompany *company1 = NULL;
    scriba_inn_t inn1 = scriba_inn_from_string("0123456789");
    scriba_inn_t inn2 = scriba_inn_from_string("9876543210");

    // add the first company and verify it's been added
    scriba_addCompany("Test company #1", "Test1 LLC", "Test Address 1",
                      inn1, "111", "test1@test1.com");
    companies = scriba_getAllCompanies();
    CU_ASSERT_FALSE(scriba_list_is_empty(companies));
    CU_ASSERT(scriba_list_is_empty(companies->next));
    company = scriba_getCompany(companies->id);
    CU_ASSERT_PTR_NOT_NULL(company);
    CU_ASSERT_EQUAL(company->id, companies->id);
    CU_ASSERT_STRING_EQUAL(company->name, "Test company #1");
    CU_ASSERT_STRING_EQUAL(company->jur_name, "Test1 LLC");
    CU_ASSERT_STRING_EQUAL(company->address, "Test Address 1");
    CU_ASSERT(scriba_inn_is_equal(&(company->inn), &inn1));
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
                      inn2, "222", "test2@test2.com");
    companies = scriba_getAllCompanies();
    CU_ASSERT_FALSE(scriba_list_is_empty(companies));
    CU_ASSERT_FALSE(scriba_list_is_empty(companies->next));
    CU_ASSERT(scriba_list_is_empty(companies->next->next));
    company = scriba_getCompany(companies->next->id);
    CU_ASSERT_PTR_NOT_NULL(company);
    CU_ASSERT_EQUAL(company->id, companies->next->id);
    CU_ASSERT_STRING_EQUAL(company->name, "Test company #2");
    CU_ASSERT_STRING_EQUAL(company->jur_name, "Test2 LLC");
    CU_ASSERT_STRING_EQUAL(company->address, "Test Address 2");
    CU_ASSERT(scriba_inn_is_equal(&(company->inn), &inn2));
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
    CU_ASSERT_EQUAL(company->id, companies->id);
    CU_ASSERT_STRING_EQUAL(company->name, "Test company #1");

    scriba_list_delete(companies);
    scriba_freeCompanyData(company);
    company = NULL;

    companies = scriba_getCompaniesByJurName("Test2 LLC");
    CU_ASSERT_FALSE(scriba_list_is_empty(companies));
    CU_ASSERT(scriba_list_is_empty(companies->next));
    company = scriba_getCompany(companies->id);
    CU_ASSERT_PTR_NOT_NULL(company);
    CU_ASSERT_EQUAL(company->id, companies->id);
    CU_ASSERT_STRING_EQUAL(company->name, "Test company #2");

    scriba_list_delete(companies);
    scriba_freeCompanyData(company);
    company = NULL;

    companies = scriba_getCompaniesByAddress("Test Address 1");
    CU_ASSERT_FALSE(scriba_list_is_empty(companies));
    CU_ASSERT(scriba_list_is_empty(companies->next));
    company = scriba_getCompany(companies->id);
    CU_ASSERT_PTR_NOT_NULL(company);
    CU_ASSERT_EQUAL(company->id, companies->id);
    CU_ASSERT_STRING_EQUAL(company->name, "Test company #1");

    scriba_list_delete(companies);

    // create a copy of the first company's data structure
    company1 = scriba_copyCompany(company);
    CU_ASSERT_PTR_NOT_NULL(company1);
    CU_ASSERT_NOT_EQUAL(company1, company);
    CU_ASSERT_EQUAL(company1->id, company->id);
    CU_ASSERT_STRING_EQUAL(company->name, "Test company #1");
    CU_ASSERT_STRING_EQUAL(company->jur_name, "Test1 LLC");
    CU_ASSERT_STRING_EQUAL(company->address, "Test Address 1");
    CU_ASSERT(scriba_inn_is_equal(&(company->inn), &inn1));
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
    CU_ASSERT(scriba_inn_is_equal(&(company->inn), &inn1));
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
    CU_ASSERT_EQUAL(company->id, companies->id);
    CU_ASSERT_STRING_EQUAL(company->name, "Test company #2");

    scriba_list_delete(companies);
    scriba_freeCompanyData(company);
    scriba_freeCompanyData(company1);
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
    scriba_inn_t inn1 = scriba_inn_from_string("0123456789");
    scriba_inn_t inn2 = scriba_inn_from_string("9876543210");
    int count = 0;

    // add test companies
    scriba_addCompany("Test company #1", "Test1 LLC", "Test Address 1",
                      inn1, "111", "test1@test1.com");
    scriba_addCompany("Test company #2", "Test2 LLC", "Test Address 2",
                      inn2, "222", "test2@test2.com");

    companies = scriba_getAllCompanies();

    // add test people and verify that they are added properly
    scriba_addPOC("Mikhail", "Alekseevich", "Sapozhnikov",
                  "1111", "2222", "mikhail@test1.com",
                  "SW Engineer", companies->id);
    people = scriba_getPOCByName("Mikhail", "Alekseevich", "Sapozhnikov");
    CU_ASSERT_FALSE(scriba_list_is_empty(people));
    CU_ASSERT(scriba_list_is_empty(people->next));
    poc1 = scriba_getPOC(people->id);
    CU_ASSERT_PTR_NOT_NULL(poc1);
    CU_ASSERT_EQUAL(poc1->id, people->id);
    CU_ASSERT_STRING_EQUAL(poc1->firstname, "Mikhail");
    CU_ASSERT_STRING_EQUAL(poc1->secondname, "Alekseevich");
    CU_ASSERT_STRING_EQUAL(poc1->lastname, "Sapozhnikov");
    CU_ASSERT_STRING_EQUAL(poc1->mobilenum, "1111");
    CU_ASSERT_STRING_EQUAL(poc1->phonenum, "2222");
    CU_ASSERT_STRING_EQUAL(poc1->email, "mikhail@test1.com");
    CU_ASSERT_STRING_EQUAL(poc1->position, "SW Engineer");
    CU_ASSERT_EQUAL(poc1->company_id, companies->id);

    scriba_list_delete(people);

    scriba_addPOC("Alexey", "Vladimirovich", "Sapozhnikov",
                  "3333", "4444", "alexey@test2.com",
                  "Maritime Inspector", companies->next->id);
    people = scriba_getPOCByName("Alexey", NULL, NULL);
    CU_ASSERT_FALSE(scriba_list_is_empty(people));
    CU_ASSERT(scriba_list_is_empty(people->next));
    poc2 = scriba_getPOC(people->id);
    CU_ASSERT_PTR_NOT_NULL(poc2);
    CU_ASSERT_EQUAL(poc2->id, people->id);
    CU_ASSERT_STRING_EQUAL(poc2->firstname, "Alexey");
    CU_ASSERT_STRING_EQUAL(poc2->secondname, "Vladimirovich");
    CU_ASSERT_STRING_EQUAL(poc2->lastname, "Sapozhnikov");
    CU_ASSERT_STRING_EQUAL(poc2->mobilenum, "3333");
    CU_ASSERT_STRING_EQUAL(poc2->phonenum, "4444");
    CU_ASSERT_STRING_EQUAL(poc2->email, "alexey@test2.com");
    CU_ASSERT_STRING_EQUAL(poc2->position, "Maritime Inspector");
    CU_ASSERT_EQUAL(poc2->company_id, companies->next->id);

    scriba_list_delete(people);

    scriba_addPOC("Elena", "Yurievna", "Sapozhnikova",
                  "5555", "6666", "elena@test2.com",
                  "Accountant", companies->next->id);
    people = scriba_getPOCByName(NULL, "Yurievna", NULL);
    CU_ASSERT_FALSE(scriba_list_is_empty(people));
    CU_ASSERT(scriba_list_is_empty(people->next));
    poc3 = scriba_getPOC(people->id);
    CU_ASSERT_PTR_NOT_NULL(poc3);
    CU_ASSERT_EQUAL(poc3->id, people->id);
    CU_ASSERT_STRING_EQUAL(poc3->firstname, "Elena");
    CU_ASSERT_STRING_EQUAL(poc3->secondname, "Yurievna");
    CU_ASSERT_STRING_EQUAL(poc3->lastname, "Sapozhnikova");
    CU_ASSERT_STRING_EQUAL(poc3->mobilenum, "5555");
    CU_ASSERT_STRING_EQUAL(poc3->phonenum, "6666");
    CU_ASSERT_STRING_EQUAL(poc3->email, "elena@test2.com");
    CU_ASSERT_STRING_EQUAL(poc3->position, "Accountant");
    CU_ASSERT_EQUAL(poc3->company_id, companies->next->id);

    scriba_list_delete(people);

    people = scriba_getPOCByName(NULL, NULL, "Sapozhnikov");
    CU_ASSERT_FALSE(scriba_list_is_empty(people));
    CU_ASSERT_FALSE(scriba_list_is_empty(people->next));
    CU_ASSERT_EQUAL(people->id, poc1->id);
    CU_ASSERT_EQUAL(people->next->id, poc2->id);

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
    CU_ASSERT_EQUAL(people->id, poc2->id);
    CU_ASSERT_EQUAL(people->next->id, poc3->id);

    scriba_list_delete(people);

    people = scriba_getPOCByPosition("SW Engineer");
    CU_ASSERT_FALSE(scriba_list_is_empty(people));
    CU_ASSERT(scriba_list_is_empty(people->next));
    CU_ASSERT_EQUAL(people->id, poc1->id);

    scriba_list_delete(people);

    people = scriba_getPOCByEmail("elena@test2.com");
    CU_ASSERT_FALSE(scriba_list_is_empty(people));
    CU_ASSERT(scriba_list_is_empty(people->next));
    CU_ASSERT_EQUAL(people->id, poc3->id);

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
    CU_ASSERT_EQUAL(poc4->company_id, companies->next->id);

    scriba_freePOCData(poc3);
    poc3 = NULL;

    // copy POC data and verify
    poc3 = scriba_copyPOC(poc4);
    CU_ASSERT_PTR_NOT_NULL(poc3);
    CU_ASSERT_NOT_EQUAL(poc3, poc4);
    CU_ASSERT_EQUAL(poc3->id, poc4->id);
    CU_ASSERT_STRING_EQUAL(poc3->firstname, "Ksenia");
    CU_ASSERT_STRING_EQUAL(poc3->secondname, "Nikolayevna");
    CU_ASSERT_STRING_EQUAL(poc3->lastname, "Sapozhnikova");
    CU_ASSERT_STRING_EQUAL(poc3->mobilenum, "5555");
    CU_ASSERT_STRING_EQUAL(poc3->phonenum, "6666");
    CU_ASSERT_STRING_EQUAL(poc3->email, "ksenia@test2.com");
    CU_ASSERT_STRING_EQUAL(poc3->position, "Logistics specialist");
    CU_ASSERT_EQUAL(poc3->company_id, companies->next->id);
    
    // check companies' POC lists
    company = scriba_getCompany(companies->id);
    CU_ASSERT_FALSE(scriba_list_is_empty(company->poc_list));
    CU_ASSERT(scriba_list_is_empty(company->poc_list->next));
    CU_ASSERT_EQUAL(company->poc_list->id, poc1->id);
    scriba_freeCompanyData(company);

    company = scriba_getCompany(companies->next->id);
    CU_ASSERT_FALSE(scriba_list_is_empty(company->poc_list));
    CU_ASSERT_FALSE(scriba_list_is_empty(company->poc_list->next));
    CU_ASSERT_EQUAL(company->poc_list->id, poc2->id);
    CU_ASSERT_EQUAL(company->poc_list->next->id, poc4->id);
    scriba_freeCompanyData(company);

    // remove one poc and verify that it is indeed removed
    scriba_removePOC(poc1->id);
    people = NULL;
    people = scriba_getPOCByName("Mikhail", "Alekseevich", "Sapozhnikov");
    CU_ASSERT(scriba_list_is_empty(people));
    scriba_list_delete(people);

    scriba_list_delete(companies);
    scriba_freePOCData(poc1);
    scriba_freePOCData(poc2);
    scriba_freePOCData(poc3);
    scriba_freePOCData(poc4);
}

void test_project()
{
    scriba_list_t *companies = NULL;
    scriba_list_t *projects = NULL;
    struct ScribaProject *project1 = NULL;
    struct ScribaProject *project2 = NULL;
    struct ScribaProject *project3 = NULL;
    struct ScribaCompany *company = NULL;
    scriba_inn_t inn1 = scriba_inn_from_string("0123456789");
    scriba_inn_t inn2 = scriba_inn_from_string("9876543210");

    // add test company
    scriba_addCompany("Test company #1", "Test1 LLC", "Test Address 1",
                      inn1, "111", "test1@test1.com");

    companies = scriba_getAllCompanies();

    // add test project and verify that it's been added correctly
    scriba_addProject("Test Project #1", "100 bottles of whisky",
                      companies->id, PROJECT_STATE_CONTRACT_SIGNED);

    projects = scriba_getAllProjects();
    CU_ASSERT_FALSE(scriba_list_is_empty(projects));
    CU_ASSERT(scriba_list_is_empty(projects->next));

    project1 = scriba_getProject(projects->id);
    CU_ASSERT_PTR_NOT_NULL(project1);
    CU_ASSERT_EQUAL(project1->id, projects->id);
    CU_ASSERT_STRING_EQUAL(project1->title, "Test Project #1");
    CU_ASSERT_STRING_EQUAL(project1->descr, "100 bottles of whisky");
    CU_ASSERT_EQUAL(project1->company_id, companies->id);
    CU_ASSERT_EQUAL(project1->state, PROJECT_STATE_CONTRACT_SIGNED);

    scriba_list_delete(projects);

    // add another project
    scriba_addProject("Test Project #2", "1000 bottles of milk",
                      companies->id, PROJECT_STATE_REJECTED);

    projects = scriba_getProjectsByCompany(companies->id);
    CU_ASSERT_FALSE(scriba_list_is_empty(projects));
    CU_ASSERT_FALSE(scriba_list_is_empty(projects->next));

    project2 = scriba_getProject(projects->next->id);
    CU_ASSERT_PTR_NOT_NULL(project2);

    CU_ASSERT_PTR_EQUAL(projects->id, project1->id);
    CU_ASSERT_PTR_EQUAL(projects->next->id, project2->id);

    scriba_list_delete(projects);

    projects = scriba_getProjectsByState(PROJECT_STATE_REJECTED);
    CU_ASSERT_FALSE(scriba_list_is_empty(projects));
    CU_ASSERT(scriba_list_is_empty(projects->next));
    CU_ASSERT_EQUAL(projects->id, project2->id);
    
    scriba_list_delete(projects);

    // update project and verify
    projects = scriba_getAllProjects();
    project2->state = PROJECT_STATE_OFFER;
    scriba_updateProject(project2);
    scriba_freeProjectData(project2);
    project2 = scriba_getProject(projects->next->id);
    CU_ASSERT_PTR_NOT_NULL(project2);
    CU_ASSERT_PTR_EQUAL(project2->id, projects->next->id);
    CU_ASSERT_STRING_EQUAL(project2->title, "Test Project #2");
    CU_ASSERT_STRING_EQUAL(project2->descr, "1000 bottles of milk");
    CU_ASSERT_EQUAL(project2->company_id, companies->id);
    CU_ASSERT_EQUAL(project2->state, PROJECT_STATE_OFFER);

    scriba_list_delete(projects);

    // copy project data and verify
    project3 = scriba_copyProject(project1);
    CU_ASSERT_PTR_NOT_NULL(project3);
    CU_ASSERT_NOT_EQUAL(project3, project1);
    CU_ASSERT_EQUAL(project3->id, project1->id);
    CU_ASSERT_STRING_EQUAL(project1->title, "Test Project #1");
    CU_ASSERT_STRING_EQUAL(project1->descr, "100 bottles of whisky");
    CU_ASSERT_EQUAL(project1->company_id, companies->id);
    CU_ASSERT_EQUAL(project1->state, PROJECT_STATE_CONTRACT_SIGNED);
    scriba_freeProjectData(project3);
    project3 = NULL;

    // check company project list
    company = scriba_getCompany(companies->id);
    CU_ASSERT_FALSE(scriba_list_is_empty(company->proj_list));
    CU_ASSERT_FALSE(scriba_list_is_empty(company->proj_list->next));
    CU_ASSERT_EQUAL(company->proj_list->id, project1->id);
    CU_ASSERT_EQUAL(company->proj_list->next->id, project2->id);
    scriba_freeCompanyData(company);

    // remove the first project and verify
    scriba_removeProject(project1->id);
    project3 = scriba_getProject(project1->id);
    CU_ASSERT_PTR_NULL(project3);

    scriba_list_delete(companies);

    scriba_freeProjectData(project1);
    scriba_freeProjectData(project2);
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
    scriba_inn_t inn1 = scriba_inn_from_string("0123456789");
    scriba_inn_t inn2 = scriba_inn_from_string("9876543210");
    int count = 0;
    scriba_time_t cur_time = time(NULL);

    // add test data - companies, people, projects
    scriba_addCompany("Test company #1", "Test1 LLC", "Test Address 1",
                      inn1, "111", "test1@test1.com");
    scriba_addCompany("Test company #2", "Test2 LLC", "Test Address 2",
                      inn2, "222", "test2@test2.com");

    companies = scriba_getAllCompanies();

    scriba_addPOC("Mikhail", "Alekseevich", "Sapozhnikov",
                  "1111", "2222", "mikhail@test1.com",
                  "SW Engineer", companies->id);
    scriba_addPOC("Andrey", "Gennadyevich", "Pilyaev",
                  "333", "444", "andrey@test2.com",
                  "Salesperson", companies->next->id);

    people1 = scriba_getPOCByName(NULL, NULL, "Sapozhnikov");
    people2 = scriba_getPOCByName(NULL, NULL, "Pilyaev");

    scriba_addProject("Test Project #1", "100 bottles of whisky",
                      companies->id, PROJECT_STATE_CONTRACT_SIGNED);
    scriba_addProject("Test Project #2", "100 bottles of rum",
                      companies->id, PROJECT_STATE_CONTRACT_SIGNED);

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
    CU_ASSERT_EQUAL(event1->id, events->id);
    CU_ASSERT_STRING_EQUAL(event1->descr, "Test event #1");
    CU_ASSERT_EQUAL(event1->company_id, companies->id);
    CU_ASSERT_EQUAL(event1->poc_id, people1->id);
    CU_ASSERT_EQUAL(event1->project_id, projects->id);
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
    CU_ASSERT_EQUAL(event2->id, events->id);
    CU_ASSERT_STRING_EQUAL(event2->descr, "Test event #2");
    CU_ASSERT_EQUAL(event2->company_id, companies->next->id);
    CU_ASSERT_EQUAL(event2->poc_id, people2->id);
    CU_ASSERT_EQUAL(event2->project_id, projects->next->id);
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
    CU_ASSERT_EQUAL(event2->id, events->id);
    CU_ASSERT_STRING_EQUAL(event2->descr, "Test event #2");
    CU_ASSERT_EQUAL(event2->company_id, companies->next->id);
    CU_ASSERT_EQUAL(event2->poc_id, people2->id);
    CU_ASSERT_EQUAL(event2->project_id, projects->next->id);
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
    CU_ASSERT_EQUAL(event3->id, events->id);
    CU_ASSERT_STRING_EQUAL(event3->descr, "Test event #2");
    CU_ASSERT_EQUAL(event3->company_id, companies->next->id);
    CU_ASSERT_EQUAL(event3->poc_id, people2->id);
    CU_ASSERT_EQUAL(event3->project_id, projects->next->id);
    CU_ASSERT_EQUAL(event3->type, EVENT_TYPE_TASK);
    CU_ASSERT_STRING_EQUAL(event3->outcome, "Missed");
    CU_ASSERT_EQUAL(event3->timestamp, cur_time + 100);
    CU_ASSERT_EQUAL(event3->state, EVENT_STATE_SCHEDULED);

    // copy event data and verify
    event4 = scriba_copyEvent(event3);
    CU_ASSERT_PTR_NOT_NULL(event4);
    CU_ASSERT_NOT_EQUAL(event4, event3);
    CU_ASSERT_EQUAL(event4->id, event3->id);
    CU_ASSERT_STRING_EQUAL(event4->descr, "Test event #2");
    CU_ASSERT_EQUAL(event4->company_id, companies->next->id);
    CU_ASSERT_EQUAL(event4->poc_id, people2->id);
    CU_ASSERT_EQUAL(event4->project_id, projects->next->id);
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
    CU_ASSERT_EQUAL(company->event_list->id, event2->id);

    scriba_list_delete(people1);
    scriba_list_delete(people2);
    scriba_list_delete(projects);
    scriba_list_delete(companies);
    scriba_freeCompanyData(company);
    scriba_freeEventData(event1);
    scriba_freeEventData(event2);
    scriba_freeEventData(event3);
}
