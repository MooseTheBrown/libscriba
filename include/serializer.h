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

#ifndef SERIALIZER_H
#define SERIALIZER_H

#include "types.h"

#ifdef __cplusplus
extern "C"
{
#endif

// different merge strageties used when importing entries from a binary buffer
enum ScribaMergeStrategy
{
    SCRIBA_MERGE_LOCAL_OVERRIDE = 0,    // leave local data intact in case of conflicts
    SCRIBA_MERGE_REMOTE_OVERRIDE,       // overwrite local data in case of conflicts
    SCRIBA_MERGE_MANUAL
    /* When manual merge is used during de-serialization, the library creates
     * additional entry in the database for each conflicting object. This entry
     * is marked as duplicate of specific scriba ID, so that application could
     * retrieve all such entries and allow user to manually merge their data
     * before removing duplicates. */
};

// outcome of binary buffer data merge with local DB
enum ScribaMergeStatus
{
    SCRIBA_MERGE_OK = 0,
    SCRIBA_MERGE_CONFLICTS
};

// serialize the given entries into binary buffer and return the buffer pointer
// buflen will contain buffer size
void *serialize(scriba_list_t *companies,
                scriba_list_t *events,
                scriba_list_t *people,
                scriba_list_t *projects,
                unsigned long *buflen);

// read entry data from the given buffer and store it in the local database
// according to the given merge strategy
enum ScribaMergeStatus deserialize(void *buf, unsigned long buflen,
                                   enum ScribaMergeStrategy strategy);

#ifdef __cplusplus
}
#endif

#endif // SERIALIZER_H
