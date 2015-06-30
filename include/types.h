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

#ifndef SCRIBA_TYPES_H
#define SCRIBA_TYPES_H

#ifdef __cplusplus
extern "C"
{
#endif

#define SCRIBA_ID_BLOB_SIZE     16      // size of scriba id binary blob in bytes

// unique record id
typedef struct
{
    unsigned long long _high;
    unsigned long long _low;
} scriba_id_t;

// create new scriba id
void scriba_id_create(scriba_id_t *id);

// zero-initialize scriba id
void scriba_id_zero_init(scriba_id_t *id);

// compare two ids; returns 1 if ids match, 0 otherwise
int scriba_id_compare(const scriba_id_t *id1, const scriba_id_t *id2);

// convert scriba id to NULL-terminated string
char *scriba_id_to_string(const scriba_id_t *id);

// get scriba ID value from string representation
void scriba_id_from_string(const char *str, scriba_id_t *id);

// convert scriba id to 16-byte binary blob
void *scriba_id_to_blob(const scriba_id_t *id);

// restore scriba id from 16-byte binary blob
void scriba_id_from_blob(const void *blob, scriba_id_t *id);

// copy scriba id
void scriba_id_copy(scriba_id_t *dest, const scriba_id_t *src);

// list of ids
typedef struct _scriba_list
{
    scriba_id_t id;                 // id
    char *text;                     // optional description text, depends on item type
    struct _scriba_list *next;      // pointer to the next item

    // the following field is for internal use only
    char init;
} scriba_list_t;

// init new list
scriba_list_t *scriba_list_init();
// add new item to the end of the list
void scriba_list_add(scriba_list_t *head, scriba_id_t id, char *text);
// free memory occupied by the list
void scriba_list_delete(scriba_list_t *head);
// iterate through the list
#define scriba_list_for_each(list, item) for (scriba_list_t *item = list; (item != NULL) && (list->init != 0); item = item->next)
// check whether list is empty or not
int scriba_list_is_empty(scriba_list_t *list);

// timestamp in seconds since Epoch
typedef long long scriba_time_t;

#ifdef __cplusplus
}
#endif

#endif // SCRIBA_TYPES_H
