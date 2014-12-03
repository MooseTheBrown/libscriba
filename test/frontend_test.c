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

#include "frontend_test.h"
#include "mock_backend.h"
#include <CUnit/CUnit.h>
#include <stdlib.h>

int frontend_test_init()
{
    int ret = 0;

    struct ScribaInternalDB *mockDB = NULL;

    mockDB = mock_backend_init();
    scriba_addInternalDB(mockDB);

    struct ScribaDB db;
    db.name = mockDB->name;
    db.type = SCRIBA_DB_BUILTIN;
    db.location = NULL;
    
    if (scriba_init(&db, NULL) != SCRIBA_INIT_SUCCESS)
    {
        ret = 1;
    }

    return ret;
}

int frontend_test_cleanup()
{
    scriba_cleanup();

    return 0;
}
