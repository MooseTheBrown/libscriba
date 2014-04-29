#ifndef SCRIBA_POC_H
#define SCRIBA_POC_H

#include "types.h"

// Person Of Contact data structure
struct ScribaPoc
{
    scriba_id_t id;             // unique id assigned by the library
    char *firstname;            // first name
    char *secondname;           // second name
    char *lastname;             // last name
    char *mobilenum;            // mobile phone number
    char *phonenum;             // phone number
    char *email;                // email
    char *position;             // position in a company
    scriba_id_t company_id;     // company of employment
};

// get POC by id
struct ScribaPoc *scriba_getPOC(scriba_id_t id);
// get all people with given name
scriba_list_t *scriba_getPOCByName(const char *firstname,
                                   const char *secondname,
                                   const char *lastname);
// get all people
scriba_list_t *scriba_getAllPeople();
// get all people working in given company
scriba_list_t *scriba_getPOCByCompany(scriba_id_t id);
// get all people with given position
scriba_list_t *scriba_getPOCByPosition(const char *position);
// get all people with given phone number
scriba_list_t *scriba_getPOCByPhoneNum(const char *phonenum);
// get all people with given email
scriba_list_t *scriba_getPOCByEmail(const char *email);
// add person to the database
void scriba_addPOC(const char *firstname, const char *secondname, const char *lastname,
            const char *mobilenum, const char *phonenum, const char *email,
            const char *position, scriba_id_t company_id);
// update POC info
void scriba_updatePOC(const struct ScribaPoc *poc);
// remove POC from the database
void scriba_removePOC(scriba_id_t id);
// create a copy of POC data structure
struct ScribaPoc *scriba_copyPOC(const struct ScribaPoc *poc);
// free memory occupied by POC data structure
void scriba_freePOCData(struct ScribaPoc *poc);

#endif // SCRIBA_POC_H
