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
static bool deserialize_company(const Company *company, enum ScribaMergeStrategy strategy);
static bool deserialize_event(const Event *event, enum ScribaMergeStrategy strategy);
static bool deserialize_poc(const POC *poc, enum ScribaMergeStrategy strategy);
static bool deserialize_project(const Project *project, enum ScribaMergeStrategy strategy);

// externally visible functions should use C linkage
#ifdef __cplusplus
extern "C"
{
#endif

// serialize the given entries into binary buffer and return the buffer pointer
// buflen will contain buffer size
void *scriba_serialize(scriba_list_t *companies,
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

    // copy it (the original buffer will die with FlatBufferBuilder object)
    void *buf = malloc((size_t)(fbb.GetSize()));
    memcpy(buf, (const void *)fbb.GetBufferPointer(), fbb.GetSize());

    if (buflen != NULL)
    {
        *buflen = fbb.GetSize();
    }
    return buf;
}

// read entry data from the given buffer and store it in the local database
// according to the given merge strategy
enum ScribaMergeStatus scriba_deserialize(void *buf, unsigned long buflen,
                                          enum ScribaMergeStrategy strategy)
{
    const Entries *entries = GetEntries(buf);
    bool conflicts = false;

    const fb::Vector<fb::Offset<Company>> *companies = entries->companies();
    if (companies != nullptr)
    {
        for (fb::uoffset_t i = 0; i < companies->Length(); i++)
        {
            bool ret = deserialize_company(companies->Get(i), strategy);
            if (ret == true)
            {
                conflicts = true;
            }
        }
    }

    const fb::Vector<fb::Offset<Event>> *events = entries->events();
    if (events != nullptr)
    {
        for (fb::uoffset_t i = 0; i < events->Length(); i++)
        {
            bool ret = deserialize_event(events->Get(i), strategy);
            if (ret == true)
            {
                conflicts = true;
            }
        }
    }

    const fb::Vector<fb::Offset<POC>> *people = entries->people();
    if (people != nullptr)
    {
        for (fb::uoffset_t i = 0; i < people->Length(); i++)
        {
            bool ret = deserialize_poc(people->Get(i), strategy);
            if (ret == true)
            {
                conflicts = true;
            }
        }
    }

    const fb::Vector<fb::Offset<Project>> *projects = entries->projects();
    if (projects != nullptr)
    {
        for (fb::uoffset_t i = 0; i < projects->Length(); i++)
        {
            bool ret = deserialize_project(projects->Get(i), strategy);
            if (ret == true)
            {
                conflicts = true;
            }
        }
    }

    return (conflicts == true ? SCRIBA_MERGE_CONFLICTS : SCRIBA_MERGE_OK);
}

#ifdef __cplusplus
}
#endif

// internal serializer functions implementation

static fb::Offset<Company> serialize_company(scriba_id_t id, fb::FlatBufferBuilder &fbb)
{
    ScribaCompany *company = scriba_getCompany(id);
    auto company_name = fbb.CreateString(company->name);
    auto company_jur_name = fbb.CreateString(company->jur_name);
    auto company_address = fbb.CreateString(company->address);
    char *inn = scriba_inn_to_string(&(company->inn));
    auto company_inn = fbb.CreateString(inn);
    auto company_phonenum = fbb.CreateString(company->phonenum);
    auto company_email = fbb.CreateString(company->email);
    ID bufID(id._high, id._low);

    CompanyBuilder cb(fbb);
    cb.add_id(&bufID);
    cb.add_name(company_name);
    cb.add_jur_name(company_jur_name);
    cb.add_address(company_address);
    cb.add_inn(company_inn);
    cb.add_phonenum(company_phonenum);
    cb.add_email(company_email);

    scriba_freeCompanyData(company);
    std::free(inn);
    return cb.Finish();
}

static fb::Offset<Event> serialize_event(scriba_id_t id, fb::FlatBufferBuilder &fbb)
{
    ScribaEvent *event = scriba_getEvent(id);
    ID bufID(id._high, id._low);
    ID companyID(event->company_id._high, event->company_id._low);
    ID projectID(event->project_id._high, event->project_id._low);
    ID pocID(event->poc_id._high, event->poc_id._low);
    auto event_descr = fbb.CreateString(event->descr);
    auto event_outcome = fbb.CreateString(event->outcome);

    EventBuilder evb(fbb);
    evb.add_id(&bufID);
    evb.add_descr(event_descr);
    evb.add_company_id(&companyID);
    evb.add_project_id(&projectID);
    evb.add_poc_id(&pocID);
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
    evb.add_outcome(event_outcome);
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
    ID bufID(id._high, id._low);
    ID companyID(poc->company_id._high, poc->company_id._low);
    auto poc_firstname = fbb.CreateString(poc->firstname);
    auto poc_secondname = fbb.CreateString(poc->secondname);
    auto poc_lastname = fbb.CreateString(poc->lastname);
    auto poc_mobilenum = fbb.CreateString(poc->mobilenum);
    auto poc_phonenum = fbb.CreateString(poc->phonenum);
    auto poc_email = fbb.CreateString(poc->email);
    auto poc_position = fbb.CreateString(poc->position);

    POCBuilder pb(fbb);
    pb.add_id(&bufID);
    pb.add_firstname(poc_firstname);
    pb.add_secondname(poc_secondname);
    pb.add_lastname(poc_lastname);
    pb.add_mobilenum(poc_mobilenum);
    pb.add_phonenum(poc_phonenum);
    pb.add_email(poc_email);
    pb.add_position(poc_position);
    pb.add_company_id(&companyID);

    scriba_freePOCData(poc);
    return pb.Finish();
}

