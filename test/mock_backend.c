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

#include "mock_backend.h"
#include <string.h>
#include <stdlib.h>

static struct MockBackendData mockData;

static struct ScribaInternalDB mockDB;



static int internal_init(struct ScribaDBParamList *parList, struct ScribaDBFuncTbl *fTbl);
static void mock_backend_cleanup();

static struct ScribaCompany *getCompany(scriba_id_t id);
static scriba_list_t *getAllCompanies();
static scriba_list_t *getCompaniesByName(const char *name);
static scriba_list_t *getCompaniesByJurName(const char *juridicial_name);
static scriba_list_t *getCompaniesByAddress(const char *address);
static void addCompany(const char *name, const char *jur_name, const char *address,
                       scriba_inn_t inn, const char *phonenum, const char *email);
static void updateCompany(const struct ScribaCompany *company);
static void removeCompany(scriba_id_t id);
static void free_company_data(struct ScribaCompany *company);

static struct ScribaEvent *getEvent(scriba_id_t id);
static scriba_list_t *getAllEvents();
static scriba_list_t *getEventsByCompany(scriba_id_t id);
static scriba_list_t *getEventsByPOC(scriba_id_t id);
static scriba_list_t *getEventsByProject(scriba_id_t id);
static void addEvent(const char *descr, scriba_id_t company_id, scriba_id_t poc_id,
                     scriba_id_t project_id, enum ScribaEventType type, const char *outcome,
                     scriba_time_t timestamp, enum ScribaEventState state);
static void updateEvent(const struct ScribaEvent *event);
static void removeEvent(scriba_id_t id);
static void free_event_data(struct ScribaEvent *event);

static struct ScribaPoc *getPOC(scriba_id_t id);
static scriba_list_t *getAllPeople();
static scriba_list_t *getPOCByName(const char *firstname,
                                   const char *secondname,
                                   const char *lastname);
static scriba_list_t *getPOCByCompany(scriba_id_t id);
static scriba_list_t *getPOCByPosition(const char *position);
static scriba_list_t *getPOCByPhoneNum(const char *phonenum);
static scriba_list_t *getPOCByEmail(const char *email);
static void addPOC(const char *firstname, const char *secondname, const char *lastname,
                   const char *mobilenum, const char *phonenum, const char *email,
                   const char *position, scriba_id_t company_id);
static void updatePOC(const struct ScribaPoc *poc);
static void removePOC(scriba_id_t id);
static void free_poc_data(struct ScribaPoc *poc);

static struct ScribaProject *getProject(scriba_id_t id);
static scriba_list_t *getAllProjects();
static scriba_list_t *getProjectsByCompany(scriba_id_t id);
static scriba_list_t *getProjectsByState(enum ScribaProjectState state);
static void addProject(const char *title, const char *descr, scriba_id_t company_id,
                       enum ScribaProjectState state);
static void updateProject(const struct ScribaProject *project);
static void removeProject(scriba_id_t id);
static void free_project_data(struct ScribaProject *project);



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
    int i;

    for (i = 0; i < mockData.num_companies; i++)
    {
        if (mockData.companies[i] != NULL)
        {
            free_company_data(mockData.companies[i]);
            free(mockData.companies[i]);
            mockData.companies[i] = NULL;
        }
    }
    if (mockData.companies != NULL)
    {
        free(mockData.companies);
    }
    mockData.num_companies = 0;

    for (i = 0; i < mockData.num_events; i++)
    {
        if (mockData.events[i] != NULL)
        {
            free_event_data(mockData.events[i]);
            free(mockData.events[i]);
            mockData.events[i] = NULL;
        }
    }
    if (mockData.events != NULL)
    {
        free(mockData.events);
    }
    mockData.num_events = 0;

    for (i = 0; i < mockData.num_people; i++)
    {
        if (mockData.people[i] != NULL)
        {
            free_poc_data(mockData.people[i]);
            free(mockData.people[i]);
            mockData.people[i] = NULL;
        }
    }
    if (mockData.people != NULL)
    {
        free(mockData.people);
    }
    mockData.num_people = 0;

    for (i = 0; i < mockData.num_projects; i++)
    {
        if (mockData.projects[i] != NULL)
        {
            free_project_data(mockData.projects[i]);
            free(mockData.projects[i]);
            mockData.projects[i] = NULL;
        }
    }
    if (mockData.projects != NULL)
    {
        free(mockData.projects);
    }
    mockData.num_projects = 0;
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
    fTbl->getEventsByCompany = getEventsByCompany;
    fTbl->getEventsByPOC = getEventsByPOC;
    fTbl->getEventsByProject = getEventsByProject;
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
    fTbl->getProjectsByCompany = getProjectsByCompany;
    fTbl->getProjectsByState = getProjectsByState;
    fTbl->addProject = addProject;
    fTbl->updateProject = updateProject;
    fTbl->removeProject = removeProject;

    // init mock data
    mockData.companies = NULL;
    mockData.num_companies = 0;
    mockData.events = NULL;
    mockData.num_events = 0;
    mockData.people = NULL;
    mockData.num_people = 0;
    mockData.projects = NULL;
    mockData.num_projects = 0;

    return 0;
}

