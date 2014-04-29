#ifndef SCRIBA_MOCK_BACKEND_H
#define SCRIBA_MOCK_BACKEND_H

#include "db_backend.h"

struct MockBackendData
{
    // companies
    struct ScribaCompany **companies;
    int num_companies;
    scriba_id_t next_company_id;

    // events
    struct ScribaEvent **events;
    int num_events;
    scriba_id_t next_event_id;

    // people
    struct ScribaPoc **people;
    int num_people;
    scriba_id_t next_poc_id;

    // projects
    struct ScribaProject **projects;
    int num_projects;
    scriba_id_t next_project_id;
};

struct ScribaInternalDB *mock_backend_init();
struct MockBackendData *mock_backend_get_data();

#endif // SCRIBA_MOCK_BACKEND_H
