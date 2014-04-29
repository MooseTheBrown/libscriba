#ifndef SCRIBA_PROJECT_H
#define SCRIBA_PROJECT_H

#include "types.h"

// project states
enum ScribaProjectState
{
    PROJECT_STATE_INITIAL = 0,
    PROJECT_STATE_CLIENT_INFORMED = 1,
    PROJECT_STATE_CLIENT_RESPONSE = 2,
    PROJECT_STATE_OFFER = 3,
    PROJECT_STATE_REJECTED = 4,
    PROJECT_STATE_CONTRACT_SIGNED = 5,
    PROJECT_STATE_EXECUTION = 6,
    PROJECT_STATE_PAYMENT = 7
};

// project data structure
struct ScribaProject
{
    scriba_id_t id;                     // unique id assigned by the library
    char *title;                        // project title
    char *descr;                        // project description
    scriba_id_t company_id;             // id of company associated with the project
    enum ScribaProjectState state;      // state of the project
};

// get project by id
struct ScribaProject *scriba_getProject(scriba_id_t id);
// get all projects in the database
scriba_list_t *scriba_getAllProjects();
// get projects associated with given company
scriba_list_t *scriba_getProjectsByCompany(scriba_id_t id);
// get projects with given state
scriba_list_t *scriba_getProjectsByState(enum ScribaProjectState state);
// add project to the database
void scriba_addProject(const char *title, const char *descr, scriba_id_t company_id,
                       enum ScribaProjectState state);
// update project info
void scriba_updateProject(const struct ScribaProject *project);
// remove project from the database
void scriba_removeProject(scriba_id_t id);
// create a copy of project data structure
struct ScribaProject *scriba_copyProject(const struct ScribaProject *project);
// free memory occupied by project data structure
void scriba_freeProjectData(struct ScribaProject *project);

#endif // SCRIBA_PROJECT_H
