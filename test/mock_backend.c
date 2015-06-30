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

#include "mock_backend.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static struct MockBackendData mockData;

static struct ScribaInternalDB mockDB;



static int internal_init(struct ScribaDBParamList *parList, struct ScribaDBFuncTbl *fTbl);
static void mock_backend_cleanup();

static struct ScribaCompany *getCompany(scriba_id_t id);
static scriba_list_t *getAllCompanies();
static scriba_list_t *getCompaniesByName(const char *name);
static scriba_list_t *getCompaniesByJurName(const char *juridicial_name);
static scriba_list_t *getCompaniesByAddress(const char *address);
static void addCompany(scriba_id_t id, const char *name, const char *jur_name,
                       const char *address, const char *inn, const char *phonenum,
                       const char *email);
static void updateCompany(const struct ScribaCompany *company);
static void removeCompany(scriba_id_t id);
static void free_company_data(struct ScribaCompany *company);

static struct ScribaEvent *getEvent(scriba_id_t id);
static scriba_list_t *getAllEvents();
static scriba_list_t *getEventsByDescr(const char *descr);
static scriba_list_t *getEventsByCompany(scriba_id_t id);
static scriba_list_t *getEventsByPOC(scriba_id_t id);
static scriba_list_t *getEventsByProject(scriba_id_t id);
static scriba_list_t *getEventsByState(enum ScribaEventState state);
static void addEvent(scriba_id_t id, const char *descr, scriba_id_t company_id, scriba_id_t poc_id,
                     scriba_id_t project_id, enum ScribaEventType type, const char *outcome,
                     scriba_time_t timestamp, enum ScribaEventState state);
static void updateEvent(const struct ScribaEvent *event);
static void removeEvent(scriba_id_t id);
static void free_event_data(struct ScribaEvent *event);

static struct ScribaPoc *getPOC(scriba_id_t id);
static scriba_list_t *getAllPeople();
static scriba_list_t *getPOCByName(const char *name);
static scriba_list_t *getPOCByCompany(scriba_id_t id);
static scriba_list_t *getPOCByPosition(const char *position);
static scriba_list_t *getPOCByPhoneNum(const char *phonenum);
static scriba_list_t *getPOCByEmail(const char *email);
static void addPOC(scriba_id_t id, const char *firstname, const char *secondname,
                   const char *lastname, const char *mobilenum, const char *phonenum,
                   const char *email, const char *position, scriba_id_t company_id);
static void updatePOC(const struct ScribaPoc *poc);
static void removePOC(scriba_id_t id);
static void free_poc_data(struct ScribaPoc *poc);

static struct ScribaProject *getProject(scriba_id_t id);
static scriba_list_t *getAllProjects();
static scriba_list_t *getProjectsByTitle(const char *title);
static scriba_list_t *getProjectsByCompany(scriba_id_t id);
static scriba_list_t *getProjectsByState(enum ScribaProjectState state);
static void addProject(scriba_id_t id, const char *title, const char *descr,
                       scriba_id_t company_id, enum ScribaProjectState state);
static void updateProject(const struct ScribaProject *project);
static void removeProject(scriba_id_t id);
static void free_project_data(struct ScribaProject *project);

static char *str_tolower(const char *str);



struct ScribaInternalDB *mock_backend_init()
{
    mockDB.name = "mock backend";
    mockDB.init = internal_init;
    mockDB.cleanup = mock_backend_cleanup;

    return &mockDB;
}

struct MockBackendData *mock_backend_get_data()
{
    return &mockData;
}

static void mock_backend_cleanup()
{
    struct MockCompanyList *company = mockData.companies;
    while (company != NULL)
    {
        struct MockCompanyList *current = company;
        free_company_data(current->data);
        company = current->next;
        free(current);
    }

    struct MockEventList *event = mockData.events;
    while (event != NULL)
    {
        struct MockEventList *current = event;
        free_event_data(current->data);
        event = current->next;
        free(current);
    }

    struct MockPOCList *poc = mockData.people;
    while (poc != NULL)
    {
        struct MockPOCList *current = poc;
        free_poc_data(current->data);
        poc = current->next;
        free(current);
    }

    struct MockProjectList *project = mockData.projects;
    while (project != NULL)
    {
        struct MockProjectList *current = project;
        free_project_data(current->data);
        project = current->next;
        free(current);
    }
}

