#include "sqlite_backend.h"
#include <stdlib.h>
#include <string.h>

// maximum number of internal database backends
#define MAX_INT_BACKENDS    10


/* To add new built-in backend add pointer to its ScribaInternalDB
   descriptor to the following array. If the array is already full,
   add new element, but don't forget to update MAX_INT_BACKENDS 
   value. */
static struct ScribaInternalDB *int_backends[MAX_INT_BACKENDS] = 
{
    &sqliteDB,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static struct ScribaInternalDB *cur_backend = NULL;

struct ScribaDBFuncTbl *fTbl = NULL;



// add new internal database backend at run-time
void scriba_addInternalDB(struct ScribaInternalDB *db)
{
    if (db == NULL)
    {
        goto out;
    }

    for (int i = 0; i < MAX_INT_BACKENDS; i++)
    {
        if (int_backends[i] == NULL)
        {
            int_backends[i] = db;
            break;
        }
    }

out:
    return;
}

// initialize libscriba
int scriba_init(struct ScribaDB *db, struct ScribaDBParamList *pl)
{
    int ret = SCRIBA_INIT_SUCCESS;

    if (db == NULL)
    {
        ret = SCRIBA_INIT_INVALID_ARG;
        goto out;
    }
    if (db->name == NULL)
    {
        ret = SCRIBA_INIT_INVALID_ARG;
        goto out;
    }
    if (db->type == SCRIBA_DB_EXT)
    {
        // currently not supported
        ret = SCRIBA_INIT_NOT_SUPPORTED;
        goto out;
    }

    // find backend with given name
    for (int i = 0; i < MAX_INT_BACKENDS; i++)
    {
        if (strcmp(int_backends[i]->name, db->name) == 0)
        {
            cur_backend = int_backends[i];
            break;
        }
    }

    if (cur_backend != NULL)
    {
        fTbl = (struct ScribaDBFuncTbl *)malloc(sizeof (struct ScribaDBFuncTbl));
        memset(fTbl, 0, sizeof (struct ScribaDBFuncTbl));

        // call backend init function
        if (cur_backend->init(pl, fTbl) != 0)
        {
            ret = SCRIBA_INIT_BACKEND_INIT_FAILED;
            goto out;
        }
    }
    else
    {
        ret = SCRIBA_INIT_BACKEND_NOT_FOUND;
        goto out;
    }

out:
    return ret;
}

// library clean up
void scriba_cleanup()
{
    // call backend clean up function
    if (cur_backend != NULL)
    {
        cur_backend->cleanup();
        cur_backend = NULL;
    }

    if (fTbl != NULL)
    {
        free(fTbl);
        fTbl = NULL;
    }
}
