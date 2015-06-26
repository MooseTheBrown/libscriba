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

#include "sqlite_backend.h"
#include "sqlite3.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>



// database tables structure
#define CREATE_COMPANY_TABLE "CREATE TABLE Companies"\
    "("\
    "id BLOB PRIMARY KEY,"\
    "name TEXT COLLATE NOCASE,"\
    "jur_name TEXT COLLATE NOCASE,"\
    "address TEXT COLLATE NOCASE,"\
    "inn TEXT,"\
    "phonenum TEXT,"\
    "email TEXT"\
    ")"

#define COMPANY_TABLE_COLUMNS 7

#define CREATE_EVENT_TABLE "CREATE TABLE Events"\
    "("\
    "id BLOB PRIMARY KEY,"\
    "descr TEXT COLLATE NOCASE,"\
    "company_id BLOB,"\
    "poc_id BLOB,"\
    "project_id BLOB,"\
    "type INTEGER,"\
    "outcome TEXT,"\
    "timestamp INTEGER,"\
    "state INTEGER"\
    ")"\

#define EVENT_TABLE_COLUMNS 9

#define CREATE_POC_TABLE "CREATE TABLE People"\
    "("\
    "id BLOB PRIMARY KEY,"\
    "firstname TEXT COLLATE NOCASE,"\
    "secondname TEXT COLLATE NOCASE,"\
    "lastname TEXT COLLATE NOCASE,"\
    "mobilenum TEXT,"\
    "phonenum TEXT COLLATE NOCASE,"\
    "email TEXT COLLATE NOCASE,"\
    "position TEXT COLLATE NOCASE,"\
    "company_id BLOB"\
    ")"

#define POC_TABLE_COLUMNS 9

#define CREATE_PROJECT_TABLE "CREATE TABLE Projects"\
    "("\
    "id BLOB PRIMARY KEY,"\
    "title TEXT COLLATE NOCASE,"\
    "descr TEXT,"\
    "company_id BLOB,"\
    "state INTEGER"\
    ")"

#define PROJECT_TABLE_COLUMNS 5

#define ENABLE_SYNC "PRAGMA synchronous=2"
#define DISABLE_SYNC "PRAGMA synchronous=0"

struct ScribaSQLite
{
    sqlite3 *db;
    char *db_filename;
    int sync;
} *data = NULL;



struct ScribaInternalDB sqliteDB = 
{
    SCRIBA_SQLITE_BACKEND_NAME,
    scriba_sqlite_init,
    scriba_sqlite_cleanup
};



// backend init and cleanup
int scriba_sqlite_init(struct ScribaDBParamList *pl, struct ScribaDBFuncTbl *fTbl);
void scriba_sqlite_cleanup();



// process parameter list; returns 0 on success, 1 on failure
static int parse_param_list(struct ScribaDBParamList *pl);
// create new database; returns 0 on success, 1 on failure
static int create_database();
// configure SQLite sync mode
static int configure_sync();
// insert % at the beginning and at the end of search string for LIKE operator
static char *str_for_like_op(const char *src);

// create company data structure based on given parameters
static struct ScribaCompany *fillCompanyData(scriba_id_t id, const char *name,
                                             const char *jur_name, const char *addr,
                                             scriba_inn_t inn, const char *phonenum,
                                             const char *email);

// common company search routine
static scriba_list_t *companySearch(const char *query, const char *text);

// company handling interface functions
static struct ScribaCompany *getCompany(scriba_id_t id);
static scriba_list_t *getAllCompanies();
static scriba_list_t *getCompaniesByName(const char *name);
static scriba_list_t *getCompaniesByJurName(const char *juridicial_name);
static scriba_list_t *getCompaniesByAddress(const char *address);
static void addCompany(scriba_id_t id, const char *name, const char *jur_name,
                       const char *address, scriba_inn_t inn, const char *phonenum,
                       const char *email);
static void updateCompany(const struct ScribaCompany *company);
static void removeCompany(scriba_id_t id);

// create event data structure based on given parameters
static struct ScribaEvent *fillEventData(scriba_id_t id, const char *descr,
                                         scriba_id_t company_id, scriba_id_t poc_id,
                                         scriba_id_t project_id, enum ScribaEventType type,
                                         const char *outcome, scriba_time_t timestamp,
                                         enum ScribaEventState state);

// common event search routine
static scriba_list_t *eventSearch(const char *query, scriba_id_t id);

// event handling interface functions
static struct ScribaEvent *getEvent(scriba_id_t id);
static scriba_list_t *getAllEvents();
static scriba_list_t *getEventsByDescr(const char *descr);
static scriba_list_t *getEventsByCompany(scriba_id_t id);
static scriba_list_t *getEventsByPOC(scriba_id_t id);
static scriba_list_t *getEventsByProject(scriba_id_t id);
static void addEvent(scriba_id_t id, const char *descr, scriba_id_t company_id, scriba_id_t poc_id,
                     scriba_id_t project_id, enum ScribaEventType type, const char *outcome,
                     scriba_time_t timestamp, enum ScribaEventState state);
static void updateEvent(const struct ScribaEvent *event);
static void removeEvent(scriba_id_t id);

// create POC data structure based on given parameters
static struct ScribaPoc *fillPOCData(scriba_id_t id, const char *firstname,
                                     const char *secondname, const char *lastname,
                                     const char *mobilenum, const char *phonenum,
                                     const char *email, const char *position,
                                     scriba_id_t company_id);

// combine POC first, second and last names into a single string
static char *combine_poc_names(const char *firstname,
                               const char *secondname,
                               const char *lastname);

// execute prepared SQLite query and append results to given list
static void pocExecuteQuery(sqlite3_stmt *stmt, scriba_list_t *list);
// POC search by string routine
static scriba_list_t *pocSearchByStr(const char *query, const char *str);

// POC handling interface functions
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

// create project data structure based on given parameters
static struct ScribaProject *fillProjectData(scriba_id_t id, const char *title, const char *descr,
                                             scriba_id_t company_id, enum ScribaProjectState state);

// common project search routine
static scriba_list_t *projectSearch(sqlite3_stmt *stmt);

// project handling interface functions
static struct ScribaProject *getProject(scriba_id_t id);
static scriba_list_t *getAllProjects();
static scriba_list_t *getProjectsByTitle(const char *title);
static scriba_list_t *getProjectsByCompany(scriba_id_t id);
static scriba_list_t *getProjectsByState(enum ScribaProjectState state);
static void addProject(scriba_id_t id, const char *title, const char *descr,
                       scriba_id_t company_id, enum ScribaProjectState state);
static void updateProject(const struct ScribaProject *project);
static void removeProject(scriba_id_t id);



int scriba_sqlite_init(struct ScribaDBParamList *pl, struct ScribaDBFuncTbl *fTbl)
{
    if ((pl == NULL) || (fTbl == NULL))
    {
        goto error;
    }

    data = (struct ScribaSQLite *)malloc(sizeof (struct ScribaSQLite));
    memset(data, 0, sizeof (struct ScribaSQLite));
    data->sync = 1;     // sync is on by default

    if (parse_param_list(pl) != 0)
    {
        goto error;
    }

    // try to open database file using filename received via param list
    int err = sqlite3_open_v2(data->db_filename, &data->db, SQLITE_OPEN_READWRITE, NULL);
    if (err == SQLITE_CANTOPEN)
    {
        sqlite3_close(data->db);
        // given database does not exist, create new one
        if (create_database() != 0)
        {
            goto error;
        }
    }
    else if (err != SQLITE_OK)
    {
        goto error;
    }

    if (configure_sync() != 0)
    {
        goto error;
    }

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

success:
    return 0;

error:
    if (data != NULL)
    {
        if (data->db != NULL)
        {
            sqlite3_close(data->db);
        }
    }

    return 1;
}

