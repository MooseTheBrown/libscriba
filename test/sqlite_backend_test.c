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

#include "sqlite_backend_test.h"
#include "sqlite_backend.h"
#include "scriba.h"
#include <stdlib.h>
#include <unistd.h>

#define TEST_DB_LOCATION "./test_sqlite_db"

int sqlite_backend_test_init()
{
    int ret = 0;

    struct ScribaDB db;
    db.name = SCRIBA_SQLITE_BACKEND_NAME;
    db.type = SCRIBA_DB_BUILTIN;
    db.location = NULL;

    struct ScribaDBParam param;
    param.key = SCRIBA_SQLITE_DB_LOCATION_PARAM;
    param.value = TEST_DB_LOCATION;

    struct ScribaDBParamList paramList;
    paramList.param = &param;
    paramList.next = NULL;
    
    if (scriba_init(&db, &paramList) != SCRIBA_INIT_SUCCESS)
    {
        ret = 1;
    }

    return ret;
}

int sqlite_backend_test_cleanup()
{
    scriba_cleanup();
    unlink(TEST_DB_LOCATION);

    return 0;
}