static struct ScribaCompany *getCompany(scriba_id_t id)
{
    int i;
    struct ScribaCompany *ret = NULL;

    for (i = 0; i < mockData.num_companies; i++)
    {
        if (mockData.companies[i] != NULL)
        {
            if (scriba_id_compare(&(mockData.companies[i]->id), &id))
            {
                ret = scriba_copyCompany(mockData.companies[i]);
                break;
            }
        }
    }

    if (ret != NULL)
    {
        // populate POC list
        ret->poc_list = scriba_list_init();
        for (i = 0; i < mockData.num_people; i++)
        {
            if ((mockData.people[i] != NULL) &&
                scriba_id_compare(&(mockData.people[i]->company_id), &id))
            {
                scriba_list_add(ret->poc_list, mockData.people[i]->id, NULL);
            }
        }

        // populate project list
        ret->proj_list = scriba_list_init();
        for (i = 0; i < mockData.num_projects; i++)
        {
            if ((mockData.projects[i] != NULL) &&
                scriba_id_compare(&(mockData.projects[i]->company_id), &id))
            {
                scriba_list_add(ret->proj_list, mockData.projects[i]->id, NULL);
            }
        }

        // populate event list
        ret->event_list = scriba_list_init();
        for (i = 0; i < mockData.num_events; i++)
        {
            if ((mockData.events[i] != NULL) &&
                scriba_id_compare(&(mockData.events[i]->company_id), &id))
            {
                scriba_list_add(ret->event_list, mockData.events[i]->id, NULL);
            }
        }
    }

    return ret;
}

static scriba_list_t *getAllCompanies()
{
    int i;
    scriba_list_t *list = scriba_list_init();

    for (i = 0; i < mockData.num_companies; i++)
    {
        if (mockData.companies[i] != NULL)
        {
            scriba_list_add(list, mockData.companies[i]->id, mockData.companies[i]->name);
        }
    }

    return list;
}

static scriba_list_t *getCompaniesByName(const char *name)
{
    int i;
    scriba_list_t *list = scriba_list_init();

    for (i = 0; i < mockData.num_companies; i++)
    {
        if (mockData.companies[i] != NULL)
        {
            if (strcmp(name, mockData.companies[i]->name) == 0)
            {
                scriba_list_add(list, mockData.companies[i]->id, mockData.companies[i]->name);
            }
        }
    }

    return list;
}

static scriba_list_t *getCompaniesByJurName(const char *juridicial_name)
{
    int i;
    scriba_list_t *list = scriba_list_init();

    for (i = 0; i < mockData.num_companies; i++)
    {
        if (mockData.companies[i] != NULL)
        {
            if (strcmp(juridicial_name, mockData.companies[i]->jur_name) == 0)
            {
                scriba_list_add(list, mockData.companies[i]->id, mockData.companies[i]->name);
            }
        }
    }

    return list;
}

static scriba_list_t *getCompaniesByAddress(const char *address)
{
    int i;
    scriba_list_t *list = scriba_list_init();

    for (i = 0; i < mockData.num_companies; i++)
    {
        if (mockData.companies[i] != NULL)
        {
            if (strcmp(address, mockData.companies[i]->address) == 0)
            {
                scriba_list_add(list, mockData.companies[i]->id, mockData.companies[i]->name);
            }
        }
    }

    return list;
}

static void addCompany(const char *name, const char *jur_name, const char *address,
                       scriba_inn_t inn, const char *phonenum, const char *email)
{
    struct ScribaCompany *new_company = (struct ScribaCompany *)malloc(sizeof (struct ScribaCompany));
    int len = 0;

    memset((void *)new_company, 0, sizeof (struct ScribaCompany));

    scriba_id_create(&(new_company->id));
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
    scriba_copy_inn(&new_company->inn, &inn);
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

    if (mockData.companies == NULL)
    {
        mockData.companies = (struct ScribaCompany **)malloc(sizeof (struct ScribaCompany *));
    }
    else
    {
        size_t new_size = sizeof (struct ScribaCompany *) * (mockData.num_companies + 1);
        mockData.companies = (struct ScribaCompany **)realloc(mockData.companies, new_size);
    }

    mockData.companies[mockData.num_companies] = new_company;
    mockData.num_companies++;
}

