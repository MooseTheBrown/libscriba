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

#include "poc.h"
#include "db_backend.h"
#include <stdlib.h>
#include <string.h>

extern struct ScribaDBFuncTbl *fTbl;

// get POC by id
struct ScribaPoc *scriba_getPOC(scriba_id_t id)
{
    return (fTbl->getPOC(id));
}

// search people by name
scriba_list_t *scriba_getPOCByName(const char *name)
{
    return (fTbl->getPOCByName(name));
}

// get all people
scriba_list_t *scriba_getAllPeople()
{
    return (fTbl->getAllPeople());
}

// get all people working in given company
scriba_list_t *scriba_getPOCByCompany(scriba_id_t id)
{
    return (fTbl->getPOCByCompany(id));
}

// get all people with given position
scriba_list_t *scriba_getPOCByPosition(const char *position)
{
    return (fTbl->getPOCByPosition(position));
}

// get all people with given phone number
scriba_list_t *scriba_getPOCByPhoneNum(const char *phonenum)
{
    return (fTbl->getPOCByPhoneNum(phonenum));
}

// get all people with given email
scriba_list_t *scriba_getPOCByEmail(const char *email)
{
    return (fTbl->getPOCByEmail(email));
}

// add person to the database
void scriba_addPOC(const char *firstname, const char *secondname, const char *lastname,
            const char *mobilenum, const char *phonenum, const char *email,
            const char *position, scriba_id_t company_id)
{
    scriba_id_t id;

    scriba_id_create(&id);
    fTbl->addPOC(id, firstname, secondname, lastname,
                 mobilenum, phonenum, email,
                 position, company_id);
}

// add person with given ID to the database
void scriba_addPOCWithID(scriba_id_t id, const char *firstname, const char *secondname,
                         const char *lastname, const char *mobilenum, const char *phonenum,
                         const char *email, const char *position, scriba_id_t company_id)
{
    fTbl->addPOC(id, firstname, secondname, lastname,
                 mobilenum, phonenum, email,
                 position, company_id);
}

// update POC info
void scriba_updatePOC(const struct ScribaPoc *poc)
{
    fTbl->updatePOC(poc);
}

// remove POC from the database
void scriba_removePOC(scriba_id_t id)
{
    fTbl->removePOC(id);
}

// create a copy of POC data structure
struct ScribaPoc *scriba_copyPOC(const struct ScribaPoc *poc)
{
    int len = 0;

    if (poc == NULL)
    {
        return NULL;
    }

    struct ScribaPoc *ret = (struct ScribaPoc *)malloc(sizeof (struct ScribaPoc));
    memset(ret, 0, sizeof(struct ScribaPoc));

    scriba_id_copy(&(ret->id), &(poc->id));
    if ((len = strlen(poc->firstname)) != 0)
    {
        ret->firstname = (char *)malloc(len + 1);
        memset(ret->firstname, 0, len + 1);
        strncpy(ret->firstname, poc->firstname, len);
    }
    if ((len = strlen(poc->secondname)) != 0)
    {
        ret->secondname = (char *)malloc(len + 1);
        memset(ret->secondname, 0, len + 1);
        strncpy(ret->secondname, poc->secondname, len);
    }
    if ((len = strlen(poc->lastname)) != 0)
    {
        ret->lastname = (char *)malloc(len + 1);
        memset(ret->lastname, 0, len + 1);
        strncpy(ret->lastname, poc->lastname, len);
    }
    if ((len = strlen(poc->mobilenum)) != 0)
    {
        ret->mobilenum = (char *)malloc(len + 1);
        memset(ret->mobilenum, 0, len + 1);
        strncpy(ret->mobilenum, poc->mobilenum, len);
    }
    if ((len = strlen(poc->phonenum)) != 0)
    {
        ret->phonenum = (char *)malloc(len + 1);
        memset(ret->phonenum, 0, len + 1);
        strncpy(ret->phonenum, poc->phonenum, len);
    }
    if ((len = strlen(poc->email)) != 0)
    {
        ret->email = (char *)malloc(len + 1);
        memset(ret->email, 0, len + 1);
        strncpy(ret->email, poc->email, len);
    }
    if ((len = strlen(poc->position)) != 0)
    {
        ret->position = (char *)malloc(len + 1);
        memset(ret->position, 0, len + 1);
        strncpy(ret->position, poc->position, len);
    }
    scriba_id_copy(&(ret->company_id), &(poc->company_id));

    return ret;
}

// free memory occupied by POC data structure
void scriba_freePOCData(struct ScribaPoc *poc)
{
    if (poc == NULL)
    {
        return;
    }

    if (poc->firstname != NULL)
    {
        free(poc->firstname);
    }
    if (poc->secondname != NULL)
    {
        free(poc->secondname);
    }
    if (poc->lastname != NULL)
    {
        free(poc->lastname);
    }
    if (poc->mobilenum != NULL)
    {
        free(poc->mobilenum);
    }
    if (poc->phonenum != NULL)
    {
        free(poc->phonenum);
    }
    if (poc->email != NULL)
    {
        free(poc->email);
    }
    if (poc->position != NULL)
    {
        free(poc->position);
    }

    free(poc);
}