static int internal_init(struct ScribaDBParamList *parList, struct ScribaDBFuncTbl *fTbl)
{
    fTbl->getCompany = getCompany;
    fTbl->getAllCompanies = getAllCompanies;
    fTbl->getCompaniesByName = getCompaniesByName;
    fTbl->getCompaniesByJurName = getCompaniesByJurName;
    fTbl->getCompaniesByAddress = getCompaniesByAddress;
    fTbl->addCompany = addCompany;
    fTbl->updateCompany = updateCompany;
    fTbl->removeCompany = removeCompany;
    fTbl->getEvent = getEvent;
    fTbl->getAllEvents = getAllEvents;
    fTbl->getEventsByDescr = getEventsByDescr;
    fTbl->getEventsByCompany = getEventsByCompany;
    fTbl->getEventsByPOC = getEventsByPOC;
    fTbl->getEventsByProject = getEventsByProject;
    fTbl->getEventsByState = getEventsByState;
    fTbl->addEvent = addEvent;
    fTbl->updateEvent = updateEvent;
    fTbl->removeEvent = removeEvent;
    fTbl->getPOC = getPOC;
    fTbl->getAllPeople = getAllPeople;
    fTbl->getPOCByName = getPOCByName;
    fTbl->getPOCByCompany = getPOCByCompany;
    fTbl->getPOCByPosition = getPOCByPosition;
    fTbl->getPOCByPhoneNum = getPOCByPhoneNum;
    fTbl->getPOCByEmail = getPOCByEmail;
    fTbl->addPOC = addPOC;
    fTbl->updatePOC = updatePOC;
    fTbl->removePOC = removePOC;
    fTbl->getProject = getProject;
    fTbl->getAllProjects = getAllProjects;
    fTbl->getProjectsByTitle = getProjectsByTitle;
    fTbl->getProjectsByCompany = getProjectsByCompany;
    fTbl->getProjectsByState = getProjectsByState;
    fTbl->addProject = addProject;
    fTbl->updateProject = updateProject;
    fTbl->removeProject = removeProject;

    // init mock data
    mockData.companies = NULL;
    mockData.events = NULL;
    mockData.people = NULL;
    mockData.projects = NULL;

    return 0;
}

static struct ScribaCompany *getCompany(scriba_id_t id)
{
    struct ScribaCompany *ret = NULL;
    struct MockCompanyList *company = mockData.companies;

    while (company != NULL)
    {
        if (scriba_id_compare(&(company->data->id), &id))
        {
            ret = scriba_copyCompany(company->data);
            break;
        }
        company = company->next;
    }

    if (ret != NULL)
    {
        // populate POC list
        ret->poc_list = scriba_list_init();
        struct MockPOCList *poc = mockData.people;
        while (poc != NULL)
        {
            if (scriba_id_compare(&(poc->data->company_id), &id))
            {
                scriba_list_add(ret->poc_list, poc->data->id, NULL);
            }
            poc = poc->next;
        }

        // populate project list
        ret->proj_list = scriba_list_init();
        struct MockProjectList *project = mockData.projects;
        while (project != NULL)
        {
            if (scriba_id_compare(&(project->data->company_id), &id))
            {
                scriba_list_add(ret->proj_list, project->data->id, NULL);
            }
            project = project->next;
        }

        // populate event list
        ret->event_list = scriba_list_init();
        struct MockEventList *event = mockData.events;
        while (event != NULL)
        {
            if (scriba_id_compare(&(event->data->company_id), &id))
            {
                scriba_list_add(ret->event_list, event->data->id, NULL);
            }
            event = event->next;
        }
    }

    return ret;
}

static scriba_list_t *getAllCompanies()
{
    scriba_list_t *list = scriba_list_init();
    struct MockCompanyList *company = mockData.companies;

    while (company != NULL)
    {
        scriba_list_add(list, company->data->id, company->data->name);
        company = company->next;
    }

    return list;
}

