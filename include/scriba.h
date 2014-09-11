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

#ifndef SCRIBA_SCRIBA_H
#define SCRIBA_SCRIBA_H

#ifdef __cplusplus
extern "C"
{
#endif

// library initialization return codes
#define SCRIBA_INIT_SUCCESS             0
#define SCRIBA_INIT_INVALID_ARG         1
#define SCRIBA_INIT_NOT_SUPPORTED       2
#define SCRIBA_INIT_BACKEND_INIT_FAILED 3
#define SCRIBA_INIT_BACKEND_NOT_FOUND   4
#define SCRIBA_INIT_MAX_ERR             SCRIBA_INIT_BACKEND_NOT_FOUND

// single database parameter
struct ScribaDBParam
{
    char *key;
    char *value;
};

// list of database parameters
struct ScribaDBParamList
{
    struct ScribaDBParam *param;
    struct ScribaDBParamList *next;
};

// backend type
enum ScribaDBType
{
    SCRIBA_DB_BUILTIN = 0,      // built-in database backend
    SCRIBA_DB_EXT = 1           // external database backend
};

// database backend structure
struct ScribaDB
{
    char *name;                         // unique backend name
    enum ScribaDBType type;             // backend type
    char *location;                     // location (for external backends only)
};

// initialize libscriba
// returns one of the error codes defined above
int scriba_init(struct ScribaDB *db, struct ScribaDBParamList *pl);
// library clean up, no further use is possible until scriba_init() is called again
void scriba_cleanup();

#ifdef __cplusplus
}
#endif

#endif // SCRIBA_SCRIBA_H
