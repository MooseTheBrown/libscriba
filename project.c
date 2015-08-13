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

#include "project.h"
#include "db_backend.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern struct ScribaDBFuncTbl *fTbl;

// get project by id
struct ScribaProject *scriba_getProject(scriba_id_t id)
{
    return (fTbl->getProject(id));
}

// get all projects in the database
scriba_list_t *scriba_getAllProjects()
{
    return (fTbl->getAllProjects());
}

// search projects by title
scriba_list_t *scriba_getProjectsByTitle(const char *title)
{
    return (fTbl->getProjectsByTitle(title));
}

// get projects associated with given company
scriba_list_t *scriba_getProjectsByCompany(scriba_id_t id)
{
    return (fTbl->getProjectsByCompany(id));
}

// get projects with given state
scriba_list_t *scriba_getProjectsByState(enum ScribaProjectState state)
{
    return (fTbl->getProjectsByState(state));
}

// get projects by time
scriba_list_t *scriba_getProjectsByTime(scriba_time_t start_time, enum ScribaTimeComp start_comp,
                                        scriba_time_t mod_time, enum ScribaTimeComp mod_comp)
{
    return (fTbl->getProjectsByTime(start_time, start_comp, mod_time, mod_comp));
}

// get projects by state and time
scriba_list_t *scriba_getProjectsByStateTime(enum ScribaProjectState state,
                                             scriba_time_t start_time,
                                             enum ScribaTimeComp start_comp,
                                             scriba_time_t mod_time,
                                             enum ScribaTimeComp mod_comp)
{
    return (fTbl->getProjectsByStateTime(state, start_time, start_comp, mod_time, mod_comp));
}

// add project to the database
void scriba_addProject(const char *title, const char *descr, scriba_id_t company_id,
                       enum ScribaProjectState state, enum ScribaCurrency currency,
                       long long cost, scriba_time_t start_time)
{
    scriba_id_t id;

    scriba_id_create(&id);
    fTbl->addProject(id, title, descr, company_id, state, currency, cost, start_time);
}

// add project with given ID to the database
void scriba_addProjectWithID(scriba_id_t id, const char *title, const char *descr,
                             scriba_id_t company_id, enum ScribaProjectState state,
                             enum ScribaCurrency currency, long long cost,
                             scriba_time_t start_time)
{
    fTbl->addProject(id, title, descr, company_id, state, currency, cost, start_time);
}

// update project info
void scriba_updateProject(struct ScribaProject *project)
{
    struct ScribaProject *old_project = NULL;

    if (project == NULL)
    {
        return;
    }

    old_project = scriba_getProject(project->id);
    if (old_project->state != project->state)
    {
        // project state is being changed, update mod_time
        project->mod_time = (scriba_time_t)time(NULL);
    }
    scriba_freeProjectData(old_project);

    fTbl->updateProject(project);
}

// remove project from the database
void scriba_removeProject(scriba_id_t id)
{
    fTbl->removeProject(id);
}

// create a copy of project data structure
struct ScribaProject *scriba_copyProject(const struct ScribaProject *project)
{
    int len = 0;

    if (project == NULL)
    {
        return NULL;
    }

    struct ScribaProject *ret = (struct ScribaProject *)malloc(sizeof (struct ScribaProject));
    memset(ret, 0, sizeof (struct ScribaProject));

    scriba_id_copy(&(ret->id), &(project->id));
    if((len = strlen(project->title)) != 0)
    {
        ret->title = (char *)malloc(len + 1);
        memset(ret->title, 0, len + 1);
        strncpy(ret->title, project->title, len);
    }
    if((len = strlen(project->descr)) != 0)
    {
        ret->descr = (char *)malloc(len + 1);
        memset(ret->descr, 0, len + 1);
        strncpy(ret->descr, project->descr, len);
    }
    scriba_id_copy(&(ret->company_id), &(project->company_id));
    ret->state = project->state;
    ret->currency = project->currency;
    ret->cost = project->cost;
    ret->start_time = project->start_time;
    ret->mod_time = project->mod_time;

    return ret;
}

// free memory occupied by project data structure
void scriba_freeProjectData(struct ScribaProject *project)
{
    if (project == NULL)
    {
        return;
    }

    if (project->title != NULL)
    {
        free(project->title);
    }
    if (project->descr != NULL)
    {
        free(project->descr);
    }

    free(project);
}