void scriba_sqlite_cleanup()
{
    if (data != NULL)
    {
        if (data->db_filename != NULL)
        {
            free(data->db_filename);
        }

        if (data->db != NULL)
        {
            sqlite3_close(data->db);
        }

        free(data);
        data = NULL;
    }
}

static int parse_param_list(struct ScribaDBParamList *pl)
{
    int name_found = 0;

    do
    {
        struct ScribaDBParam *param = pl->param;
        if ((param->key == NULL) || (param->value == NULL))
        {
            continue;
        }
        else if ((strlen(param->key) == 0) || (strlen(param->value) == 0))
        {
            continue;
        }

        if (strcmp(param->key, SCRIBA_SQLITE_DB_LOCATION_PARAM) == 0)
        {
            name_found = 1;
            int len = strlen(param->value);
            data->db_filename = (char *)malloc(len + 1);
            memset(data->db_filename, 0, len + 1);
            strcpy(data->db_filename, param->value);
        }
        else if (strcmp(param->key, SCRIBA_SQLITE_DB_SYNC_PARAM) == 0)
        {
            if (strcmp(param->value, SCRIBA_SQLITE_DB_SYNC_OFF) == 0)
            {
                data->sync = 0;
            }
            else if (strcmp(param->value, SCRIBA_SQLITE_DB_SYNC_ON) == 0)
            {
                data->sync = 1;
            }
        }

        pl = pl->next;
    } while (pl != NULL);
 
    return !name_found;
}

// create new database; returns 0 on success, 1 on failure
static int create_database()
{
    if (data == NULL)
    {
        goto error;
    }

    if (sqlite3_open_v2(data->db_filename, &(data->db),
                        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL) != SQLITE_OK)
    {
        goto error;
    }

    if (sqlite3_exec(data->db, CREATE_COMPANY_TABLE, NULL, NULL, NULL) != SQLITE_OK)
    {
        goto error;
    }
    if (sqlite3_exec(data->db, CREATE_EVENT_TABLE, NULL, NULL, NULL) != SQLITE_OK)
    {
        goto error;
    }
    if (sqlite3_exec(data->db, CREATE_POC_TABLE, NULL, NULL, NULL) != SQLITE_OK)
    {
        goto error;
    }
    if (sqlite3_exec(data->db, CREATE_PROJECT_TABLE, NULL, NULL, NULL) != SQLITE_OK)
    {
        goto error;
    }

success:
    return 0;

error:
    return 1;
}

// configure SQLite sync mode
static int configure_sync()
{
    if (data == NULL)
    {
        goto error;
    }

    if (data->sync == 0)
    {
        if (sqlite3_exec(data->db, DISABLE_SYNC, NULL, NULL, NULL) != SQLITE_OK)
        {
            goto error;
        }
    }
    else
    {
        if (sqlite3_exec(data->db, ENABLE_SYNC, NULL, NULL, NULL) != SQLITE_OK)
        {
            goto error;
        }
    }

    return 0;

error:
    return 1;
}

// insert % at the beginning and at the end of search string for LIKE operator
static char *str_for_like_op(const char *src)
{
    if (src == NULL)
    {
        return NULL;
    }

    int len = strlen(src) + 3; // 2 '%' and terminating 0
    char *result = (char *)malloc(len);
    snprintf(result, len, "%%%s%%", src);
    return result;
}

// create company data structure based on given parameters
static struct ScribaCompany *fillCompanyData(scriba_id_t id, const char *name,
                                             const char *jur_name, const char *addr,
                                             scriba_inn_t inn, const char *phonenum,
                                             const char *email)
{
    int len = 0;
    struct ScribaCompany *company = (struct ScribaCompany *)malloc(sizeof (struct ScribaCompany));
    if (company == NULL)
    {
        return NULL;
    }
    memset((void *)company, 0, sizeof (struct ScribaCompany));

    scriba_id_copy(&(company->id), &id);
    if (name != NULL)
    {
        len = strlen(name);
        if (len != 0)
        {
            company->name = (char *)malloc(len + 1);
            strcpy(company->name, name);
        }
    }
    if (jur_name != NULL)
    {
        len = strlen(jur_name);
        if (len != 0)
        {
            company->jur_name = (char *)malloc(len + 1);
            strcpy(company->jur_name, jur_name);
        }
    }
    if (addr != NULL)
    {
        len = strlen(addr);
        if (len != 0)
        {
            company->address = (char *)malloc(len + 1);
            strcpy(company->address, addr);
        }
    }
    scriba_copy_inn(&(company->inn), &inn);
    if (phonenum != NULL)
    {
        len = strlen(phonenum);
        if (len != 0)
        {
            company->phonenum = (char *)malloc(len + 1);
            strcpy(company->phonenum, phonenum);
        }
    }
    if (email != NULL)
    {
        len = strlen(email);
        if (len != 0)
        {
            company->email = (char *)malloc(len + 1);
            strcpy(company->email, email);
        }
    }

    return company;
}

// common company search routine
static scriba_list_t *companySearch(const char *query, const char *text)
{
    scriba_list_t *companies = scriba_list_init();
    sqlite3_stmt *stmt = NULL;
    char *company_name = NULL;

    if ((query == NULL) || (data == NULL))
    {
        goto exit;
    }

    // prepare query
    if (sqlite3_prepare_v2(data->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        goto exit;
    }
    // if text is NULL, we assume the query does not contain any placeholders
    if (text != NULL)
    {
        if (sqlite3_bind_text(stmt, 1, text, -1, SQLITE_TRANSIENT) != SQLITE_OK)
        {
            goto exit;
        }
    }

    // execute query
    while (1)
    {
        int ret = sqlite3_step(stmt);
        if (ret == SQLITE_BUSY)
        {
            // retry
            continue;
        }
        else if (ret == SQLITE_ROW)
        {
            // we have the data
            // id and name fields should be selected from the table
            scriba_id_t id;
            scriba_id_from_blob(sqlite3_column_blob(stmt, 0), &id);
            const char *name = sqlite3_column_text(stmt, 1);
            if (name != NULL)
            {
                int len = strlen(name);
                company_name = (char *)malloc(len + 1);
                strcpy(company_name, name);
                company_name[len] = 0;
                scriba_list_add(companies, id, company_name);
                free(company_name);
            }
            else
            {
                scriba_list_add(companies, id, NULL);
            }
        }
        else if (ret == SQLITE_DONE)
        {
            break;
        }
        else
        {
            // this should not happen
            goto exit;
        }
    }

exit:
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
    }
    return companies;
}

// company search functions
static struct ScribaCompany *getCompany(scriba_id_t id)
{
    struct ScribaCompany *company = NULL;
    char query[512];
    sqlite3_stmt *sqlite_stmt = NULL;
    void *id_blob = scriba_id_to_blob(&id);