static fb::Offset<Project> serialize_project(scriba_id_t id, fb::FlatBufferBuilder &fbb)
{
    ScribaProject *project = scriba_getProject(id);
    ID bufID(id._high, id._low);
    ID companyID(project->company_id._high, project->company_id._low);
    auto project_title = fbb.CreateString(project->title);
    auto project_descr = fbb.CreateString(project->descr);

    ProjectBuilder prjb(fbb);
    prjb.add_id(&bufID);
    prjb.add_title(project_title);
    prjb.add_descr(project_descr);
    prjb.add_company_id(&companyID);
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

static bool deserialize_company(const Company *company, enum ScribaMergeStrategy strategy)
{
    scriba_id_t company_id;

    company_id._high = (unsigned long long)(company->id()->high());
    company_id._low = (unsigned long long)(company->id()->low());

    ScribaCompany *duplicate = scriba_getCompany(company_id);
    if (duplicate != NULL)
    {
        if (strategy == SCRIBA_MERGE_REMOTE_OVERRIDE)
        {
            ScribaCompany updated_company;
            scriba_id_copy(&(updated_company.id), &company_id);
            updated_company.name = (char *)(company->name()->c_str());
            updated_company.jur_name = (char *)(company->jur_name()->c_str());
            updated_company.address = (char *)(company->address()->c_str());
            updated_company.inn = scriba_inn_from_string(company->inn()->c_str());
            updated_company.phonenum = (char *)(company->phonenum()->c_str());
            updated_company.email = (char *)(company->email()->c_str());
            scriba_updateCompany(&updated_company);
        }

        // If merge stragegy is LOCAL_OVERRIDE, we just keep local data.
        // Manual merge is not supported yet.

        scriba_freeCompanyData(duplicate);
        duplicate = NULL;
    }
    else
    {
        // no such company in the local database, add new one
        scriba_addCompanyWithID(company_id,
                                company->name()->c_str(),
                                company->jur_name()->c_str(),
                                company->address()->c_str(),
                                scriba_inn_from_string(company->inn()->c_str()),
                                company->phonenum()->c_str(),
                                company->email()->c_str());
    }

    return false;
}

static bool deserialize_event(const Event *event, enum ScribaMergeStrategy strategy)
{
    scriba_id_t event_id;
    scriba_id_t company_id;
    scriba_id_t poc_id;
    scriba_id_t project_id;
    enum ScribaEventType event_type = EVENT_TYPE_MEETING;
    enum ScribaEventState event_state = EVENT_STATE_SCHEDULED;

    event_id._high = (unsigned long long)(event->id()->high());
    event_id._low = (unsigned long long)(event->id()->low());
    company_id._high = (unsigned long long)(event->company_id()->high());
    company_id._low = (unsigned long long)(event->company_id()->low());
    poc_id._high = (unsigned long long)(event->poc_id()->high());
    poc_id._low = (unsigned long long)(event->poc_id()->low());
    project_id._high = (unsigned long long)(event->project_id()->high());
    project_id._low = (unsigned long long)(event->project_id()->low());
    switch (event->state())
    {
    case EventState_COMPLETED:
        event_state = EVENT_STATE_COMPLETED;
        break;
    case EventState_CANCELED:
        event_state = EVENT_STATE_CANCELLED;
        break;
    default:
        break;
    }
    switch (event->type())
    {
    case EventType_CALL:
        event_type = EVENT_TYPE_CALL;
        break;
    case EventType_TASK:
        event_type = EVENT_TYPE_TASK;
        break;
    default:
        break;
    }

    ScribaEvent *duplicate = scriba_getEvent(event_id);
    if (duplicate != NULL)
    {
        if (strategy == SCRIBA_MERGE_REMOTE_OVERRIDE)
        {
            ScribaEvent updatedEvent;

            scriba_id_copy(&(updatedEvent.id), &event_id);
            updatedEvent.descr = (char *)(event->descr()->c_str());
            scriba_id_copy(&(updatedEvent.company_id), &company_id);
            scriba_id_copy(&(updatedEvent.poc_id), &poc_id);
            scriba_id_copy(&(updatedEvent.project_id), &project_id);
            updatedEvent.type = event_type;
            updatedEvent.outcome = (char *)(event->outcome()->c_str());
            updatedEvent.timestamp = (scriba_time_t)(event->timestamp());
            updatedEvent.state = event_state;
            scriba_updateEvent(&updatedEvent);
        }

        // If merge stragegy is LOCAL_OVERRIDE, we just keep local data.
        // Manual merge is not supported yet.
        scriba_freeEventData(duplicate);
        duplicate = NULL;
    }
    else
    {
        // no such event, add it
        scriba_addEventWithID(event_id,
                              event->descr()->c_str(),
                              company_id,
                              poc_id,
                              project_id,
                              event_type,
                              event->outcome()->c_str(),
                              (scriba_time_t)(event->timestamp()),
                              event_state);
    }

    return false;
}

static bool deserialize_poc(const POC *poc, enum ScribaMergeStrategy strategy)
{
    scriba_id_t poc_id;
    scriba_id_t company_id;

    poc_id._high = (unsigned long long)(poc->id()->high());
    poc_id._low = (unsigned long long)(poc->id()->low());
    company_id._high = (unsigned long long)(poc->company_id()->high());
    company_id._low = (unsigned long long)(poc->company_id()->low());

    // do we have this POC locally?
    ScribaPoc *duplicate = scriba_getPOC(poc_id);
    if (duplicate != NULL)
    {
        if (strategy == SCRIBA_MERGE_REMOTE_OVERRIDE)
        {
            ScribaPoc updated_poc;

            scriba_id_copy(&(updated_poc.id), &poc_id);
            updated_poc.firstname = (char *)(poc->firstname()->c_str());
            updated_poc.secondname = (char *)(poc->secondname()->c_str());
            updated_poc.lastname = (char *)(poc->lastname()->c_str());
            updated_poc.mobilenum = (char *)(poc->mobilenum()->c_str());
            updated_poc.phonenum = (char *)(poc->phonenum()->c_str());
            updated_poc.email = (char *)(poc->email()->c_str());
            updated_poc.position = (char *)(poc->position()->c_str());
            scriba_id_copy(&(updated_poc.company_id), &company_id);
            scriba_updatePOC(&updated_poc);
        }

        // If merge stragegy is LOCAL_OVERRIDE, we just keep local data.
        // Manual merge is not supported yet.
        scriba_freePOCData(duplicate);
        duplicate = NULL;
    }
    else
    {
        // we don't have this POC locally, add it
        scriba_addPOCWithID(poc_id,
                            poc->firstname()->c_str(),
                            poc->secondname()->c_str(),
                            poc->lastname()->c_str(),
                            poc->mobilenum()->c_str(),
                            poc->phonenum()->c_str(),
                            poc->email()->c_str(),
                            poc->position()->c_str(),
                            company_id);
    }

    return false;
}

static bool deserialize_project(const Project *project, enum ScribaMergeStrategy strategy)
{
    scriba_id_t project_id;
    scriba_id_t company_id;
    enum ScribaProjectState project_state = PROJECT_STATE_INITIAL;

    project_id._high = project->id()->high();
    project_id._low = project->id()->low();
    company_id._high = project->company_id()->high();
    company_id._low = project->company_id()->low();
    switch(project->state())
    {
    case ProjectState_CLIENT_INFORMED:
        project_state = PROJECT_STATE_CLIENT_INFORMED;
        break;
    case ProjectState_CLIENT_RESPONSE:
        project_state = PROJECT_STATE_CLIENT_RESPONSE;
        break;
    case ProjectState_OFFER:
        project_state = PROJECT_STATE_OFFER;
        break;
    case ProjectState_REJECTED:
        project_state = PROJECT_STATE_REJECTED;
        break;
    case ProjectState_CONTRACT_SIGNED:
        project_state = PROJECT_STATE_CONTRACT_SIGNED;
        break;
    case ProjectState_EXECUTION:
        project_state = PROJECT_STATE_EXECUTION;
        break;
    case ProjectState_PAYMENT:
        project_state = PROJECT_STATE_PAYMENT;
        break;
    default:
        break;
    }

    // do we have such project?
    ScribaProject *duplicate = scriba_getProject(project_id);
    if (duplicate != NULL)
    {
        if (strategy == SCRIBA_MERGE_REMOTE_OVERRIDE)
        {
            ScribaProject updated_project;

            scriba_id_copy(&(updated_project.id), &project_id);
            updated_project.title = (char *)(project->title()->c_str());
            updated_project.descr = (char *)(project->descr()->c_str());
            scriba_id_copy(&(updated_project.company_id), &company_id);
            updated_project.state = project_state;

            scriba_updateProject(&updated_project);
        }

        // If merge stragegy is LOCAL_OVERRIDE, we just keep local data.
        // Manual merge is not supported yet.
        scriba_freeProjectData(duplicate);
        duplicate = NULL;
    }
    else
    {
        // we don't have this project locally, add it
        scriba_addProjectWithID(project_id,
                                project->title()->c_str(),
                                project->descr()->c_str(),
                                company_id,
                                project_state);
    }

    return false;
}

} // namespace scriba
