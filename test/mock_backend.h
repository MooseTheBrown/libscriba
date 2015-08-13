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

#ifndef SCRIBA_MOCK_BACKEND_H
#define SCRIBA_MOCK_BACKEND_H

#include "db_backend.h"

struct MockCompanyList
{
    struct ScribaCompany *data;
    struct MockCompanyList *next;
};

struct MockEventList
{
    struct ScribaEvent *data;
    struct MockEventList *next;
};

struct MockPOCList
{
    struct ScribaPoc *data;
    struct MockPOCList *next;
};

struct MockProjectList
{
    struct ScribaProject *data;
    struct MockProjectList *next;
};

struct MockBackendData
{
    // companies
    struct MockCompanyList *companies;

    // events
    struct MockEventList *events;

    // people
    struct MockPOCList *people;

    // projects
    struct MockProjectList *projects;
};

struct ScribaInternalDB *mock_backend_init();
struct MockBackendData *mock_backend_get_data();

#endif // SCRIBA_MOCK_BACKEND_H
