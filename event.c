#include "event.h"
#include "db_backend.h"
#include <stdlib.h>
#include <string.h>

extern struct ScribaDBFuncTbl *fTbl;

// get event info by id
struct ScribaEvent *scriba_getEvent(scriba_id_t id)
{
    return (fTbl->getEvent(id));
}

// get all events
scriba_list_t *scriba_getAllEvents()
{
    return (fTbl->getAllEvents());
}

// get all events associated with given company
scriba_list_t *scriba_getEventsByCompany(scriba_id_t id)
{
    return (fTbl->getEventsByCompany(id));
}

// get all events associated with given person
scriba_list_t *scriba_getEventsByPOC(scriba_id_t id)
{
    return (fTbl->getEventsByPOC(id));
}

// get all events associated with given project
scriba_list_t *scriba_getEventsByProject(scriba_id_t id)
{
    return (fTbl->getEventsByProject(id));
}

// add new event to the database
void scriba_addEvent(const char *descr, scriba_id_t company_id, scriba_id_t poc_id,
                     scriba_id_t project_id, enum ScribaEventType type, const char *outcome,
                     scriba_time_t timestamp)
{
    fTbl->addEvent(descr, company_id, poc_id, project_id, type, outcome, timestamp);
}

// update event info
void scriba_updateEvent(const struct ScribaEvent *event)
{
    fTbl->updateEvent(event);
}

// delete event info from the database
void scriba_removeEvent(scriba_id_t id)
{
    fTbl->removeEvent(id);
}

// create a copy of event data structure
struct ScribaEvent *scriba_copyEvent(const struct ScribaEvent *event)
{
    int len = 0;

    if (event == NULL)
    {
        return NULL;
    }

    struct ScribaEvent *ret = (struct ScribaEvent *)malloc(sizeof (struct ScribaEvent));
    memset(ret, 0, sizeof (struct ScribaEvent));

    ret->id = event->id;
    if ((len = strlen(event->descr)) != 0)
    {
        ret->descr = (char *)malloc(len + 1);
        memset(ret->descr, 0, len + 1);
        strncpy(ret->descr, event->descr, len);
    }
    ret->company_id = event->company_id;
    ret->poc_id = event->poc_id;
    ret->project_id = event->project_id;
    ret->type = event->type;
    if ((len = strlen(event->outcome)) != 0)
    {
        ret->outcome = (char *)malloc(len + 1);
        memset(ret->outcome, 0, len + 1);
        strncpy(ret->outcome, event->outcome, len);
    }
    ret->timestamp = event->timestamp;

    return ret;
}

// free memory occupied by event data structure
void scriba_freeEventData(struct ScribaEvent *event)
{
    if (event == NULL)
    {
        return;
    }

    if (event->descr != NULL)
    {
        free(event->descr);
    }
    if (event->outcome != NULL)
    {
        free(event->outcome);
    }

    free(event);
}