static scriba_list_t *getCompaniesByName(const char *name)
{
    scriba_list_t *list = scriba_list_init();
    struct MockCompanyList *company = mockData.companies;
    char *search_lower = str_tolower(name);

    while (company != NULL)
    {
        char *name_lower = str_tolower(company->data->name);

        if (strstr(name_lower, search_lower) != NULL)
        {
            scriba_list_add(list, company->data->id, company->data->name);
        }

        free(name_lower);
        company = company->next;
    }

    free(search_lower);
    return list;
}

static scriba_list_t *getCompaniesByJurName(const char *juridicial_name)
{
    scriba_list_t *list = scriba_list_init();
    struct MockCompanyList *company = mockData.companies;
    char *search_lower = str_tolower(juridicial_name);

    while (company != NULL)
    {
        char *jur_name_lower = str_tolower(company->data->jur_name);
        if (strstr(jur_name_lower, search_lower) != NULL)
        {
            scriba_list_add(list, company->data->id, company->data->name);
        }

        company = company->next;
        free(jur_name_lower);
    }

    free(search_lower);
    return list;
}

static scriba_list_t *getCompaniesByAddress(const char *address)
{
    scriba_list_t *list = scriba_list_init();
    struct MockCompanyList *company = mockData.companies;
    char *search_lower = str_tolower(address);

    while (company != NULL)
    {
        char *addr_lower = str_tolower(company->data->address);
        if (strstr(addr_lower, search_lower) != NULL)
        {
            scriba_list_add(list, company->data->id, company->data->name);
        }

        company = company->next;
        free(addr_lower);
    }

    free(search_lower);
    return list;
}

static void addCompany(scriba_id_t id, const char *name, const char *jur_name,
                       const char *address, const char *inn, const char *phonenum,
                       const char *email)
{
    struct ScribaCompany *new_company = (struct ScribaCompany *)malloc(sizeof (struct ScribaCompany));
    int len = 0;

    memset((void *)new_company, 0, sizeof (struct ScribaCompany));

    scriba_id_copy(&(new_company->id), &id);
    if ((len = strlen(name)) > 0)
    {
        new_company->name = (char *)malloc(len + 1);
        memset(new_company->name, 0, len + 1);
        strncpy(new_company->name, name, len);
    }
    if ((len = strlen(jur_name)) > 0)
    {
        new_company->jur_name = (char *)malloc(len + 1);
        memset(new_company->jur_name, 0, len + 1);
        strncpy(new_company->jur_name, jur_name, len);
    }
    if ((len = strlen(address)) > 0)
    {
        new_company->address = (char *)malloc(len + 1);
        memset(new_company->address, 0, len + 1);
        strncpy(new_company->address, address, len);
    }
    if ((len = strlen(inn)) > 0)
    {
        new_company->inn = (char *)malloc(len + 1);
        memset(new_company->inn, 0, len + 1);
        strncpy(new_company->inn, inn, len);
    }
    if ((len = strlen(phonenum)) > 0)
    {
        new_company->phonenum = (char *)malloc(len + 1);
        memset(new_company->phonenum, 0, len + 1);
        strncpy(new_company->phonenum, phonenum, len);
    }
    if ((len = strlen(email)) > 0)
    {
        new_company->email = (char *)malloc(len + 1);
        memset(new_company->email, 0, len + 1);
        strncpy(new_company->email, email, len);
    }

    struct MockCompanyList *company = mockData.companies;
    struct MockCompanyList *last = NULL;
    while (company != NULL)
    {
        if (company->next == NULL)
        {
            last = company;
        }
        company = company->next;
    }
    company = (struct MockCompanyList *)malloc(sizeof (struct MockCompanyList));
    company->data = new_company;
    company->next = NULL;
    if (mockData.companies == NULL)
    {
        mockData.companies = company;
    }
    if (last != NULL)
    {
        last->next = company;
    }
}

