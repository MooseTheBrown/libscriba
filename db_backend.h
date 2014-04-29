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
    void (*addCompany)(const char *, const char *, const char *,
                       scriba_inn_t, const char *, const char *);
    void (*updateCompany)(const struct ScribaCompany *);
    void (*removeCompany)(scriba_id_t);

    // poc-related functions
    struct ScribaPoc* (*getPOC)(scriba_id_t);
    scriba_list_t* (*getAllPeople)(void);
    scriba_list_t* (*getPOCByName)(const char *, const char *, const char *);
    scriba_list_t* (*getPOCByCompany)(scriba_id_t);
    scriba_list_t* (*getPOCByPosition)(const char *);
    scriba_list_t* (*getPOCByPhoneNum)(const char *);
    scriba_list_t* (*getPOCByEmail)(const char *);
    void (*addPOC)(const char *, const char *, const char *,
                   const char *, const char *, const char *,
                   const char *, scriba_id_t);
    void (*updatePOC)(const struct ScribaPoc *);
    void (*removePOC)(scriba_id_t);

    // project-related functions
    struct ScribaProject* (*getProject)(scriba_id_t);
    scriba_list_t* (*getAllProjects)(void);
    scriba_list_t* (*getProjectsByCompany)(scriba_id_t);
    scriba_list_t* (*getProjectsByState)(enum ScribaProjectState);
    void (*addProject)(const char *, const char *, scriba_id_t,
                       enum ScribaProjectState);
    void (*updateProject)(const struct ScribaProject *);
    void (*removeProject)(scriba_id_t);

    // event-related functions
    struct ScribaEvent* (*getEvent)(scriba_id_t);
    scriba_list_t* (*getAllEvents)(void);
    scriba_list_t* (*getEventsByCompany)(scriba_id_t);
    scriba_list_t* (*getEventsByPOC)(scriba_id_t);
    scriba_list_t* (*getEventsByProject)(scriba_id_t);
    void (*addEvent)(const char *, scriba_id_t, scriba_id_t,
                     scriba_id_t, enum ScribaEventType, const char *,
                     scriba_time_t);
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