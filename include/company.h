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

#ifndef SCRIBA_COMPANY_H
#define SCRIBA_COMPANY_H

#include "types.h"

#ifdef __cplusplus
extern "C"
{
#endif

// company data structure
struct ScribaCompany
{
    scriba_id_t id;                 // unique id assigned by the library
    char *name;                     // company name
    char *jur_name;                 // juridicial name of the company
    char *address;                  // company address
    scriba_inn_t inn;               // tax payer id (INN) of the company
    char *phonenum;                 // official company phone number
    char *email;                    // official company email
    scriba_list_t *poc_list;        // list of people associated with the company
    scriba_list_t *proj_list;       // list of projects associated with the company
    scriba_list_t *event_list;      // list of event associated with the company
};

// get company info by company id
struct ScribaCompany *scriba_getCompany(scriba_id_t id);
// get all companies stored in the database
scriba_list_t *scriba_getAllCompanies();
// search companies by name
scriba_list_t *scriba_getCompaniesByName(const char *name);
// search companies by juridicial name
scriba_list_t *scriba_getCompaniesByJurName(const char *juridicial_name);
// search companies by address
scriba_list_t *scriba_getCompaniesByAddress(const char *address);
// add new company to the database
void scriba_addCompany(const char *name, const char *jur_name, const char *address,
                       scriba_inn_t inn, const char *phonenum, const char *email);
// add company with given id to the database
void scriba_addCompanyWithID(scriba_id_t id, const char *name, const char *jur_name,
                             const char *address, scriba_inn_t inn, const char *phonenum,
                             const char *email);
// update company info
void scriba_updateCompany(const struct ScribaCompany *company);
// remove company info from the database
void scriba_removeCompany(scriba_id_t id);
// create a copy of company data structure
struct ScribaCompany *scriba_copyCompany(const struct ScribaCompany *company);
// free memory occupied by company data structure
void scriba_freeCompanyData(struct ScribaCompany *company);

#ifdef __cplusplus
}
#endif

#endif // SCRIBA_COMPANY_H