static void updateCompany(const struct ScribaCompany *company)
{
    struct MockCompanyList *cur_company = mockData.companies;

    while (cur_company != NULL)
    {
        if (scriba_id_compare(&(cur_company->data->id), &(company->id)))
        {
            struct ScribaCompany *updated = scriba_copyCompany(company);
            free_company_data(cur_company->data);
            free(cur_company->data);
            cur_company->data = updated;
            break;
        }

        cur_company = cur_company->next;
    }
}

static void removeCompany(scriba_id_t id)
{
    struct MockCompanyList *company = mockData.companies;
    struct MockCompanyList *prev = NULL;

    while (company != NULL)
    {
        if (scriba_id_compare(&(company->data->id), &id))
        {
            if (prev != NULL)
            {
                prev->next = company->next;
            }
            else
            {
                // this was the first one
                mockData.companies = company->next;
            }
            free_company_data(company->data);
            free(company->data);
            free(company);
            break;
        }

        prev = company;
        company = company->next;
    }
}

static void free_company_data(struct ScribaCompany *company)
{
    if (company->name != NULL)
    {
        free(company->name);
        company->name = NULL;
    }
    if (company->jur_name != NULL)
    {
        free(company->jur_name);
        company->jur_name = NULL;
    }
    if (company->address != NULL)
    {
        free(company->address);
        company->address = NULL;
    }
    if (company->inn != NULL)
    {
        free(company->inn);
        company->inn = NULL;
    }
    if (company->phonenum != NULL)
    {
        free(company->phonenum);
        company->phonenum = NULL;
    }
    if (company->email != NULL)
    {
        free(company->email);
        company->email = NULL;
    }
}

static struct ScribaEvent *getEvent(scriba_id_t id)
{
    struct ScribaEvent *ret = NULL;
    struct MockEventList *event = mockData.events;

    while (event != NULL)
    {
        if (scriba_id_compare(&(event->data->id), &id))
        {
            ret = scriba_copyEvent(event->data);
            break;
        }

        event = event->next;
    }

    return ret;
}

static scriba_list_t *getAllEvents()
{
    scriba_list_t *list = scriba_list_init();
    struct MockEventList *event = mockData.events;

    while (event != NULL)
    {
        scriba_list_add(list, event->data->id, event->data->descr);
        event = event->next;
    }

    return list;
}

static scriba_list_t *getEventsByDescr(const char *descr)
{
    scriba_list_t *list = scriba_list_init();
    struct MockEventList *event = mockData.events;
    char *search_lower = str_tolower(descr);

    while (event != NULL)
    {
        char *descr_lower = str_tolower(event->data->descr);
        if (strstr(descr_lower, search_lower) != NULL)
        {
            scriba_list_add(list, event->data->id, event->data->descr);
        }
        event = event->next;
        free(descr_lower);
    }

    free(search_lower);
    return list;
}

static scriba_list_t *getEventsByCompany(scriba_id_t id)
{
    scriba_list_t *list = scriba_list_init();
    struct MockEventList *event = mockData.events;

    while (event != NULL)
    {
        if (scriba_id_compare(&(event->data->company_id), &id))
        {
            scriba_list_add(list, event->data->id, event->data->descr);
        }

        event = event->next;
    }

    return list;
}

static scriba_list_t *getEventsByPOC(scriba_id_t id)
{
    scriba_list_t *list = scriba_list_init();
    struct MockEventList *event = mockData.events;

    while (event != NULL)
    {
        if (scriba_id_compare(&(event->data->poc_id), &id))
        {
            scriba_list_add(list, event->data->id, event->data->descr);
        }

        event = event->next;
    }

    return list;
}

static scriba_list_t *getEventsByProject(scriba_id_t id)
{
    scriba_list_t *list = scriba_list_init();
    struct MockEventList *event = mockData.events;

    while (event != NULL)
    {
        if (scriba_id_compare(&(event->data->project_id), &id))
        {
            scriba_list_add(list, event->data->id, event->data->descr);
        }

        event = event->next;
    }

    return list;
}

static scriba_list_t *getEventsByState(enum ScribaEventState state)
{
    scriba_list_t *list = scriba_list_init();
    struct MockEventList *event = mockData.events;

    while (event != NULL)
    {
        if (event->data->state == state)
        {
            scriba_list_add(list, event->data->id, event->data->descr);
        }

        event = event->next;
    }

    return list;
}

