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

#include "company.h"
#include "db_backend.h"
#include <stdlib.h>
#include <string.h>

extern struct ScribaDBFuncTbl *fTbl;

// get company info by company id
struct ScribaCompany *scriba_getCompany(scriba_id_t id)
{
    return (fTbl->getCompany(id));
}

// get all companies stored in the database
scriba_list_t *scriba_getAllCompanies()
{
    return (fTbl->getAllCompanies());
}

// get companies with given name
scriba_list_t *scriba_getCompaniesByName(const char *name)
{
    return (fTbl->getCompaniesByName(name));
}

// get companies with given juridicial name
scriba_list_t *scriba_getCompaniesByJurName(const char *juridicial_name)
{
    return (fTbl->getCompaniesByJurName(juridicial_name));
}

// get companies with given address
scriba_list_t *scriba_getCompaniesByAddress(const char *address)
{
    return (fTbl->getCompaniesByAddress(address));
}

// add new company to the database
void scriba_addCompany(const char *name, const char *jur_name, const char *address,
                       scriba_inn_t inn, const char *phonenum, const char *email)
{
    scriba_id_t company_id;

    scriba_id_create(&company_id);
    fTbl->addCompany(company_id, name, jur_name, address, inn, phonenum, email);
}

// add company with given id to the database
void scriba_addCompanyWithID(scriba_id_t id, const char *name, const char *jur_name,
                             const char *address, scriba_inn_t inn, const char *phonenum,
                             const char *email)
{
    fTbl->addCompany(id, name, jur_name, address, inn, phonenum, email);
}

// update company info
void scriba_updateCompany(const struct ScribaCompany *company)
{
    fTbl->updateCompany(company);
}

// remove company info from the database
void scriba_removeCompany(scriba_id_t id)
{
    fTbl->removeCompany(id);
}

// create a copy of company data structure
struct ScribaCompany *scriba_copyCompany(const struct ScribaCompany *company)
{
    int len = 0;

    if (company == NULL)
    {
        return NULL;
    }

    struct ScribaCompany *ret = (struct ScribaCompany *)malloc(sizeof (struct ScribaCompany));
    memset((void *)ret, 0, sizeof (struct ScribaCompany));
    scriba_id_copy(&(ret->id), &(company->id));
    if ((len = strlen(company->name)) != 0)
    {
        ret->name = (char *)malloc(len + 1);
        memset(ret->name, 0, len + 1);
        strncpy(ret->name, company->name, len);
    }
    if ((len = strlen(company->jur_name)) != 0)
    {
        ret->jur_name = (char *)malloc(len + 1);
        memset(ret->jur_name, 0, len + 1);
        strncpy(ret->jur_name, company->jur_name, len);
    } 
    if ((len = strlen(company->address)) != 0)
    {
        ret->address = (char *)malloc(len + 1);
        memset(ret->address, 0, len + 1);
        strncpy(ret->address, company->address, len);
    } 
    scriba_copy_inn(&(ret->inn), &(company->inn));
    if ((len = strlen(company->phonenum)) != 0)
    {
        ret->phonenum = (char *)malloc(len + 1);
        memset(ret->phonenum, 0, len + 1);
        strncpy(ret->phonenum, company->phonenum, len);
    } 
    if ((len = strlen(company->email)) != 0)
    {
        ret->email = (char *)malloc(len + 1);
        memset(ret->email, 0, len + 1);
        strncpy(ret->email, company->email, len);
    } 

    return ret;
}

// free memory occupied by company data structure
void scriba_freeCompanyData(struct ScribaCompany *company)
{
    if (company == NULL)
    {
        return;
    }

    if (company->name != NULL)
    {
        free(company->name);
    }
    if (company->jur_name != NULL)
    {
        free(company->jur_name);
    }
    if (company->address != NULL)
    {
        free(company->address);
    }
    if (company->phonenum != NULL)
    {
        free(company->phonenum);
    }
    if (company->email != NULL)
    {
        free(company->email);
    }

    scriba_list_delete(company->poc_list);
    scriba_list_delete(company->proj_list);
    scriba_list_delete(company->event_list);

    free(company);
}