static void updateCompany(const struct ScribaCompany *company)
{
    int i;

    for (i = 0; i < mockData.num_companies; i++)
    {
        if (mockData.companies[i] != NULL)
        {
            if (scriba_id_compare(&(mockData.companies[i]->id), &(company->id)))
            {
                struct ScribaCompany *updated = scriba_copyCompany(company);
                free_company_data(mockData.companies[i]);
                free(mockData.companies[i]);
                mockData.companies[i] = updated;
                break;
            }
        }
    }
}

static void removeCompany(scriba_id_t id)
{
    int i;

    for (i = 0; i < mockData.num_companies; i++)
    {
        if (mockData.companies[i] != NULL)
        {
            if (scriba_id_compare(&(mockData.companies[i]->id), &id))
            {
                // just free company data and set company pointer to NULL;
                // pointer itself is not removed from the array for the sake of simplicity
                free_company_data(mockData.companies[i]);
                free(mockData.companies[i]);
                mockData.companies[i] = NULL;
                break;
            }
        }
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
    int i;
    struct ScribaEvent *ret = NULL;

    for (i = 0; i < mockData.num_events; i++)
    {
        if ((mockData.events[i] != NULL) &&
            scriba_id_compare(&(mockData.events[i]->id), &id))
        {
            ret = scriba_copyEvent(mockData.events[i]);
            break;
        }
    }

    return ret;
}

static scriba_list_t *getAllEvents()
{
    int i;
    scriba_list_t *list = scriba_list_init();

    for (i = 0; i < mockData.num_events; i++)
    {
        scriba_list_add(list, mockData.events[i]->id, mockData.events[i]->descr);
    }

    return list;
}

static scriba_list_t *getEventsByCompany(scriba_id_t id)
{
    int i;
    scriba_list_t *list = scriba_list_init();

    for (i = 0; i < mockData.num_events; i++)
    {
        if ((mockData.events[i] != NULL) &&
            scriba_id_compare(&(mockData.events[i]->company_id), &id))
        {
            scriba_list_add(list, mockData.events[i]->id, mockData.events[i]->descr);
        }
    }

    return list;
}

static scriba_list_t *getEventsByPOC(scriba_id_t id)
{
    int i;
    scriba_list_t *list = scriba_list_init();

    for (i = 0; i < mockData.num_events; i++)
    {
        if ((mockData.events[i] != NULL) &&
            scriba_id_compare(&(mockData.events[i]->poc_id), &id))
        {
            scriba_list_add(list, mockData.events[i]->id, mockData.events[i]->descr);
        }
    }

    return list;
}

static scriba_list_t *getEventsByProject(scriba_id_t id)
{
    int i;
    scriba_list_t *list = scriba_list_init();

    for (i = 0; i < mockData.num_events; i++)
    {
        if ((mockData.events[i] != NULL) &&
            scriba_id_compare(&(mockData.events[i]->project_id), &id))
        {
            scriba_list_add(list, mockData.events[i]->id, mockData.events[i]->descr);
        }
    }

    return list;
}

static void addEvent(const char *descr, scriba_id_t company_id, scriba_id_t poc_id,
                     scriba_id_t project_id, enum ScribaEventType type, const char *outcome,
                     scriba_time_t timestamp, enum ScribaEventState state)
{
    struct ScribaEvent *new_event = (struct ScribaEvent *)malloc(sizeof (struct ScribaEvent));
    int len = 0;

    memset(new_event, 0, sizeof (struct ScribaEvent));

    scriba_id_create(&(new_event->id));
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

    if (mockData.events == NULL)
    {
        mockData.events = (struct ScribaEvent **)malloc(sizeof (struct ScribaEvent *));
    }
    else
    {
        int new_size = (mockData.num_events + 1) * sizeof (struct ScribaEvent *);
        mockData.events = (struct ScribaEvent **)realloc(mockData.events, new_size);
    }

    mockData.events[mockData.num_events] = new_event;
    mockData.num_events++;
}

static void updateEvent(const struct ScribaEvent *event)
{
    int i;

    for (i = 0; i < mockData.num_events; i++)
    {
        if ((mockData.events[i] != NULL) &&
            scriba_id_compare(&(mockData.events[i]->id), &(event->id)))
        {
            struct ScribaEvent *updated = scriba_copyEvent(event);
            free_event_data(mockData.events[i]);
            free(mockData.events[i]);
            mockData.events[i] = updated;
            break;
        }
    }
}

static void removeEvent(scriba_id_t id)
{
    int i;

    for (i = 0; i < mockData.num_events; i++)
    {
        if ((mockData.events[i] != NULL) &&
            scriba_id_compare(&(mockData.events[i]->id), &id))
        {
            free_event_data(mockData.events[i]);
            free(mockData.events[i]);
            mockData.events[i] = NULL;
        }
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
    int i;
    struct ScribaPoc *ret = NULL;

    for (i = 0; i < mockData.num_people; i++)
    {
        if ((mockData.people[i] != NULL) &&
            scriba_id_compare(&(mockData.people[i]->id), &id))
        {
            ret = scriba_copyPOC(mockData.people[i]);
            break;
        }
    }

    return ret;
}

static scriba_list_t *getAllPeople()
{
    int i;
    scriba_list_t *list = scriba_list_init();

    for (i = 0; i < mockData.num_people; i++)
    {
        scriba_list_add(list, mockData.people[i]->id, NULL);
    }

    return list;
} 

static scriba_list_t *getPOCByName(const char *firstname,
                                   const char *secondname,
                                   const char *lastname)
{
    int i;
    scriba_list_t *list = scriba_list_init();

    for (i = 0; i < mockData.num_people; i++)
    {
        if (mockData.people[i] != NULL)
        {
            int match = 1;

            if (firstname != NULL)
            {
                match &= (strcmp(mockData.people[i]->firstname, firstname) == 0);
            }
            if (secondname != NULL)
            {
                match &= (strcmp(mockData.people[i]->secondname, secondname) == 0);
            }
            if (lastname != NULL)
            {
                match &= (strcmp(mockData.people[i]->lastname, lastname) == 0);
            }
            if (match)
            {
                scriba_list_add(list, mockData.people[i]->id, NULL);
            }
        }
    }

    return list;
}

static scriba_list_t *getPOCByCompany(scriba_id_t id)
{
    int i;
    scriba_list_t *list = scriba_list_init();

    for (i = 0; i < mockData.num_people; i++)
    {
        if ((mockData.people[i] != NULL) &&
            scriba_id_compare(&(mockData.people[i]->company_id), &id))
        {
            scriba_list_add(list, mockData.people[i]->id, NULL);
        }
    }

    return list;
}

static scriba_list_t *getPOCByPosition(const char *position)
{
    int i;
    scriba_list_t *list = scriba_list_init();

    for (i = 0; i < mockData.num_people; i++)
    {
        if (mockData.people[i] != NULL)
        {
            if (strcmp(mockData.people[i]->position, position) == 0)
            {
                scriba_list_add(list, mockData.people[i]->id, NULL);
            }
        }
    }

    return list;
}

static scriba_list_t *getPOCByPhoneNum(const char *phonenum)
{
    int i;
    scriba_list_t *list = scriba_list_init();

    for (i = 0; i < mockData.num_people; i++)
    {
        if (mockData.people[i] != NULL)
        {
            if (strcmp(mockData.people[i]->phonenum, phonenum) == 0)
            {
                scriba_list_add(list, mockData.people[i]->id, NULL);
            }
        }
    }

    return list;
}

static scriba_list_t *getPOCByEmail(const char *email)
{
    int i;
    scriba_list_t *list = scriba_list_init();

    for (i = 0; i < mockData.num_people; i++)
    {
        if (mockData.people[i] != NULL)
        {
            if (strcmp(mockData.people[i]->email, email) == 0)
            {
                scriba_list_add(list, mockData.people[i]->id, NULL);
            }
        }
    }

    return list;
}

static void addPOC(const char *firstname, const char *secondname, const char *lastname,
                   const char *mobilenum, const char *phonenum, const char *email,
                   const char *position, scriba_id_t company_id)
{
    struct ScribaPoc *new_poc = (struct ScribaPoc *)malloc(sizeof (struct ScribaPoc));
    int len = 0;

    memset(new_poc, 0, sizeof (struct ScribaPoc));

    scriba_id_create(&(new_poc->id));
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

    if (mockData.people == NULL)
    {
        mockData.people = (struct ScribaPoc **)malloc(sizeof (struct ScribaPoc *));
    }
    else
    {
        int new_size = sizeof (struct ScribaPoc *) * (mockData.num_people + 1);
        mockData.people = (struct ScribaPoc **)realloc(mockData.people, new_size);
    }
    mockData.people[mockData.num_people] = new_poc;
    mockData.num_people++;
}

static void updatePOC(const struct ScribaPoc *poc)
{
    int i;

    for (i = 0; i < mockData.num_people; i++)
    {
        if ((mockData.people[i] != NULL) &&
            scriba_id_compare(&(mockData.people[i]->id), &(poc->id)))
        {
            struct ScribaPoc *updated = scriba_copyPOC(poc);
            free_poc_data(mockData.people[i]);
            free(mockData.people[i]);
            mockData.people[i] = updated;
            break;
        }
    }
}

static void removePOC(scriba_id_t id)
{
    int i;

    for (i = 0; i < mockData.num_people; i++)
    {
        if ((mockData.people[i] != NULL) &&
            scriba_id_compare(&(mockData.people[i]->id), &id))
        {
            free_poc_data(mockData.people[i]);
            free(mockData.people[i]);
            mockData.people[i] = NULL;
            break;
        }
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
    int i;
    struct ScribaProject *ret = NULL;

    for (i = 0; i < mockData.num_projects; i++)
    {
        if ((mockData.projects[i] != NULL) &&
            scriba_id_compare(&(mockData.projects[i]->id), &id))
        {
            ret = scriba_copyProject(mockData.projects[i]);
            break;
        }
    }

    return ret;
}

static scriba_list_t *getAllProjects()
{
    int i;
    scriba_list_t *list = scriba_list_init();

    for (i = 0; i < mockData.num_projects; i++)
    {
        if (mockData.projects[i] != NULL)
        {
            scriba_list_add(list, mockData.projects[i]->id, mockData.projects[i]->title);
        }
    }

    return list;
}

static scriba_list_t *getProjectsByCompany(scriba_id_t id)
{
    int i;
    scriba_list_t *list = scriba_list_init();

    for (i = 0; i < mockData.num_projects; i++)
    {
        if ((mockData.projects[i] != NULL) &&
            scriba_id_compare(&(mockData.projects[i]->company_id), &id))
        {
            scriba_list_add(list, mockData.projects[i]->id, mockData.projects[i]->title);
        }
    }

    return list;
}

static scriba_list_t *getProjectsByState(enum ScribaProjectState state)
{
    int i;
    scriba_list_t *list = scriba_list_init();

    for (i = 0; i < mockData.num_projects; i++)
    {
        if ((mockData.projects[i] != NULL) && (mockData.projects[i]->state == state))
        {
            scriba_list_add(list, mockData.projects[i]->id, mockData.projects[i]->title);
        }
    }

    return list;
}

static void addProject(const char *title, const char *descr, scriba_id_t company_id,
                       enum ScribaProjectState state)
{
    struct ScribaProject *new_project = (struct ScribaProject *)malloc(sizeof (struct ScribaProject));
    int len = 0;

    scriba_id_create(&(new_project->id));
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

    if (mockData.projects == NULL)
    {
        mockData.projects = (struct ScribaProject **)malloc(sizeof (struct ScribaProject *));
    }
    else
    {
        int new_size = sizeof (struct ScribaProject *) * (mockData.num_projects + 1);
        mockData.projects = (struct ScribaProject **)realloc(mockData.projects, new_size);
    }
    mockData.projects[mockData.num_projects] = new_project;
    mockData.num_projects++;
}

static void updateProject(const struct ScribaProject *project)
{
    int i;

    for (i = 0; i < mockData.num_projects; i++)
    {
        if ((mockData.projects[i] != NULL) &&
            scriba_id_compare(&(mockData.projects[i]->id), &(project->id)))
        {
            struct ScribaProject *updated = scriba_copyProject(project);
            free_project_data(mockData.projects[i]);
            free(mockData.projects[i]);
            mockData.projects[i] = updated;
            break;
        }
    }
}

static void removeProject(scriba_id_t id)
{
    int i;

    for (i = 0; i < mockData.num_projects; i++)
    {
        if ((mockData.projects[i] != NULL) &&
            scriba_id_compare(&(mockData.projects[i]->id), &id))
        {
            free_project_data(mockData.projects[i]);
            free(mockData.projects[i]);
            mockData.projects[i] = NULL;
            break;
        }
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