static void addEvent(scriba_id_t id, const char *descr, scriba_id_t company_id, scriba_id_t poc_id,
                     scriba_id_t project_id, enum ScribaEventType type, const char *outcome,
                     scriba_time_t timestamp, enum ScribaEventState state)
{
    struct ScribaEvent *new_event = (struct ScribaEvent *)malloc(sizeof (struct ScribaEvent));
    int len = 0;

    memset(new_event, 0, sizeof (struct ScribaEvent));

    scriba_id_copy(&(new_event->id), &id);
    if ((len = strlen(descr)) > 0)
    {
        new_event->descr = (char *)malloc(len + 1);
        memset(new_event->descr, 0, len + 1);
        strncpy(new_event->descr, descr, len);
    }
    scriba_id_copy(&(new_event->company_id), &company_id);
    scriba_id_copy(&(new_event->poc_id), &poc_id);
    scriba_id_copy(&(new_event->project_id), &project_id);
    new_event->type = type;
    if ((len = strlen(outcome)) > 0)
    {
        new_event->outcome = (char *)malloc(len + 1);
        memset(new_event->outcome, 0, len + 1);
        strncpy(new_event->outcome, outcome, len);
    }
    new_event->timestamp = timestamp;
    new_event->state = state;

    struct MockEventList *event = mockData.events;
    struct MockEventList *last = NULL;
    while (event != NULL)
    {
        if (event->next == NULL)
        {
            last = event;
        }
        event = event->next;
    }
    event = (struct MockEventList *)malloc(sizeof (struct MockEventList));
    event->data = new_event;
    event->next = NULL;
    if (mockData.events == NULL)
    {
        mockData.events = event;
    }
    if (last != NULL)
    {
        last->next = event;
    }
}

static void updateEvent(const struct ScribaEvent *event)
{
    struct MockEventList *cur_event = mockData.events;

    while (cur_event != NULL)
    {
        if (scriba_id_compare(&(cur_event->data->id), &(event->id)))
        {
            struct ScribaEvent *updated = scriba_copyEvent(event);
            free_event_data(cur_event->data);
            free(cur_event->data);
            cur_event->data = updated;
            break;
        }

        cur_event = cur_event->next;
    }
}

static void removeEvent(scriba_id_t id)
{
    struct MockEventList *event = mockData.events;
    struct MockEventList *prev = NULL;

    while (event != NULL)
    {
        if (scriba_id_compare(&(event->data->id), &id))
        {
            if (prev != NULL)
            {
                prev->next = event->next;
            }
            else
            {
                // this was the first one
                mockData.events = event->next;
            }
            free_event_data(event->data);
            free(event->data);
            free(event);
            break;
        }

        prev = event;
        event = event->next;
    }
}

static void free_event_data(struct ScribaEvent *event)
{
    if (event->descr != NULL)
    {
        free(event->descr);
        event->descr = NULL;
    }

    if (event->outcome != NULL)
    {
        free(event->outcome);
        event->outcome = NULL;
    }
}

static struct ScribaPoc *getPOC(scriba_id_t id)
{
    struct ScribaPoc *ret = NULL;
    struct MockPOCList *poc = mockData.people;

    while (poc != NULL)
    {
        if (scriba_id_compare(&(poc->data->id), &id))
        {
            ret = scriba_copyPOC(poc->data);
            break;
        }

        poc = poc->next;
    }

    return ret;
}

static scriba_list_t *getAllPeople()
{
    scriba_list_t *list = scriba_list_init();
    struct MockPOCList *poc = mockData.people;

    while (poc != NULL)
    {
        scriba_list_add(list, poc->data->id, NULL);
        poc = poc->next;
    }

    return list;
} 

