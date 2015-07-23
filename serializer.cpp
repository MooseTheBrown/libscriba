/*
 * Copyright (C) 2015 Mikhail Sapozhnikov
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
    std::vector<fb::Offset<Company>> comp_offsets;
    std::vector<fb::Offset<Event>> event_offsets;
    std::vector<fb::Offset<POC>> poc_offsets;
    std::vector<fb::Offset<Project>> project_offsets;

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
    fb::Offset<fb::String> company_name;
    fb::Offset<fb::String> company_jur_name;
    fb::Offset<fb::String> company_address;
    fb::Offset<fb::String> company_inn;
    fb::Offset<fb::String> company_phonenum;
    fb::Offset<fb::String> company_email;

    ID bufID(id._high, id._low);
    if (company->name != NULL)
    {
        company_name = fbb.CreateString(company->name);
    }
    if (company->jur_name != NULL)
    {
        company_jur_name = fbb.CreateString(company->jur_name);
    }
    if (company->address != NULL)
    {
        company_address = fbb.CreateString(company->address);
    }
    if (company->inn != NULL)
    {
        company_inn = fbb.CreateString(company->inn);
    }
    if (company->phonenum != NULL)
    {
        company_phonenum = fbb.CreateString(company->phonenum);
    }
    if (company->email != NULL)
    {
        company_email = fbb.CreateString(company->email);
    }

    CompanyBuilder cb(fbb);
    cb.add_id(&bufID);
    if (company->name != NULL)
    {
        cb.add_name(company_name);
    }
    if (company->jur_name != NULL)
    {
        cb.add_jur_name(company_jur_name);
    }
    if (company->address != NULL)
    {
        cb.add_address(company_address);
    }
    if (company->inn != NULL)
    {
        cb.add_inn(company_inn);
    }
    if (company->phonenum != NULL)
    {
        cb.add_phonenum(company_phonenum);
    }
    if (company->email != NULL)
    {
        cb.add_email(company_email);
    }

    scriba_freeCompanyData(company);
    return cb.Finish();
}

static fb::Offset<Event> serialize_event(scriba_id_t id, fb::FlatBufferBuilder &fbb)
{
    ScribaEvent *event = scriba_getEvent(id);
    fb::Offset<fb::String> event_descr;
    fb::Offset<fb::String> event_outcome;

    ID bufID(id._high, id._low);
    ID companyID(event->company_id._high, event->company_id._low);
    ID projectID(event->project_id._high, event->project_id._low);
    ID pocID(event->poc_id._high, event->poc_id._low);
    if (event->descr != NULL)
    {
        event_descr = fbb.CreateString(event->descr);
    }
    if (event->outcome != NULL)
    {
        event_outcome = fbb.CreateString(event->outcome);
    }

    EventBuilder evb(fbb);
    evb.add_id(&bufID);
    evb.add_company_id(&companyID);
    evb.add_project_id(&projectID);
    evb.add_poc_id(&pocID);
    if (event->descr != NULL)
    {
        evb.add_descr(event_descr);
    }
    if (event->outcome != NULL)
    {
        evb.add_outcome(event_outcome);
    }
    switch (event->type)
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

    evb.add_timestamp(event->timestamp);

    switch (event->state)
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
    fb::Offset<fb::String> poc_firstname;
    fb::Offset<fb::String> poc_secondname;
    fb::Offset<fb::String> poc_lastname;
    fb::Offset<fb::String> poc_mobilenum;
    fb::Offset<fb::String> poc_phonenum;
    fb::Offset<fb::String> poc_email;
    fb::Offset<fb::String> poc_position;

    ID bufID(id._high, id._low);
    ID companyID(poc->company_id._high, poc->company_id._low);

    if (poc->firstname != NULL)
    {
        poc_firstname = fbb.CreateString(poc->firstname);
    }
    if (poc->secondname != NULL)
    {
        poc_secondname = fbb.CreateString(poc->secondname);
    }
    if (poc->lastname != NULL)
    {
        poc_lastname = fbb.CreateString(poc->lastname);
    }
    if (poc->mobilenum != NULL)
    {
        poc_mobilenum = fbb.CreateString(poc->mobilenum);
    }
    if (poc->phonenum != NULL)
    {
        poc_phonenum = fbb.CreateString(poc->phonenum);
    }
    if (poc->email != NULL)
    {
        poc_email = fbb.CreateString(poc->email);
    }
    if (poc->position != NULL)
    {
        poc_position = fbb.CreateString(poc->position);
    }

    POCBuilder pb(fbb);
    pb.add_id(&bufID);
    pb.add_company_id(&companyID);
    if (poc->firstname != NULL)
    {
        pb.add_firstname(poc_firstname);
    }
    if (poc->secondname != NULL)
    {
        pb.add_secondname(poc_secondname);
    }
    if (poc->lastname != NULL)
    {
        pb.add_lastname(poc_lastname);
    }
    if (poc->mobilenum != NULL)
    {
        pb.add_mobilenum(poc_mobilenum);
    }
    if (poc->phonenum != NULL)
    {
        pb.add_phonenum(poc_phonenum);
    }
    if (poc->email != NULL)
    {
        pb.add_email(poc_email);
    }
    if (poc->position != NULL)
    {
        pb.add_position(poc_position);
    }

    scriba_freePOCData(poc);
    return pb.Finish();
}

static fb::Offset<Project> serialize_project(scriba_id_t id, fb::FlatBufferBuilder &fbb)
{
    ScribaProject *project = scriba_getProject(id);
    fb::Offset<fb::String> project_title;
    fb::Offset<fb::String> project_descr;

    ID bufID(id._high, id._low);
    ID companyID(project->company_id._high, project->company_id._low);

    if (project->title != NULL)
    {
        project_title = fbb.CreateString(project->title);
    }
    if (project->descr != NULL)
    {
        project_descr = fbb.CreateString(project->descr);
    }

    ProjectBuilder prjb(fbb);
    prjb.add_id(&bufID);
    prjb.add_company_id(&companyID);
    if (project->title != NULL)
    {
        prjb.add_title(project_title);
    }
    if (project->descr != NULL)
    {
        prjb.add_descr(project_descr);
    }
    switch (project->state)
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

    switch (project->currency)
    {
    case SCRIBA_CURRENCY_RUB:
        prjb.add_currency(Currency_RUB);
        break;
    case SCRIBA_CURRENCY_USD:
        prjb.add_currency(Currency_USD);
        break;
    case SCRIBA_CURRENCY_EUR:
        prjb.add_currency(Currency_EUR);
        break;
    }

    prjb.add_cost(project->cost);

    scriba_freeProjectData(project);
    return prjb.Finish();
}

static bool deserialize_company(const Company *company, enum ScribaMergeStrategy strategy)
{
    scriba_id_t company_id;

    company_id._high = (unsigned long long)(company->id()->high());
    company_id._low = (unsigned long long)(company->id()->low());
    char *company_name = NULL;
    char *company_jur_name = NULL;
    char *company_address = NULL;
    char *company_inn = NULL;
    char *company_phonenum = NULL;
    char *company_email = NULL;

    if (company->name() != NULL)
    {
        company_name = (char *)(company->name()->c_str());
    }
    if (company->jur_name() != NULL)
    {
        company_jur_name = (char *)(company->jur_name()->c_str());
    }
    if (company->address() != NULL)
    {
        company_address = (char *)(company->address()->c_str());
    }
    if (company->inn() != NULL)
    {
        company_inn = (char *)(company->inn()->c_str());
    }
    if (company->phonenum() != NULL)
    {
        company_phonenum = (char *)(company->phonenum()->c_str());
    }
    if (company->email() != NULL)
    {
        company_email = (char *)(company->email()->c_str());
    }

    ScribaCompany *duplicate = scriba_getCompany(company_id);
    if (duplicate != NULL)
    {
        if (strategy == SCRIBA_MERGE_REMOTE_OVERRIDE)
        {
            ScribaCompany updated_company;
            scriba_id_copy(&(updated_company.id), &company_id);
            updated_company.name = company_name;
            updated_company.jur_name = company_jur_name;
            updated_company.address = company_address;
            updated_company.inn = company_inn;
            updated_company.phonenum = company_phonenum;
            updated_company.email = company_email;
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
                                company_name,
                                company_jur_name,
                                company_address,
                                company_inn,
                                company_phonenum,
                                company_email);
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
    char *event_descr = NULL;
    char *event_outcome = NULL;

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

    if (event->descr() != NULL)
    {
        event_descr = (char *)(event->descr()->c_str());
    }
    if (event->outcome() != NULL)
    {
        event_outcome = (char *)(event->outcome()->c_str());
    }

    ScribaEvent *duplicate = scriba_getEvent(event_id);
    if (duplicate != NULL)
    {
        if (strategy == SCRIBA_MERGE_REMOTE_OVERRIDE)
        {
            ScribaEvent updatedEvent;

            scriba_id_copy(&(updatedEvent.id), &event_id);
            updatedEvent.descr = event_descr;
            scriba_id_copy(&(updatedEvent.company_id), &company_id);
            scriba_id_copy(&(updatedEvent.poc_id), &poc_id);
            scriba_id_copy(&(updatedEvent.project_id), &project_id);
            updatedEvent.type = event_type;
            updatedEvent.outcome = event_outcome;
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
                              event_descr,
                              company_id,
                              poc_id,
                              project_id,
                              event_type,
                              event_outcome,
                              (scriba_time_t)(event->timestamp()),
                              event_state);
    }

    return false;
}

static bool deserialize_poc(const POC *poc, enum ScribaMergeStrategy strategy)
{
    scriba_id_t poc_id;
    scriba_id_t company_id;
    char *poc_firstname = NULL;
    char *poc_secondname = NULL;
    char *poc_lastname = NULL;
    char *poc_mobilenum = NULL;
    char *poc_phonenum = NULL;
    char *poc_email = NULL;
    char *poc_position = NULL;

    poc_id._high = (unsigned long long)(poc->id()->high());
    poc_id._low = (unsigned long long)(poc->id()->low());
    company_id._high = (unsigned long long)(poc->company_id()->high());
    company_id._low = (unsigned long long)(poc->company_id()->low());

    if (poc->firstname() != NULL)
    {
        poc_firstname = (char *)(poc->firstname()->c_str());
    }
    if (poc->secondname() != NULL)
    {
        poc_secondname = (char *)(poc->secondname()->c_str());
    }
    if (poc->lastname() != NULL)
    {
        poc_lastname = (char *)(poc->lastname()->c_str());
    }
    if (poc->mobilenum() != NULL)
    {
        poc_mobilenum = (char *)(poc->mobilenum()->c_str());
    }
    if (poc->phonenum() != NULL)
    {
        poc_phonenum = (char *)(poc->phonenum()->c_str());
    }
    if (poc->email() != NULL)
    {
        poc_email = (char *)(poc->email()->c_str());
    }
    if (poc->position() != NULL)
    {
        poc_position = (char *)(poc->position()->c_str());
    }

    // do we have this POC locally?
    ScribaPoc *duplicate = scriba_getPOC(poc_id);
    if (duplicate != NULL)
    {
        if (strategy == SCRIBA_MERGE_REMOTE_OVERRIDE)
        {
            ScribaPoc updated_poc;

            scriba_id_copy(&(updated_poc.id), &poc_id);
            updated_poc.firstname = poc_firstname;
            updated_poc.secondname = poc_secondname;
            updated_poc.lastname = poc_lastname;
            updated_poc.mobilenum = poc_mobilenum;
            updated_poc.phonenum = poc_phonenum;
            updated_poc.email = poc_email;
            updated_poc.position = poc_position;
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
                            poc_firstname,
                            poc_secondname,
                            poc_lastname,
                            poc_mobilenum,
                            poc_phonenum,
                            poc_email,
                            poc_position,
                            company_id);
    }

    return false;
}

static bool deserialize_project(const Project *project, enum ScribaMergeStrategy strategy)
{
    scriba_id_t project_id;
    scriba_id_t company_id;
    enum ScribaProjectState project_state = PROJECT_STATE_INITIAL;
    char *project_title = NULL;
    char *project_descr = NULL;
    enum ScribaCurrency project_currency = SCRIBA_CURRENCY_RUB;

    project_id._high = project->id()->high();
    project_id._low = project->id()->low();
    company_id._high = project->company_id()->high();
    company_id._low = project->company_id()->low();
    switch (project->state())
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
    switch (project->currency())
    {
    case Currency_RUB:
        project_currency = SCRIBA_CURRENCY_RUB;
        break;
    case Currency_USD:
        project_currency = SCRIBA_CURRENCY_USD;
        break;
    case Currency_EUR:
        project_currency = SCRIBA_CURRENCY_EUR;
        break;
    }

    if (project->title() != NULL)
    {
        project_title = (char *)(project->title()->c_str());
    }
    if (project->descr() != NULL)
    {
        project_descr = (char *)(project->descr()->c_str());
    }

    // do we have such project?
    ScribaProject *duplicate = scriba_getProject(project_id);
    if (duplicate != NULL)
    {
        if (strategy == SCRIBA_MERGE_REMOTE_OVERRIDE)
        {
            ScribaProject updated_project;

            scriba_id_copy(&(updated_project.id), &project_id);
            updated_project.title = project_title;
            updated_project.descr = project_descr;
            scriba_id_copy(&(updated_project.company_id), &company_id);
            updated_project.state = project_state;
            updated_project.currency = project_currency;
            updated_project.cost = project->cost();

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
                                project_title,
                                project_descr,
                                company_id,
                                project_state,
                                project_currency,
                                project->cost());
    }

    return false;
}

} // namespace scriba
