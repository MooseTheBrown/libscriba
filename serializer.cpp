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

#include "serializer.h"
#include "scriba_generated.h"
#include "company.h"
#include "event.h"
#include "poc.h"
#include "project.h"
#include <cstdlib>

namespace scriba
{

namespace fb = flatbuffers;

// internal serializer functions use C++ linkage
static fb::Offset<Company> serialize_company(scriba_id_t id, fb::FlatBufferBuilder &fbb);
static fb::Offset<Event> serialize_event(scriba_id_t id, fb::FlatBufferBuilder &fbb);
static fb::Offset<POC> serialize_poc(scriba_id_t id, fb::FlatBufferBuilder &fbb);
static fb::Offset<Project> serialize_project(scriba_id_t id, fb::FlatBufferBuilder &fbb);

// externally visible functions should use C linkage
#ifdef __cplusplus
extern "C"
{
#endif

// serialize the given entries into binary buffer and return the buffer pointer
// buflen will contain buffer size
void *serialize(scriba_list_t *companies,
                scriba_list_t *events,
                scriba_list_t *people,
                scriba_list_t *projects,
                unsigned long *buflen)
{
    fb::FlatBufferBuilder fbb;
    std::vector<flatbuffers::Offset<Company>> comp_offsets;
    std::vector<flatbuffers::Offset<Event>> event_offsets;
    std::vector<flatbuffers::Offset<POC>> poc_offsets;
    std::vector<flatbuffers::Offset<Project>> project_offsets;

    // serialize each entry and create offset vectors
    scriba_list_for_each(companies, company)
    {
        auto offset = serialize_company(company->id, fbb);
        comp_offsets.push_back(offset);
    }
    auto comp_vector = fbb.CreateVector(comp_offsets);

    scriba_list_for_each(events, event)
    {
        auto offset = serialize_event(event->id, fbb);
        event_offsets.push_back(offset);
    }
    auto event_vector = fbb.CreateVector(event_offsets);

    scriba_list_for_each(people, poc)
    {
        auto offset = serialize_poc(poc->id, fbb);
        poc_offsets.push_back(offset);
    }
    auto poc_vector = fbb.CreateVector(poc_offsets);

    scriba_list_for_each(projects, project)
    {
        auto offset = serialize_project(project->id, fbb);
        project_offsets.push_back(offset);
    }
    auto project_vector = fbb.CreateVector(project_offsets);

    // now we have all the offsets, we can create the root element
    EntriesBuilder eb(fbb);
    eb.add_companies(comp_vector);
    eb.add_events(event_vector);
    eb.add_people(poc_vector);
    eb.add_projects(project_vector);
    auto root_offset = eb.Finish();

    // finalize the buffer
    fbb.Finish(root_offset);

    if (buflen != NULL)
    {
        *buflen = fbb.GetSize();
    }

    return fbb.GetBufferPointer();
}

// read entry data from the given buffer and store it in the local database
// according to the given merge strategy
enum ScribaMergeStatus deserialize(void *buf, unsigned long buflen)
{
}

#ifdef __cplusplus
}
#endif

// internal serializer functions implementation

static fb::Offset<Company> serialize_company(scriba_id_t id, fb::FlatBufferBuilder &fbb)
{
    ScribaCompany *company = scriba_getCompany(id);

    CompanyBuilder cb(fbb);
    cb.add_id(id);
    cb.add_name(fbb.CreateString(company->name));
    cb.add_jur_name(fbb.CreateString(company->jur_name));
    cb.add_address(fbb.CreateString(company->address));
    char *inn = scriba_inn_to_string(&(company->inn));
    cb.add_inn(fbb.CreateString(inn));
    std::free(inn);
    cb.add_phonenum(fbb.CreateString(company->phonenum));
    cb.add_email(fbb.CreateString(company->email));

    scriba_freeCompanyData(company);
    return cb.Finish();
}

static fb::Offset<Event> serialize_event(scriba_id_t id, fb::FlatBufferBuilder &fbb)
{
    ScribaEvent *event = scriba_getEvent(id);

    EventBuilder evb(fbb);
    evb.add_id(id);
    evb.add_descr(fbb.CreateString(event->descr));
    evb.add_company_id(event->company_id);
    evb.add_project_id(event->project_id);
    evb.add_poc_id(event->poc_id);
    switch(event->type)
    {
    case EVENT_TYPE_MEETING:
        evb.add_type(EventType_MEETING);
        break;
    case EVENT_TYPE_CALL:
        evb.add_type(EventType_CALL);
        break;
    case EVENT_TYPE_TASK:
        evb.add_type(EventType_TASK);
        break;
    }
    evb.add_outcome(fbb.CreateString(event->outcome));
    evb.add_timestamp(event->timestamp);
    switch(event->state)
    {
    case EVENT_STATE_SCHEDULED:
        evb.add_state(EventState_SCHEDULED);
        break;
    case EVENT_STATE_COMPLETED:
        evb.add_state(EventState_COMPLETED);
        break;
    case EVENT_STATE_CANCELLED:
        evb.add_state(EventState_CANCELED);
        break;
    }

    scriba_freeEventData(event);
    return evb.Finish();
}

static fb::Offset<POC> serialize_poc(scriba_id_t id, fb::FlatBufferBuilder &fbb)
{
    ScribaPoc *poc = scriba_getPOC(id);

    POCBuilder pb(fbb);
    pb.add_id(id);
    pb.add_firstname(fbb.CreateString(poc->firstname));
    pb.add_secondname(fbb.CreateString(poc->secondname));
    pb.add_lastname(fbb.CreateString(poc->lastname));
    pb.add_mobilenum(fbb.CreateString(poc->mobilenum));
    pb.add_phonenum(fbb.CreateString(poc->phonenum));
    pb.add_email(fbb.CreateString(poc->email));
    pb.add_position(fbb.CreateString(poc->position));
    pb.add_company_id(poc->company_id);

    scriba_freePOCData(poc);
    return pb.Finish();
}

static fb::Offset<Project> serialize_project(scriba_id_t id, fb::FlatBufferBuilder &fbb)
{
    ScribaProject *project = scriba_getProject(id);

    ProjectBuilder prjb(fbb);
    prjb.add_id(id);
    prjb.add_title(fbb.CreateString(project->title));
    prjb.add_descr(fbb.CreateString(project->descr));
    prjb.add_company_id(project->company_id);
    switch(project->state)
    {
    case PROJECT_STATE_INITIAL:
        prjb.add_state(ProjectState_INITIAL);
        break;
    case PROJECT_STATE_CLIENT_INFORMED:
        prjb.add_state(ProjectState_CLIENT_INFORMED);
        break;
    case PROJECT_STATE_CLIENT_RESPONSE:
        prjb.add_state(ProjectState_CLIENT_RESPONSE);
        break;
    case PROJECT_STATE_OFFER:
        prjb.add_state(ProjectState_OFFER);
        break;
    case PROJECT_STATE_REJECTED:
        prjb.add_state(ProjectState_REJECTED);
        break;
    case PROJECT_STATE_CONTRACT_SIGNED:
        prjb.add_state(ProjectState_CONTRACT_SIGNED);
        break;
    case PROJECT_STATE_EXECUTION:
        prjb.add_state(ProjectState_EXECUTION);
        break;
    case PROJECT_STATE_PAYMENT:
        prjb.add_state(ProjectState_PAYMENT);
        break;
    }

    scriba_freeProjectData(project);
    return prjb.Finish();
}

} // namespace scriba