static scriba_list_t *getPOCByName(const char *name)
{
    scriba_list_t *list = scriba_list_init();
    struct MockPOCList *poc = mockData.people;
    char *search_lower = str_tolower(name);

    while (poc != NULL)
    {
        int match = 1;
        char *firstname_lower = str_tolower(poc->data->firstname);
        char *secondname_lower = str_tolower(poc->data->secondname);
        char *lastname_lower = str_tolower(poc->data->lastname);

        if (((firstname_lower != NULL) && (strstr(firstname_lower, search_lower) != NULL)) ||
            ((secondname_lower != NULL) && (strstr(secondname_lower, search_lower) != NULL)) ||
            ((lastname_lower != NULL) && (strstr(lastname_lower, search_lower) != NULL)))
        {
            scriba_list_add(list, poc->data->id, NULL);
        }

        poc = poc->next;
        free(firstname_lower);
        free(secondname_lower);
        free(lastname_lower);
    }

    free(search_lower);
    return list;
}

static scriba_list_t *getPOCByCompany(scriba_id_t id)
{
    scriba_list_t *list = scriba_list_init();
    struct MockPOCList *poc = mockData.people;

    while (poc != NULL)
    {
        if (scriba_id_compare(&(poc->data->company_id), &id))
        {
            scriba_list_add(list, poc->data->id, NULL);
        }

        poc = poc->next;
    }

    return list;
}

static scriba_list_t *getPOCByPosition(const char *position)
{
    scriba_list_t *list = scriba_list_init();
    struct MockPOCList *poc = mockData.people;
    char *search_lower = str_tolower(position);

    while (poc != NULL)
    {
        char *pos_lower = str_tolower(poc->data->position);
        if (pos_lower != NULL)
        {
            if (strstr(pos_lower, search_lower) != NULL)
            {
                scriba_list_add(list, poc->data->id, NULL);
            }
        }

        poc = poc->next;
        free(pos_lower);
    }

    free(search_lower);
    return list;
}

static scriba_list_t *getPOCByPhoneNum(const char *phonenum)
{
    scriba_list_t *list = scriba_list_init();
    struct MockPOCList *poc = mockData.people;

    while (poc != NULL)
    {
        if (strcmp(poc->data->phonenum, phonenum) == 0)
        {
            scriba_list_add(list, poc->data->id, NULL);
        }

        poc = poc->next;
    }

    return list;
}

static scriba_list_t *getPOCByEmail(const char *email)
{
    scriba_list_t *list = scriba_list_init();
    struct MockPOCList *poc = mockData.people;
    char *search_lower = str_tolower(email);

    while (poc != NULL)
    {
        char *email_lower = str_tolower(poc->data->email);
        if (email_lower != NULL)
        {
            if (strstr(email_lower, search_lower) != NULL)
            {
                scriba_list_add(list, poc->data->id, NULL);
            }
        }

        poc = poc->next;
        free(email_lower);
    }

    free(search_lower);
    return list;
}