    if (data == NULL)
    {
        goto error;
    }

    // prepare query
    sprintf(query, "SELECT * FROM Companies WHERE id=?");

    if (sqlite3_prepare_v2(data->db, query, -1, &sqlite_stmt, NULL) != SQLITE_OK)
    {
        goto error;
    }
    if (sqlite3_bind_blob(sqlite_stmt,
                          1,
                          id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto error;
    }

    // execute query
    while (1)
    {
        int ret = sqlite3_step(sqlite_stmt);
        if (ret == SQLITE_BUSY)
        {
            // retry
            continue;
        }
        else if (ret == SQLITE_ROW)
        {
            // we have the data
            break;
        }
        else
        {
            // something went wrong or simply nothing was found
            goto error;
        }
    }

    if (sqlite3_column_count(sqlite_stmt) != COMPANY_TABLE_COLUMNS)
    {
        goto error;
    }

    const char *name = (const char *)sqlite3_column_text(sqlite_stmt, 1);
    const char *jur_name = (const char *)sqlite3_column_text(sqlite_stmt, 2);
    const char *address = (const char *)sqlite3_column_text(sqlite_stmt, 3);
    scriba_inn_t inn = scriba_inn_from_string((const char *)sqlite3_column_text(sqlite_stmt, 4));
    const char *phonenum = (const char *)sqlite3_column_text(sqlite_stmt, 5);
    const char *email = (const char *)sqlite3_column_text(sqlite_stmt, 6);

    company = fillCompanyData(id, name, jur_name, address, inn, phonenum, email);
    company->poc_list = getPOCByCompany(id);
    company->proj_list = getProjectsByCompany(id);
    company->event_list = getEventsByCompany(id);

    sqlite3_finalize(sqlite_stmt);
    if (id_blob != NULL)
    {
        free(id_blob);
    }
    return company;

error:
    if (company != NULL)
    {
        free(company);
    }
    if (sqlite_stmt != NULL)
    {
        sqlite3_finalize(sqlite_stmt);
    }
    if (id_blob != NULL)
    {
        free(id_blob);
    }

    return NULL;
}

static scriba_list_t *getAllCompanies()
{
    return companySearch("SELECT id, name FROM Companies", NULL);
}

static scriba_list_t *getCompaniesByName(const char *name)
{
    scriba_list_t *ret;
    char *search = str_for_like_op(name);

    ret = companySearch("SELECT id, name FROM Companies WHERE name LIKE ?", search);
    free(search);

    return ret;
}

static scriba_list_t *getCompaniesByJurName(const char *juridicial_name)
{
    scriba_list_t *ret;
    char *search = str_for_like_op(juridicial_name);

    ret = companySearch("SELECT id, name FROM Companies WHERE jur_name LIKE ?", search);
    free(search);

    return ret;
}

static scriba_list_t *getCompaniesByAddress(const char *address)
{
    scriba_list_t *ret;
    char *search = str_for_like_op(address);

    ret = companySearch("SELECT id, name FROM Companies WHERE address LIKE ?", search);
    free(search);

    return ret;
}

static void addCompany(scriba_id_t id, const char *name, const char *jur_name,
                       const char *address, scriba_inn_t inn, const char *phonenum,
                       const char *email)
{
    sqlite3_stmt *stmt = NULL;
    char query[] = "INSERT INTO Companies(id, name, jur_name, address, inn, phonenum, email) "
                   "VALUES(?,?,?,?,?,?,?)";
    char *inn_str = NULL;
    void *id_blob = NULL;

    if (data == NULL)
    {
        goto exit;
    }

    id_blob = scriba_id_to_blob(&id);

    if (sqlite3_prepare_v2(data->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_blob(stmt,
                          1,
                          id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 2, name, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 3, jur_name, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 4, address, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    inn_str = scriba_inn_to_string(&inn);
    if (sqlite3_bind_text(stmt, 5, inn_str, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 6, phonenum, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 7, email, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }

    // execute query
    while (1)
    {
        int ret = sqlite3_step(stmt);
        if (ret == SQLITE_BUSY)
        {
            // retry
            continue;
        }
        else
        {
            break;
        }
    }

exit:
    if (inn_str != NULL)
    {
        free(inn_str);
    }
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
    }
    if (id_blob != NULL)
    {
        free(id_blob);
    }
}

static void updateCompany(const struct ScribaCompany *company)
{
    sqlite3_stmt *stmt = NULL;
    char query[] = "UPDATE Companies SET name=?,jur_name=?,address=?,inn=?,phonenum=?,email=?"
                   "WHERE id=?";
    char *inn_str = NULL;
    void *id_blob = NULL;

    if ((company == NULL) || (data == NULL))
    {
        goto exit;
    }

    if (sqlite3_prepare_v2(data->db, query , -1, &stmt, NULL) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 1, company->name, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 2, company->jur_name, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 3, company->address, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    inn_str = scriba_inn_to_string(&(company->inn));
    if (sqlite3_bind_text(stmt, 4, inn_str, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 5, company->phonenum, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 6, company->email, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    id_blob = scriba_id_to_blob(&(company->id));
    if (sqlite3_bind_blob(stmt,
                          7,
                          id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }

    // execute query
    while (1)
    {
        int ret = sqlite3_step(stmt);
        if (ret == SQLITE_BUSY)
        {
            // retry
            continue;
        }
        else
        {
            break;
        }
    }

exit:
    if (inn_str != NULL)
    {
        free(inn_str);
    }
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
    }
    if (id_blob != NULL)
    {
        free(id_blob);
    }
}

static void removeCompany(scriba_id_t id)
{
    sqlite3_stmt *stmt = NULL;
    char query[] = "DELETE FROM Companies WHERE id=?";
    void *id_blob = NULL;

    if (data == NULL)
    {
        goto exit;
    }

    if (sqlite3_prepare_v2(data->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        goto exit;
    }

    id_blob = scriba_id_to_blob(&id);
    if (sqlite3_bind_blob(stmt,
                          1,
                          id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }

    // execute query
    while (1)
    {
        int ret = sqlite3_step(stmt);
        if (ret == SQLITE_BUSY)
        {
            // retry
            continue;
        }
        else
        {
            break;
        }
    }

exit:
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
    }
    if (id_blob != NULL)
    {
        free(id_blob);
    }
}

// create event data structure based on given parameters
static struct ScribaEvent *fillEventData(scriba_id_t id, const char *descr,
                                         scriba_id_t company_id, scriba_id_t poc_id,
                                         scriba_id_t project_id, enum ScribaEventType type,
                                         const char *outcome, scriba_time_t timestamp,
                                         enum ScribaEventState state)
{
    int len = 0;
    struct ScribaEvent *ret = (struct ScribaEvent *)malloc(sizeof (struct ScribaEvent));

    if (ret == NULL)
    {
        return NULL;
    }
    memset(ret, 0, sizeof (struct ScribaEvent));

    scriba_id_copy(&(ret->id), &id);
    if (descr != NULL)
    {
        len = strlen(descr);
        ret->descr = (char *)malloc(len + 1);
        strcpy(ret->descr, descr);
    }
    scriba_id_copy(&(ret->company_id), &company_id);
    scriba_id_copy(&(ret->poc_id), &poc_id);
    scriba_id_copy(&(ret->project_id), &project_id);
    ret->type = type;
    if (outcome != NULL)
    {
        len = strlen(outcome);
        ret->outcome = (char *)malloc(len + 1);
        strcpy(ret->outcome, outcome);
    }
    ret->timestamp = timestamp;
    ret->state = state;

    return ret;
}

// common event search routine
static scriba_list_t *eventSearch(const char *query, scriba_id_t id)
{
    scriba_list_t *events = scriba_list_init();
    sqlite3_stmt *stmt = NULL;
    void *id_blob = NULL;

    if ((data == NULL) || (query == NULL))
    {
        goto exit;
    }

    // prepare query
    if (sqlite3_prepare_v2(data->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        goto exit;
    }

    id_blob = scriba_id_to_blob(&id);
    if (sqlite3_bind_blob(stmt,
                          1,
                          id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }

    // execute query and retrieve results
    while (1)
    {
        int ret = sqlite3_step(stmt);
        if (ret == SQLITE_BUSY)
        {
            // retry
            continue;
        }
        else if (ret == SQLITE_ROW)
        {
            // we have the data
            // event id and description fields are selected from the table
            scriba_id_t id;
            scriba_id_from_blob(sqlite3_column_blob(stmt, 0), &id);
            const char *descr = sqlite3_column_text(stmt, 1);
            int len = strlen(descr);
            if (len > 0)
            {
                char *event_descr = (char *)malloc(len + 1);
                strcpy(event_descr, descr);
                scriba_list_add(events, id, event_descr);
                free(event_descr);
            }
            else
            {
                scriba_list_add(events, id, NULL);
            }
        }
        else if (ret == SQLITE_DONE)
        {
            break;
        }
        else
        {
            // this should not happen
            goto exit;
        }
    }

exit:
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
    }
    if (id_blob != NULL)
    {
        free(id_blob);
    }
    return events;
}

static struct ScribaEvent *getEvent(scriba_id_t id)
{
    char query[] = "SELECT * FROM Events WHERE id=?";
    sqlite3_stmt *stmt = NULL;
    void *id_blob = NULL;

    if (data == NULL)
    {
        goto error;
    }

    // prepare query
    if (sqlite3_prepare_v2(data->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        goto error;
    }

    id_blob = scriba_id_to_blob(&id);
    if (sqlite3_bind_blob(stmt,
                          1,
                          id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto error;
    }

    // execute query
    while (1)
    {
        int ret = sqlite3_step(stmt);
        if (ret == SQLITE_BUSY)
        {
            // retry
            continue;
        }
        else if (ret == SQLITE_ROW)
        {
            // we have the data
            break;
        }
        else
        {
            // this should not happen
            goto error;
        }
    }

    if (sqlite3_column_count(stmt) != EVENT_TABLE_COLUMNS)
    {
        goto error;
    }

    const char *descr = sqlite3_column_text(stmt, 1);
    scriba_id_t company_id;
    scriba_id_from_blob(sqlite3_column_blob(stmt, 2), &company_id);
    scriba_id_t poc_id;
    scriba_id_from_blob(sqlite3_column_blob(stmt, 3), &poc_id);
    scriba_id_t project_id;
    scriba_id_from_blob(sqlite3_column_blob(stmt, 4), &project_id);
    enum ScribaEventType type = (enum ScribaEventType)sqlite3_column_int(stmt, 5);
    const char *outcome = sqlite3_column_text(stmt, 6);
    scriba_time_t timestamp = (scriba_time_t)sqlite3_column_int64(stmt, 7);
    enum ScribaEventState state = (enum ScribaEventState)sqlite3_column_int64(stmt, 8);

    struct ScribaEvent *event = fillEventData(id, descr, company_id, poc_id,
                                              project_id, type, outcome, timestamp,
                                              state);
    sqlite3_finalize(stmt);
    if (id_blob != NULL)
    {
        free(id_blob);
    }
    return event;

error:
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
    }
    if (id_blob != NULL)
    {
        free(id_blob);
    }

    return NULL;
}

static scriba_list_t *getAllEvents()
{
    scriba_list_t *events = scriba_list_init();
    sqlite3_stmt *stmt = NULL;
    char query[] = "SELECT id,descr FROM Events";

    if (data == NULL)
    {
        goto exit;
    }

    // prepare query
    if (sqlite3_prepare_v2(data->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        goto exit;
    }

    // execute query and retrieve results
    while (1)
    {
        int ret = sqlite3_step(stmt);
        if (ret == SQLITE_BUSY)
        {
            // retry
            continue;
        }
        else if (ret == SQLITE_ROW)
        {
            // we have the data
            scriba_id_t id;
            scriba_id_from_blob(sqlite3_column_blob(stmt, 0), &id);
            const char *descr = sqlite3_column_text(stmt, 1);
            int len = strlen(descr);
            if (len > 0)
            {
                char *event_descr = (char *)malloc(len + 1);
                strcpy(event_descr, descr);
                scriba_list_add(events, id, event_descr);
                free(event_descr);
            }
            else
            {
                scriba_list_add(events, id, NULL);
            }
        }
        else if (ret == SQLITE_DONE)
        {
            break;
        }
        else
        {
            // this should not happen
            goto exit;
        }
    }

exit:
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
    }
    return events;
}

static scriba_list_t *getEventsByDescr(const char *descr)
{
    scriba_list_t *events = scriba_list_init();
    char *search = str_for_like_op(descr);
    sqlite3_stmt *stmt = NULL;
    char query[] = "SELECT id,descr FROM Events WHERE descr LIKE ?";

    if (data == NULL)
    {
        goto exit;
    }

    // prepare query
    if (sqlite3_prepare_v2(data->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        goto exit;
    }

    if (sqlite3_bind_text(stmt, 1, search, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }

    // execute query and retrieve results
    while (1)
    {
        int ret = sqlite3_step(stmt);
        if (ret == SQLITE_BUSY)
        {
            // retry
            continue;
        }
        else if (ret == SQLITE_ROW)
        {
            // we have the data
            scriba_id_t id;
            scriba_id_from_blob(sqlite3_column_blob(stmt, 0), &id);
            const char *descr = sqlite3_column_text(stmt, 1);
            int len = strlen(descr);
            if (len > 0)
            {
                char *event_descr = (char *)malloc(len + 1);
                strcpy(event_descr, descr);
                scriba_list_add(events, id, event_descr);
                free(event_descr);
            }
            else
            {
                scriba_list_add(events, id, NULL);
            }
        }
        else if (ret == SQLITE_DONE)
        {
            break;
        }
        else
        {
            // this should not happen
            goto exit;
        }
    }

exit:
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
    }
    if (search != NULL)
    {
        free(search);
    }
    return events;
}

static scriba_list_t *getEventsByCompany(scriba_id_t id)
{
    return eventSearch("SELECT id,descr FROM Events WHERE company_id=?", id);
}

static scriba_list_t *getEventsByPOC(scriba_id_t id)
{
    return eventSearch("SELECT id,descr FROM Events WHERE poc_id=?", id);
}

static scriba_list_t *getEventsByProject(scriba_id_t id)
{
    return eventSearch("SELECT id,descr FROM Events WHERE project_id=?", id);
}

static void addEvent(scriba_id_t id, const char *descr, scriba_id_t company_id, scriba_id_t poc_id,
                     scriba_id_t project_id, enum ScribaEventType type, const char *outcome,
                     scriba_time_t timestamp, enum ScribaEventState state)
{
    char query[] = "INSERT INTO Events (id,descr,company_id,poc_id,project_id,type,outcome, timestamp, state)"
                   "VALUES (?,?,?,?,?,?,?,?,?)";
    sqlite3_stmt *stmt = NULL;
    void *id_blob = NULL;
    void *company_id_blob = NULL;
    void *poc_id_blob = NULL;
    void *project_id_blob = NULL;

    if (data == NULL)
    {
        goto exit;
    }

    id_blob = scriba_id_to_blob(&id);

    // prepare query
    if (sqlite3_prepare_v2(data->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_blob(stmt,
                          1,
                          id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 2, descr, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    company_id_blob = scriba_id_to_blob(&company_id);
    if (sqlite3_bind_blob(stmt,
                          3,
                          company_id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    poc_id_blob = scriba_id_to_blob(&poc_id);
    if (sqlite3_bind_blob(stmt,
                          4,
                          poc_id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    project_id_blob = scriba_id_to_blob(&project_id);
    if (sqlite3_bind_blob(stmt,
                          5,
                          project_id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_int(stmt, 6, (int)type) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 7, outcome, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_int64(stmt, 8, (sqlite_int64)timestamp) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_int64(stmt, 9, (sqlite_int64)state) != SQLITE_OK)
    {
        goto exit;
    }

    // execute query
    while (1)
    {
        int ret = sqlite3_step(stmt);
        if (ret == SQLITE_BUSY)
        {
            // retry
            continue;
        }
        else
        {
            break;
        }
    }

exit:
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
    }
    if (id_blob != NULL)
    {
        free(id_blob);
    }
    if (company_id_blob != NULL)
    {
        free(company_id_blob);
    }
    if (poc_id_blob != NULL)
    {
        free(poc_id_blob);
    }
    if (project_id_blob != NULL)
    {
        free(project_id_blob);
    }
}

static void updateEvent(const struct ScribaEvent *event)
{
    char query[] = "UPDATE Events SET descr=?,company_id=?,poc_id=?,project_id=?,"
                   "type=?,outcome=?,timestamp=?,state=? WHERE id=?";
    sqlite3_stmt *stmt = NULL;
    void *id_blob = NULL;
    void *company_id_blob = NULL;
    void *poc_id_blob = NULL;
    void *project_id_blob = NULL;

    if ((event == NULL) || (data == NULL))
    {
        goto exit;
    }

    // prepare query
    if (sqlite3_prepare_v2(data->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 1, event->descr, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    company_id_blob = scriba_id_to_blob(&(event->company_id));
    if (sqlite3_bind_blob(stmt,
                          2,
                          company_id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    poc_id_blob = scriba_id_to_blob(&(event->poc_id));
    if (sqlite3_bind_blob(stmt,
                          3,
                          poc_id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    project_id_blob = scriba_id_to_blob(&(event->project_id));
    if (sqlite3_bind_blob(stmt,
                          4,
                          project_id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_int(stmt, 5, (int)(event->type)) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 6, event->outcome, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_int64(stmt, 7, (sqlite3_int64)(event->timestamp)) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_int64(stmt, 8, (sqlite3_int64)(event->state)) != SQLITE_OK)
    {
        goto exit;
    }
    id_blob = scriba_id_to_blob(&(event->id));
    if (sqlite3_bind_blob(stmt,
                          9,
                          id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }

    // execute query
    while (1)
    {
        int ret = sqlite3_step(stmt);
        if (ret == SQLITE_BUSY)
        {
            // retry
            continue;
        }
        else
        {
            break;
        }
    }

exit:
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
    }
    if (id_blob != NULL)
    {
        free(id_blob);
    }
    if (company_id_blob != NULL)
    {
        free(company_id_blob);
    }
    if (poc_id_blob != NULL)
    {
        free(poc_id_blob);
    }
    if (project_id_blob != NULL)
    {
        free(project_id_blob);
    }
}

static void removeEvent(scriba_id_t id)
{
    sqlite3_stmt *stmt = NULL;
    char query[] = "DELETE FROM Events WHERE id=?";
    void *id_blob = NULL;

    if (data == NULL)
    {
        goto exit;
    }

    if (sqlite3_prepare_v2(data->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        goto exit;
    }

    id_blob = scriba_id_to_blob(&id);
    if (sqlite3_bind_blob(stmt,
                          1,
                          id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }

    // execute query
    while (1)
    {
        int ret = sqlite3_step(stmt);
        if (ret == SQLITE_BUSY)
        {
            // retry
            continue;
        }
        else
        {
            break;
        }
    }

exit:
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
    }
    if (id_blob != NULL)
    {
        free(id_blob);
    }
}

// create POC data structure based on given parameters
static struct ScribaPoc *fillPOCData(scriba_id_t id, const char *firstname,
                                     const char *secondname, const char *lastname,
                                     const char *mobilenum, const char *phonenum,
                                     const char *email, const char *position,
                                     scriba_id_t company_id)
{
    int len = 0;
    struct ScribaPoc *poc = (struct ScribaPoc *)malloc(sizeof (struct ScribaPoc));
    if (poc == NULL)
    {
        return NULL;
    }

    memset(poc, 0, sizeof (struct ScribaPoc));

    scriba_id_copy(&(poc->id), &id);
    if (firstname != NULL)
    {
        len = strlen(firstname);
        poc->firstname = (char *)malloc(len + 1);
        strcpy(poc->firstname, firstname);
    }
    if (secondname != NULL)
    {
        len = strlen(secondname);
        poc->secondname = (char *)malloc(len + 1);
        strcpy(poc->secondname, secondname);
    }
    if (lastname != NULL)
    {
        len = strlen(lastname);
        poc->lastname = (char *)malloc(len + 1);
        strcpy(poc->lastname, lastname);
    }
    if (mobilenum != NULL)
    {
        len = strlen(mobilenum);
        poc->mobilenum = (char *)malloc(len + 1);
        strcpy(poc->mobilenum, mobilenum);
    }
    if (phonenum != NULL)
    {
        len = strlen(phonenum);
        poc->phonenum = (char *)malloc(len + 1);
        strcpy(poc->phonenum, phonenum);
    }
    if (email != NULL)
    {
        len = strlen(email);
        poc->email = (char *)malloc(len + 1);
        strcpy(poc->email, email);
    }
    if (position != NULL)
    {
        len = strlen(position);
        poc->position = (char *)malloc(len + 1);
        strcpy(poc->position, position);
    }
    scriba_id_copy(&(poc->company_id), &company_id);

    return poc;
}

// combine POC first, second and last names into a single string
static char *combine_poc_names(const char *firstname,
                               const char *secondname,
                               const char *lastname)
{
    int len = strlen(firstname) + strlen(secondname) + strlen(lastname);
    if (len == 0)
    {
        // nothing to combine
        return NULL;
    }

    // we need two additional spaces and a null character
    char *result = (char *)malloc(len + 3);
    strcpy(result, firstname);
    strcat(result, " ");
    strcat(result, secondname);
    strcat(result, " ");
    strcat(result, lastname);

    return result;
}

// execute prepared SQLite query and append results to given list
static void pocExecuteQuery(sqlite3_stmt *stmt, scriba_list_t *list)
{
    if ((stmt == NULL) || (list == NULL))
    {
        return;
    }

    while (1)
    {
        int ret = sqlite3_step(stmt);
        if (ret == SQLITE_BUSY)
        {
            // retry
            continue;
        }
        else if (ret == SQLITE_ROW)
        {
            // we have the data
            scriba_id_t id;
            scriba_id_from_blob(sqlite3_column_blob(stmt, 0), &id);
            const char *firstname = sqlite3_column_text(stmt, 1);
            const char *secondname = sqlite3_column_text(stmt, 2);
            const char *lastname = sqlite3_column_text(stmt, 3);
            char *name = combine_poc_names(firstname, secondname, lastname);
            scriba_list_add(list, id, name);
            free(name);
        }
        else
        {
            break;
        }
    }
}

// POC search by string routine
static scriba_list_t *pocSearchByStr(const char *query, const char *str)
{
    scriba_list_t *people = scriba_list_init();
    sqlite3_stmt *stmt = NULL;

    if ((data == NULL) || (query == NULL))
    {
        goto exit;
    }

    // prepare query
    if (sqlite3_prepare_v2(data->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        goto exit;
    }
    if (str != NULL)
    {
        if (sqlite3_bind_text(stmt, 1, str, -1, SQLITE_TRANSIENT) != SQLITE_OK)
        {
            goto exit;
        }
    }

    pocExecuteQuery(stmt, people);

exit:
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
    }
    return people;
}

// POC handling interface functions
static struct ScribaPoc *getPOC(scriba_id_t id)
{
    struct ScribaPoc *poc = NULL;
    char query[] = "SELECT * FROM People WHERE id=?";
    sqlite3_stmt *stmt = NULL;
    void *id_blob = NULL;

    if (data == NULL)
    {
        goto error;
    }

    // prepare query
    if (sqlite3_prepare_v2(data->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        goto error;
    }
    id_blob = scriba_id_to_blob(&id);
    if (sqlite3_bind_blob(stmt,
                          1,
                          id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto error;
    }

    // execute query
    while (1)
    {
        int ret = sqlite3_step(stmt);
        if (ret == SQLITE_BUSY)
        {
            // retry
            continue;
        }
        else if (ret == SQLITE_ROW)
        {
            // we have the data
            break;
        }
        else
        {
            // this should not happen
            goto error;
        }
    }

    if (sqlite3_column_count(stmt) != POC_TABLE_COLUMNS)
    {
        goto error;
    }

    scriba_id_t poc_id;
    scriba_id_from_blob(sqlite3_column_blob(stmt, 0), &poc_id);
    const char *firstname = (const char *)sqlite3_column_text(stmt, 1);
    const char *secondname = (const char *)sqlite3_column_text(stmt, 2);
    const char *lastname = (const char *)sqlite3_column_text(stmt, 3);
    const char *mobilenum = (const char *)sqlite3_column_text(stmt, 4);
    const char *phonenum = (const char *)sqlite3_column_text(stmt, 5);
    const char *email = (const char *)sqlite3_column_text(stmt, 6);
    const char *position = (const char *)sqlite3_column_text(stmt, 7);
    scriba_id_t company_id;
    scriba_id_from_blob(sqlite3_column_blob(stmt, 8), &company_id);

    poc = fillPOCData(poc_id, firstname, secondname, lastname, mobilenum, phonenum,
                      email, position, company_id);
    sqlite3_finalize(stmt);
    if (id_blob != NULL)
    {
        free(id_blob);
    }
    return poc;

error:
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
    }
    if (id_blob != NULL)
    {
        free(id_blob);
    }

    return NULL;
}

static scriba_list_t *getAllPeople()
{
    scriba_list_t *people = scriba_list_init();
    sqlite3_stmt *stmt = NULL;
    char query[1024] = "SELECT id,firstname,secondname,lastname FROM People";

    if (data == NULL)
    {
        goto exit;
    }

    if (sqlite3_prepare_v2(data->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        goto exit;
    }

    pocExecuteQuery(stmt, people);

exit:
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
    }
    return people;
}

static scriba_list_t *getPOCByName(const char *name)
{
    scriba_list_t *people = scriba_list_init();
    char *search = str_for_like_op(name);
    sqlite3_stmt *stmt_first = NULL;
    sqlite3_stmt *stmt_second = NULL;
    sqlite3_stmt *stmt_last = NULL;
    char query_first[] =
        "SELECT id,firstname,secondname,lastname FROM People WHERE firstname LIKE ?";
    char query_second[] =
        "SELECT id,firstname,secondname,lastname FROM People WHERE secondname LIKE ?";
    char query_last[] =
        "SELECT id,firstname,secondname,lastname FROM People WHERE lastname LIKE ?";

    if (data == NULL)
    {
        goto exit;
    }
    if (name == NULL)
    {
        goto exit;
    }

    // prepare queries for first, second and last name search
    if (sqlite3_prepare_v2(data->db, query_first, -1, &stmt_first, NULL) != SQLITE_OK)
    {
        goto exit;
    }

    if (sqlite3_bind_text(stmt_first, 1, search, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }

    if (sqlite3_prepare_v2(data->db, query_second, -1, &stmt_second, NULL) != SQLITE_OK)
    {
        goto exit;
    }

    if (sqlite3_bind_text(stmt_second, 1, search, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }

    if (sqlite3_prepare_v2(data->db, query_last, -1, &stmt_last, NULL) != SQLITE_OK)
    {
        goto exit;
    }

    if (sqlite3_bind_text(stmt_last, 1, search, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }

    pocExecuteQuery(stmt_first, people);
    pocExecuteQuery(stmt_second, people);
    pocExecuteQuery(stmt_last, people);

exit:
    if (stmt_first != NULL)
    {
        sqlite3_finalize(stmt_first);
    }
    if (stmt_second != NULL)
    {
        sqlite3_finalize(stmt_second);
    }
    if (stmt_last != NULL)
    {
        sqlite3_finalize(stmt_last);
    }
    if (search != NULL)
    {
        free(search);
    }
    return people;
}

static scriba_list_t *getPOCByCompany(scriba_id_t id)
{
    scriba_list_t *people = scriba_list_init();
    sqlite3_stmt *stmt = NULL;
    char query[] = "SELECT id,firstname,secondname,lastname FROM People WHERE company_id=?";
    void *id_blob = NULL;

    if (data == NULL)
    {
        goto exit;
    }

    // prepare query
    if (sqlite3_prepare_v2(data->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        goto exit;
    }
    id_blob = scriba_id_to_blob(&id);
    if (sqlite3_bind_blob(stmt,
                          1,
                          id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }

    pocExecuteQuery(stmt, people);

exit:
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
    }
    if (id_blob != NULL)
    {
        free(id_blob);
    }
    return people;
}

static scriba_list_t *getPOCByPosition(const char *position)
{
    char *search = str_for_like_op(position);
    scriba_list_t *ret;
    ret = pocSearchByStr("SELECT id,firstname,secondname,lastname FROM People WHERE position LIKE ?",
                         search);

    if (search != NULL)
    {
        free(search);
    }
    return ret;
}

static scriba_list_t *getPOCByPhoneNum(const char *phonenum)
{
    return pocSearchByStr("SELECT id,firstname,secondname,lastname FROM People WHERE phonenum=?",
                          phonenum);
}

static scriba_list_t *getPOCByEmail(const char *email)
{
    char *search = str_for_like_op(email);
    scriba_list_t *ret;
    ret = pocSearchByStr("SELECT id,firstname,secondname,lastname FROM People WHERE email LIKE ?",
                         search);

    if (search != NULL)
    {
        free(search);
    }
    return ret;
}

static void addPOC(scriba_id_t id, const char *firstname, const char *secondname,
                   const char *lastname, const char *mobilenum, const char *phonenum,
                   const char *email, const char *position, scriba_id_t company_id)
{
    sqlite3_stmt *stmt = NULL;
    char query[] = "INSERT INTO People(id, firstname, secondname, lastname, mobilenum, phonenum,"
                   "email,position,company_id) VALUES(?,?,?,?,?,?,?,?,?)";
    void *id_blob = NULL;
    void *company_id_blob = NULL;

    if (data == NULL)
    {
        goto exit;
    }

    id_blob = scriba_id_to_blob(&id);

    // prepare query
    if (sqlite3_prepare_v2(data->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_blob(stmt,
                          1,
                          id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 2, firstname, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 3, secondname, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 4, lastname, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 5, mobilenum, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 6, phonenum, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 7, email, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 8, position, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    company_id_blob = scriba_id_to_blob(&company_id);
    if (sqlite3_bind_blob(stmt,
                          9,
                          company_id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }

    // execute query
    while (1)
    {
        int ret = sqlite3_step(stmt);
        if (ret == SQLITE_BUSY)
        {
            // retry
            continue;
        }
        else
        {
            break;
        }
    }

exit:
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
    }
    if (id_blob != NULL)
    {
        free(id_blob);
    }
    if (company_id_blob != NULL)
    {
        free(company_id_blob);
    }
}

static void updatePOC(const struct ScribaPoc *poc)
{
    char query[] = "UPDATE People SET firstname=?,secondname=?,lastname=?,mobilenum=?,"
                   "phonenum=?,email=?,position=?,company_id=? WHERE id=?";
    sqlite3_stmt *stmt = NULL;
    void *id_blob = NULL;
    void *company_id_blob = NULL;

    if ((data == NULL) || (poc == NULL))
    {
        goto exit;
    }

    // prepare query
    if (sqlite3_prepare_v2(data->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 1, poc->firstname, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 2, poc->secondname, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 3, poc->lastname, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 4, poc->mobilenum, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 5, poc->phonenum, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 6, poc->email, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 7, poc->position, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    company_id_blob = scriba_id_to_blob(&(poc->company_id));
    if (sqlite3_bind_blob(stmt,
                          8,
                          company_id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    id_blob = scriba_id_to_blob(&(poc->id));
    if (sqlite3_bind_blob(stmt,
                          9,
                          id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }

    // execute query
    while (1)
    {
        int ret = sqlite3_step(stmt);
        if (ret == SQLITE_BUSY)
        {
            // retry
            continue;
        }
        else
        {
            break;
        }
    }

exit:
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
    }
    if (id_blob != NULL)
    {
        free(id_blob);
    }
    if (company_id_blob != NULL)
    {
        free(company_id_blob);
    }
}

static void removePOC(scriba_id_t id)
{
    char query[] = "DELETE FROM People WHERE id=?";
    sqlite3_stmt *stmt = NULL;
    void *id_blob = NULL;

    if (data == NULL)
    {
        goto exit;
    }

    // prepare query
    if (sqlite3_prepare_v2(data->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        goto exit;
    }
    id_blob = scriba_id_to_blob(&id);
    if (sqlite3_bind_blob(stmt,
                          1,
                          id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }

    // execute query
    while (1)
    {
        int ret = sqlite3_step(stmt);
        if (ret == SQLITE_BUSY)
        {
            // retry
            continue;
        }
        else
        {
            break;
        }
    }

exit:
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
    }
    if (id_blob != NULL)
    {
        free(id_blob);
    }
}

// create project data structure based on given parameters
static struct ScribaProject *fillProjectData(scriba_id_t id, const char *title, const char *descr,
                                             scriba_id_t company_id, enum ScribaProjectState state)
{
    int len = 0;
    struct ScribaProject *project = (struct ScribaProject*)malloc(sizeof (struct ScribaProject));
    if (project == NULL)
    {
        return NULL;
    }
    memset(project, 0, sizeof (struct ScribaProject));

    scriba_id_copy(&(project->id), &id);
    if (title != NULL)
    {
        len = strlen(title);
        project->title = (char *)malloc(len + 1);
        strcpy(project->title, title);
    }
    if (descr != NULL)
    {
        len = strlen(descr);
        project->descr = (char *)malloc(len + 1);
        strcpy(project->descr, descr);
    }
    scriba_id_copy(&(project->company_id), &company_id);
    project->state = state;

    return project;
}

// common project search routine
static scriba_list_t *projectSearch(sqlite3_stmt *stmt)
{
    scriba_list_t *projects = scriba_list_init();

    // execute query and retrieve results
    while (1)
    {
        int ret = sqlite3_step(stmt);
        if (ret == SQLITE_BUSY)
        {
            // retry
            continue;
        }
        else if (ret == SQLITE_ROW)
        {
            // we have the data
            // id and title fields are always selected from the table
            scriba_id_t id;
            scriba_id_from_blob(sqlite3_column_blob(stmt, 0), &id);
            const char *title = sqlite3_column_text(stmt, 1);
            int len = strlen(title);
            if (len > 0)
            {
                char *project_title = (char *)malloc(len + 1);
                strcpy(project_title, title);
                scriba_list_add(projects, id, project_title);
                free(project_title);
            }
            else
            {
                scriba_list_add(projects, id, NULL);
            }
        }
        else
        {
            break;
        }
    }

exit:
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
    }
    return projects;
}

// project handling interface functions
static struct ScribaProject *getProject(scriba_id_t id)
{
    char query[] = "SELECT * FROM Projects WHERE id=?";
    sqlite3_stmt *stmt = NULL;
    void *id_blob = NULL;

    if (data == NULL)
    {
        goto error;
    }

    // prepare query
    if (sqlite3_prepare_v2(data->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        goto error;
    }
    id_blob = scriba_id_to_blob(&id);
    if (sqlite3_bind_blob(stmt,
                          1,
                          id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto error;
    }

    // execute query
    while (1)
    {
        int ret = sqlite3_step(stmt);
        if (ret == SQLITE_BUSY)
        {
            // retry
            continue;
        }
        else if (ret == SQLITE_ROW)
        {
            // we have the data
            break;
        }
        else
        {
            goto error;
        }
    }

    // fetch results
    if (sqlite3_column_count(stmt) != PROJECT_TABLE_COLUMNS)
    {
        goto error;
    }
    scriba_id_t project_id;
    scriba_id_from_blob(sqlite3_column_blob(stmt, 0), &project_id);
    const char *title = (const char*)sqlite3_column_text(stmt, 1);
    const char *descr = (const char*)sqlite3_column_text(stmt, 2);
    scriba_id_t company_id;
    scriba_id_from_blob(sqlite3_column_blob(stmt, 3), &company_id);
    enum ScribaProjectState state = (enum ScribaProjectState)sqlite3_column_int(stmt, 4);

    struct ScribaProject *project = fillProjectData(project_id, title, descr, company_id, state);
    sqlite3_finalize(stmt);
    if (id_blob != NULL)
    {
        free(id_blob);
    }
    return project;

error:
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
    }
    if (id_blob != NULL)
    {
        free(id_blob);
    }
    return NULL;
}

static scriba_list_t *getAllProjects()
{
    sqlite3_stmt *stmt = NULL;
    char query[] = "SELECT id,title FROM Projects";

    if (data == NULL)
    {
        goto error;
    }

    if (sqlite3_prepare_v2(data->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        goto error;
    }

    return projectSearch(stmt);

error:
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
    }
    // we have to return an empty list, can't return NULL
    return (scriba_list_init());
}

static scriba_list_t *getProjectsByTitle(const char *title)
{
    scriba_list_t *projects = scriba_list_init();
    char query[] = "SELECT id,title FROM Projects WHERE title LIKE ?";
    sqlite3_stmt *stmt = NULL;
    char *search = str_for_like_op(title);

    if (data == NULL)
    {
        goto exit;
    }

    if (sqlite3_prepare_v2(data->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        goto exit;
    }

    if (sqlite3_bind_text(stmt, 1, search, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }

    while (1)
    {
        int ret = sqlite3_step(stmt);
        if (ret == SQLITE_BUSY)
        {
            // retry
            continue;
        }
        else if (ret == SQLITE_ROW)
        {
            // we have the data
            // id and title fields are always selected from the table
            scriba_id_t id;
            scriba_id_from_blob(sqlite3_column_blob(stmt, 0), &id);
            const char *title = sqlite3_column_text(stmt, 1);
            int len = strlen(title);
            if (len > 0)
            {
                char *project_title = (char *)malloc(len + 1);
                strcpy(project_title, title);
                scriba_list_add(projects, id, project_title);
                free(project_title);
            }
            else
            {
                scriba_list_add(projects, id, NULL);
            }
        }
        else
        {
            break;
        }
    }

exit:
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
    }
    if (search != NULL)
    {
        free(search);
    }
    return projects;
}

static scriba_list_t *getProjectsByCompany(scriba_id_t id)
{
    char query[] = "SELECT id,title FROM Projects WHERE company_id=?";
    sqlite3_stmt *stmt = NULL;
    void *id_blob = NULL;

    if (data == NULL)
    {
        goto error;
    }

    if (sqlite3_prepare_v2(data->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        goto error;
    }
    id_blob = scriba_id_to_blob(&id);
    if (sqlite3_bind_blob(stmt,
                          1,
                          id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto error;
    }

    if (id_blob != NULL)
    {
        free(id_blob);
    }
    return projectSearch(stmt);
 
error:
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
    }
    if (id_blob != NULL)
    {
        free(id_blob);
    }
    // we have to return an empty list, can't return NULL
    return (scriba_list_init());
}

static scriba_list_t *getProjectsByState(enum ScribaProjectState state)
{
    char query[] = "SELECT id,title FROM Projects WHERE state=?";
    sqlite3_stmt *stmt = NULL;

    if (data == NULL)
    {
        goto error;
    }

    if (sqlite3_prepare_v2(data->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        goto error;
    }
    if (sqlite3_bind_int(stmt, 1, (int)state) != SQLITE_OK)
    {
        goto error;
    }

    return projectSearch(stmt);

error:
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
    }
    // we have to return an empty list, can't return NULL
    return (scriba_list_init());
}

static void addProject(scriba_id_t id, const char *title, const char *descr,
                       scriba_id_t company_id, enum ScribaProjectState state)
{
    char query[] = "INSERT INTO Projects(id,title,descr,company_id,state) "
                   "VALUES(?,?,?,?,?)";
    sqlite3_stmt *stmt = NULL;
    void *id_blob = NULL;
    void *company_id_blob = NULL;

    if (data == NULL)
    {
        goto exit;
    }

    id_blob = scriba_id_to_blob(&id);

    // prepare query
    if (sqlite3_prepare_v2(data->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_blob(stmt,
                          1,
                          id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 2, title, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 3, descr, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    company_id_blob = scriba_id_to_blob(&company_id);
    if (sqlite3_bind_blob(stmt,
                          4,
                          company_id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_int(stmt, 5, (int)state) != SQLITE_OK)
    {
        goto exit;
    }

    // execute query
    while (1)
    {
        int ret = sqlite3_step(stmt);
        if (ret == SQLITE_BUSY)
        {
            // retry
            continue;
        }
        else
        {
            break;
        }
    }

exit:
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
    }
    if (id_blob != NULL)
    {
        free(id_blob);
    }
    if (company_id_blob != NULL)
    {
        free(company_id_blob);
    }
}

static void updateProject(const struct ScribaProject *project)
{
    char query[] = "UPDATE Projects SET title=?,descr=?,company_id=?,state=? WHERE id=?";
    sqlite3_stmt *stmt = NULL;
    void *id_blob = NULL;
    void *company_id_blob = NULL;

    if ((data == NULL) || (project == NULL))
    {
        goto exit;
    }

    // prepare query
    if (sqlite3_prepare_v2(data->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 1, project->title, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_text(stmt, 2, project->descr, -1, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    company_id_blob = scriba_id_to_blob(&(project->company_id));
    if (sqlite3_bind_blob(stmt,
                          3,
                          company_id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }
    if (sqlite3_bind_int(stmt, 4, (int)(project->state)) != SQLITE_OK)
    {
        goto exit;
    }
    id_blob = scriba_id_to_blob(&(project->id));
    if (sqlite3_bind_blob(stmt,
                          5,
                          id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }

    // execute query
    while (1)
    {
        int ret = sqlite3_step(stmt);
        if (ret == SQLITE_BUSY)
        {
            // retry
            continue;
        }
        else
        {
            break;
        }
    }

exit:
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
    }
    if (id_blob != NULL)
    {
        free(id_blob);
    }
    if (company_id_blob != NULL)
    {
        free(company_id_blob);
    }
}

static void removeProject(scriba_id_t id)
{
    char query[] = "DELETE FROM Projects WHERE id=?";
    sqlite3_stmt *stmt = NULL;
    void *id_blob = NULL;

    if (data == NULL)
    {
        goto exit;
    }

    // prepare query
    if (sqlite3_prepare_v2(data->db, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        goto exit;
    }
    id_blob = scriba_id_to_blob(&id);
    if (sqlite3_bind_blob(stmt,
                          1,
                          id_blob,
                          SCRIBA_ID_BLOB_SIZE,
                          SQLITE_TRANSIENT) != SQLITE_OK)
    {
        goto exit;
    }

    // execute query
    while (1)
    {
        int ret = sqlite3_step(stmt);
        if (ret == SQLITE_BUSY)
        {
            // retry
            continue;
        }
        else
        {
            break;
        }
    }

exit:
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
    }
    if (id_blob != NULL)
    {
        free(id_blob);
    }
}
