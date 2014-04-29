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