static void addPOC(scriba_id_t id, const char *firstname, const char *secondname,
                   const char *lastname, const char *mobilenum, const char *phonenum,
                   const char *email, const char *position, scriba_id_t company_id)
{
    struct ScribaPoc *new_poc = (struct ScribaPoc *)malloc(sizeof (struct ScribaPoc));
    int len = 0;

    memset(new_poc, 0, sizeof (struct ScribaPoc));

    scriba_id_copy(&(new_poc->id), &id);
    if ((len = strlen(firstname)) > 0)
    {
        new_poc->firstname = (char *)malloc(len + 1);
        memset(new_poc->firstname, 0, len + 1);
        strncpy(new_poc->firstname, firstname, len);
    }
    if ((len = strlen(secondname)) > 0)
    {
        new_poc->secondname = (char *)malloc(len + 1);
        memset(new_poc->secondname, 0, len + 1);
        strncpy(new_poc->secondname, secondname, len);
    }
    if ((len = strlen(lastname)) > 0)
    {
        new_poc->lastname = (char *)malloc(len + 1);
        memset(new_poc->lastname, 0, len + 1);
        strncpy(new_poc->lastname, lastname, len);
    }
    if ((len = strlen(mobilenum)) > 0)
    {
        new_poc->mobilenum = (char *)malloc(len + 1);
        memset(new_poc->mobilenum, 0, len + 1);
        strncpy(new_poc->mobilenum, mobilenum, len);
    }
    if ((len = strlen(phonenum)) > 0)
    {
        new_poc->phonenum = (char *)malloc(len + 1);
        memset(new_poc->phonenum, 0, len + 1);
        strncpy(new_poc->phonenum, phonenum, len);
    }
    if ((len = strlen(email)) > 0)
    {
        new_poc->email = (char *)malloc(len + 1);
        memset(new_poc->email, 0, len + 1);
        strncpy(new_poc->email, email, len);
    }
    if ((len = strlen(position)) > 0)
    {
        new_poc->position = (char *)malloc(len + 1);
        memset(new_poc->position, 0, len + 1);
        strncpy(new_poc->position, position, len);
    }
    scriba_id_copy(&(new_poc->company_id), &company_id);

    struct MockPOCList *poc = mockData.people;
    struct MockPOCList *last = NULL;
    while (poc != NULL)
    {
        if (poc->next == NULL)
        {
            last = poc;
        }
        poc = poc->next;
    }
    poc = (struct MockPOCList *)malloc(sizeof (struct MockPOCList));
    poc->data = new_poc;
    poc->next = NULL;
    if (mockData.people == NULL)
    {
        mockData.people = poc;
    }
    if (last != NULL)
    {
        last->next = poc;
    }
}

static void updatePOC(const struct ScribaPoc *poc)
{
    struct MockPOCList *cur_poc = mockData.people;

    while (cur_poc != NULL)
    {
        if (scriba_id_compare(&(cur_poc->data->id), &(poc->id)))
        {
            struct ScribaPoc *updated = scriba_copyPOC(poc);
            free_poc_data(cur_poc->data);
            free(cur_poc->data);
            cur_poc->data = updated;
            break;
        }

        cur_poc = cur_poc->next;
    }
}

static void removePOC(scriba_id_t id)
{
    struct MockPOCList *poc = mockData.people;
    struct MockPOCList *prev = NULL;

    while (poc != NULL)
    {
        if (scriba_id_compare(&(poc->data->id), &id))
        {
            if (prev != NULL)
            {
                prev->next = poc->next;
            }
            else
            {
                // this was the first one
                mockData.people = poc->next;
            }
            free_poc_data(poc->data);
            free(poc->data);
            free(poc);
            break;
        }

        prev = poc;
        poc = poc->next;
    }
}

static void free_poc_data(struct ScribaPoc *poc)
{
    if (poc->firstname != NULL)
    {
        free(poc->firstname);
        poc->firstname = NULL;
    }
    if (poc->secondname != NULL)
    {
        free(poc->secondname);
        poc->secondname = NULL;
    }
    if (poc->lastname != NULL)
    {
        free(poc->lastname);
        poc->lastname = NULL;
    }
    if (poc->mobilenum != NULL)
    {
        free(poc->mobilenum);
        poc->mobilenum = NULL;
    }
    if (poc->phonenum != NULL)
    {
        free(poc->phonenum);
        poc->phonenum = NULL;
    }
    if (poc->email != NULL)
    {
        free(poc->email);
        poc->email = NULL;
    }
    if (poc->position != NULL)
    {
        free(poc->position);
        poc->position = NULL;
    }
}

static struct ScribaProject *getProject(scriba_id_t id)
{
    struct ScribaProject *ret = NULL;
    struct MockProjectList *project = mockData.projects;

    while (project != NULL)
    {
        if (scriba_id_compare(&(project->data->id), &id))
        {
            ret = scriba_copyProject(project->data);
            break;
        }

        project = project->next;
    }

    return ret;
}

static scriba_list_t *getAllProjects()
{
    scriba_list_t *list = scriba_list_init();
    struct MockProjectList *project = mockData.projects;

    while (project != NULL)
    {
        scriba_list_add(list, project->data->id, NULL);
        project = project->next;
    }

    return list;
}

