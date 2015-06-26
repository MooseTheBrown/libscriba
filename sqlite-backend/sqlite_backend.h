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

#include "db_backend.h"

#define SCRIBA_SQLITE_BACKEND_NAME "scriba_sqlite"
#define SCRIBA_SQLITE_DB_LOCATION_PARAM "db_loc"
// see SQLite PRAGMA synchronous
#define SCRIBA_SQLITE_DB_SYNC_PARAM     "db_sync"
#define SCRIBA_SQLITE_DB_SYNC_OFF       "off"
#define SCRIBA_SQLITE_DB_SYNC_ON        "on"

extern struct ScribaInternalDB sqliteDB;

int scriba_sqlite_init(struct ScribaDBParamList *pl, struct ScribaDBFuncTbl *fTbl);

void scriba_sqlite_cleanup();
