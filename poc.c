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

// get all POCs with given name
scriba_list_t *scriba_getPOCByName(const char *firstname,
                                   const char *secondname,
                                   const char *lastname)
{
    return (fTbl->getPOCByName(firstname, secondname, lastname));
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
    fTbl->addPOC(firstname, secondname, lastname,
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

    ret->id = poc->id;
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
    ret->company_id = poc->company_id;

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
