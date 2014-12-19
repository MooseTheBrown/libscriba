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

#ifndef SCRIBA_DB_BACKEND_H
#define SCRIBA_DB_BACKEND_H

#include "company.h"
#include "event.h"
#include "poc.h"
#include "project.h"
#include "scriba.h"

// pointers to functions that must be implemented by a database backend
struct ScribaDBFuncTbl
{
    // company-related functions
    struct ScribaCompany* (*getCompany)(scriba_id_t);
    scriba_list_t* (*getAllCompanies)(void);
    scriba_list_t* (*getCompaniesByName)(const char *);
    scriba_list_t* (*getCompaniesByJurName)(const char *);
    scriba_list_t* (*getCompaniesByAddress)(const char *);
    void (*addCompany)(scriba_id_t, const char *, const char *,
                       const char *, scriba_inn_t, const char *,
                       const char *);
    void (*updateCompany)(const struct ScribaCompany *);
    void (*removeCompany)(scriba_id_t);

    // poc-related functions
    struct ScribaPoc* (*getPOC)(scriba_id_t);
    scriba_list_t* (*getAllPeople)(void);
    scriba_list_t* (*getPOCByName)(const char *);
    scriba_list_t* (*getPOCByCompany)(scriba_id_t);
    scriba_list_t* (*getPOCByPosition)(const char *);
    scriba_list_t* (*getPOCByPhoneNum)(const char *);
    scriba_list_t* (*getPOCByEmail)(const char *);
    void (*addPOC)(scriba_id_t, const char *, const char *, const char *,
                   const char *, const char *, const char *,
                   const char *, scriba_id_t);
    void (*updatePOC)(const struct ScribaPoc *);
    void (*removePOC)(scriba_id_t);

    // project-related functions
    struct ScribaProject* (*getProject)(scriba_id_t);
    scriba_list_t* (*getAllProjects)(void);
    scriba_list_t* (*getProjectsByTitle)(const char *);
    scriba_list_t* (*getProjectsByCompany)(scriba_id_t);
    scriba_list_t* (*getProjectsByState)(enum ScribaProjectState);
    void (*addProject)(scriba_id_t, const char *, const char *, scriba_id_t,
                       enum ScribaProjectState);
    void (*updateProject)(const struct ScribaProject *);
    void (*removeProject)(scriba_id_t);

    // event-related functions
    struct ScribaEvent* (*getEvent)(scriba_id_t);
    scriba_list_t* (*getAllEvents)(void);
    scriba_list_t* (*getEventsByDescr)(const char *);
    scriba_list_t* (*getEventsByCompany)(scriba_id_t);
    scriba_list_t* (*getEventsByPOC)(scriba_id_t);
    scriba_list_t* (*getEventsByProject)(scriba_id_t);
    void (*addEvent)(scriba_id_t, const char *, scriba_id_t, scriba_id_t,
                     scriba_id_t, enum ScribaEventType, const char *,
                     scriba_time_t, enum ScribaEventState);
    void (*updateEvent)(const struct ScribaEvent *);
    void (*removeEvent)(scriba_id_t);
};

// internal database backend
struct ScribaInternalDB
{
    char *name;
    // backend initialization function;
    // should return 0 on success
    int (*init)(struct ScribaDBParamList *, struct ScribaDBFuncTbl *);
    // backend cleanup function
    void (*cleanup)();
};

// add new internal database backend at run-time
// mostly useful for unit testing; must be called before scriba_init()
void scriba_addInternalDB(struct ScribaInternalDB *db);

#endif // SCRIBA_DB_BACKEND_H
