#include "db_backend.h"

#define SCRIBA_SQLITE_BACKEND_NAME "scriba_sqlite"
#define SCRIBA_SQLITE_DB_LOCATION_PARAM "db_loc"

extern struct ScribaInternalDB sqliteDB;

int scriba_sqlite_init(struct ScribaDBParamList *pl, struct ScribaDBFuncTbl *fTbl);

void scriba_sqlite_cleanup();
