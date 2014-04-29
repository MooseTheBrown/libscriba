#ifndef SCRIBA_EVENT_H
#define SCRIBA_EVENT_H

#include "types.h"

// event types
enum ScribaEventType
{
    EVENT_TYPE_MEETING = 0,
    EVENT_TYPE_CALL = 1,
    EVENT_TYPE_TASK = 2
};

// event states
enum ScribaEventState
{
    EVENT_STATE_SCHEDULED = 0,
    EVENT_STATE_COMPLETED = 1,
    EVENT_STATE_CANCELLED = 2
};

// event data structure
struct ScribaEvent
{
    scriba_id_t id;                     // unique id assigned by the library
    char *descr;                        // event description
    scriba_id_t company_id;             // company involved
    scriba_id_t poc_id;                 // poc involved
    scriba_id_t project_id;             // related project
    enum ScribaEventType type;          // type of the event
    char *outcome;                      // description of event outcome
    scriba_time_t timestamp;            // event timestamp
    enum ScribaEventState state;        // state of the event
};

// get event info by id
struct ScribaEvent *scriba_getEvent(scriba_id_t id);
// get all events
scriba_list_t *scriba_getAllEvents();
// get all events associated with given company
scriba_list_t *scriba_getEventsByCompany(scriba_id_t id);
// get all events associated with given person
scriba_list_t *scriba_getEventsByPOC(scriba_id_t id);
// get all events associated with given project
scriba_list_t *scriba_getEventsByProject(scriba_id_t id);
// add new event to the database
void scriba_addEvent(const char *descr, scriba_id_t company_id, scriba_id_t poc_id,
                     scriba_id_t project_id, enum ScribaEventType type, const char *outcome,
                     scriba_time_t timestamp, enum ScribaEventState state);
// update event info
void scriba_updateEvent(const struct ScribaEvent *event);
// delete event info from the database
void scriba_removeEvent(scriba_id_t id);
// create a copy of event data structure
struct ScribaEvent *scriba_copyEvent(const struct ScribaEvent *event);
// free memory occupied by event data structure
void scriba_freeEventData(struct ScribaEvent *event);

#endif // SCRIBA_EVENT_H