static scriba_list_t *getProjectsByTitle(const char *title)
{
    scriba_list_t *list = scriba_list_init();
    struct MockProjectList *project = mockData.projects;
    char *search_lower = str_tolower(title);

    while (project != NULL)
    {
        char *title_lower = str_tolower(project->data->title);
        if (strstr(title_lower, search_lower) != NULL)
        {
            scriba_list_add(list, project->data->id, NULL);
        }

        project = project->next;
        free(title_lower);
    }

    free(search_lower);
    return list;
}

static scriba_list_t *getProjectsByCompany(scriba_id_t id)
{
    scriba_list_t *list = scriba_list_init();
    struct MockProjectList *project = mockData.projects;

    while (project != NULL)
    {
        if (scriba_id_compare(&(project->data->company_id), &id))
        {
            scriba_list_add(list, project->data->id, project->data->title);
        }

        project = project->next;
    }

    return list;
}

static scriba_list_t *getProjectsByState(enum ScribaProjectState state)
{
    scriba_list_t *list = scriba_list_init();
    struct MockProjectList *project = mockData.projects;

    while (project != NULL)
    {
        if (project->data->state == state)
        {
            scriba_list_add(list, project->data->id, project->data->title);
        }

        project = project->next;
    }

    return list;
}

static void addProject(scriba_id_t id, const char *title, const char *descr,
                       scriba_id_t company_id, enum ScribaProjectState state)
{
    struct ScribaProject *new_project = (struct ScribaProject *)malloc(sizeof (struct ScribaProject));
    int len = 0;

    memset(new_project, 0, sizeof (struct ScribaProject));

    scriba_id_copy(&(new_project->id), &id);
    if ((len = strlen(title)) != 0)
    {
        new_project->title = (char *)malloc(len + 1);
        memset(new_project->title, 0, len + 1);
        strncpy(new_project->title, title, len);
    }
    if ((len = strlen(descr)) != 0)
    {
        new_project->descr = (char *)malloc(len + 1);
        memset(new_project->descr, 0, len + 1);
        strncpy(new_project->descr, descr, len);
    }
    scriba_id_copy(&(new_project->company_id), &company_id);
    new_project->state = state;

    struct MockProjectList *project = mockData.projects;
    struct MockProjectList *last = NULL;
    while (project != NULL)
    {
        if (project->next == NULL)
        {
            last = project;
        }
        project = project->next;
    }
    project = (struct MockProjectList *)malloc(sizeof (struct MockProjectList));
    project->data = new_project;
    project->next = NULL;
    if (mockData.projects == NULL)
    {
        mockData.projects = project;
    }
    if (last != NULL)
    {
        last->next = project;
    }
}

static void updateProject(const struct ScribaProject *project)
{
    struct MockProjectList *cur_project = mockData.projects;

    while (cur_project != NULL)
    {
        if (scriba_id_compare(&(cur_project->data->id), &(project->id)))
        {
            struct ScribaProject *updated = scriba_copyProject(project);
            free_project_data(cur_project->data);
            free(cur_project->data);
            cur_project->data = updated;
            break;
        }

        cur_project = cur_project->next;
    }
}

static void removeProject(scriba_id_t id)
{
    struct MockProjectList *project = mockData.projects;
    struct MockProjectList *prev = NULL;

    while (project != NULL)
    {
        if (scriba_id_compare(&(project->data->id), &id))
        {
            if (prev != NULL)
            {
                prev->next = project->next;
            }
            else
            {
                // this was the first one
                mockData.projects = project->next;
            }
            free_project_data(project->data);
            free(project->data);
            free(project);
            break;
        }

        prev = project;
        project = project->next;
    }
}

static void free_project_data(struct ScribaProject *project)
{
    if (project->title != NULL)
    {
        free(project->title);
        project->title = NULL;
    }
    if (project->descr != NULL)
    {
        free(project->descr);
        project->descr = NULL;
    }
}

static char *str_tolower(const char *str)
{
    int len = 0;
    char *result = NULL;

    if (str == NULL)
    {
        return NULL;
    }

    len = strlen(str);
    result = (char *)malloc(len + 1);
    for (int i = 0; i < len; i++)
    {
        result[i] = tolower(str[i]);
    }
    result[len] = 0;

    return result;
}
