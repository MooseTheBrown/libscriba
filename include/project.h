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

#ifndef SCRIBA_PROJECT_H
#define SCRIBA_PROJECT_H

#include "types.h"

#ifdef __cplusplus
extern "C"
{
#endif

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

// supported currency types
enum ScribaCurrency
{
    SCRIBA_CURRENCY_RUB = 0,
    SCRIBA_CURRENCY_USD = 1,
    SCRIBA_CURRENCY_EUR = 2
};

// time comparison type
enum ScribaTimeComp
{
    SCRIBA_TIME_IGNORE = 0,
    SCRIBA_TIME_BEFORE = 1,
    SCRIBA_TIME_AFTER = 2
};

// project data structure
struct ScribaProject
{
    scriba_id_t id;                     // unique id assigned by the library
    char *title;                        // project title
    char *descr;                        // project description
    scriba_id_t company_id;             // id of company associated with the project
    enum ScribaProjectState state;      // state of the project
    enum ScribaCurrency currency;       // currency of the project
    long long cost;                     // cost of the project
    scriba_time_t start_time;           // project start time
    scriba_time_t mod_time;             // project state modification time
};

// get project by id
struct ScribaProject *scriba_getProject(scriba_id_t id);
// get all projects in the database
scriba_list_t *scriba_getAllProjects();
// search projects by title
scriba_list_t *scriba_getProjectsByTitle(const char *title);
// get projects associated with given company
scriba_list_t *scriba_getProjectsByCompany(scriba_id_t id);
// get projects with given state
scriba_list_t *scriba_getProjectsByState(enum ScribaProjectState state);
// get projects by time
scriba_list_t *scriba_getProjectsByTime(scriba_time_t start_time, enum ScribaTimeComp start_comp,
                                        scriba_time_t mod_time, enum ScribaTimeComp mod_comp);
// get projects by state and time
scriba_list_t *scriba_getProjectsByStateTime(enum ScribaProjectState state,
                                             scriba_time_t start_time,
                                             enum ScribaTimeComp start_comp,
                                             scriba_time_t mod_time,
                                             enum ScribaTimeComp mod_comp);
// add project to the database
void scriba_addProject(const char *title, const char *descr, scriba_id_t company_id,
                       enum ScribaProjectState state, enum ScribaCurrency currency,
                       long long cost, scriba_time_t start_time);
// add project with given ID to the database
void scriba_addProjectWithID(scriba_id_t id, const char *title, const char *descr,
                             scriba_id_t company_id, enum ScribaProjectState state,
                             enum ScribaCurrency currency, long long cost,
                             scriba_time_t start_time);
// update project info
void scriba_updateProject(struct ScribaProject *project);
// remove project from the database
void scriba_removeProject(scriba_id_t id);
// create a copy of project data structure
struct ScribaProject *scriba_copyProject(const struct ScribaProject *project);
// free memory occupied by project data structure
void scriba_freeProjectData(struct ScribaProject *project);

#ifdef __cplusplus
}
#endif

#endif // SCRIBA_PROJECT_H
