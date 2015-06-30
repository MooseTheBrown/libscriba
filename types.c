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

#include "types.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef SCRIBA_UUID_FILE
#define SCRIBA_UUID_FILE "/proc/sys/kernel/random/uuid"
#endif

#define PROCFS_UUID_STR_LENGTH 38



static void scriba_id_remove_extra_symbols(const char *in, char *out);



/* scriba id type handling routines */

static void scriba_id_remove_extra_symbols(const char *in, char *out)
{
    if ((in == NULL) || (out == NULL))
    {
        return;
    }

    while (*in)
    {
        if (((*in >= '0') && (*in <= '9')) ||
            ((*in >= 'a') && (*in <= 'f')) ||
            ((*in >= 'A') && (*in <= 'F')))
        {
            *out = *in;
            out++;
        }
        in++;
    }
}

// create new scriba id
void scriba_id_create(scriba_id_t *id)
{
    if (id == NULL)
    {
        return;
    }

    FILE *fp = fopen("/proc/sys/kernel/random/uuid", "r");
    char id_str[PROCFS_UUID_STR_LENGTH];

    if (fp == NULL)
    {
        return;
    }

    memset(id_str, 0, PROCFS_UUID_STR_LENGTH);
    memset(id, 0, sizeof (scriba_id_t));

    fread((void *)id_str, 1, PROCFS_UUID_STR_LENGTH, fp);

    scriba_id_from_string(id_str, id);

    fclose(fp);
}

// zero-initialize scriba id
void scriba_id_zero_init(scriba_id_t *id)
{
    if (id == NULL)
    {
        return;
    }

    memset((void *)id, 0, sizeof (scriba_id_t));
}

// compare two ids; returns 1 if ids match, 0 otherwise
int scriba_id_compare(const scriba_id_t *id1, const scriba_id_t *id2)
{
    if ((id1 == NULL) || (id2 == NULL))
    {
        return 0;
    }

    if ((id1->_high == id2->_high) && (id1->_low == id2->_low))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// convert scriba id to NULL-terminated string
char *scriba_id_to_string(const scriba_id_t *id)
{
    if (id == NULL)
    {
        return NULL;
    }

    char *str = (char *)malloc(33); // 32 symbols for 128-bit UUID
    memset((void *)str, 0, 33);
    snprintf(str, 33, "%llx%llx",  id->_high, id->_low);

    return str;
}

// get scriba ID value from string representation
void scriba_id_from_string(const char *str, scriba_id_t *id)
{
    char id_str[33];

    if ((str == NULL) || (id == NULL))
    {
        return;
    }

    if (strlen(str) < 32)
    {
        return;
    }

    memset(id_str, 0, 33);

    scriba_id_remove_extra_symbols(str, id_str);

    char tmp = id_str[16];
    id_str[16] = '\0';
    id->_high = strtoull(id_str, NULL, 16);
    id_str[16] = tmp;
    id->_low = strtoull(&id_str[16], NULL, 16);
}

// convert scriba id to 16-byte binary blob
void *scriba_id_to_blob(const scriba_id_t *id)
{
    if (id == NULL)
    {
        return NULL;
    }

    unsigned char *blob = (unsigned char *)malloc(SCRIBA_ID_BLOB_SIZE);

    // low part
    for (int i = 0; i < 8; i++)
    {
        blob[i] = ((id->_low) >> (8 * i)) & 0xFF;
    }
    // high part
    for (int i = 8; i < 16; i++)
    {
        blob[i] = ((id->_high) >> (8 * (i - 8))) & 0xFF;
    }

    return blob;
}

// restore scriba id from 16-byte binary blob
void scriba_id_from_blob(const void *blob, scriba_id_t *id)
{
    const unsigned char *ptr = (const unsigned char *)blob;

    if ((blob == NULL) || (id == NULL))
    {
        return;
    }

    scriba_id_zero_init(id);

    // low part
    for (int i = 0; i < 8; i++)
    {
        id->_low |= ((unsigned long long)ptr[i] << (8 * i));
    }
    // high part
    for (int i = 8; i < 16; i++)
    {
        id->_high |= ((unsigned long long)ptr[i] << (8 * (i - 8)));
    }
}

// copy scriba id
void scriba_id_copy(scriba_id_t *dest, const scriba_id_t *src)
{
    if ((src == NULL) || (dest == NULL))
    {
        return;
    }

    dest->_high = src->_high;
    dest->_low = src->_low;
}

/* List type handling routines */

// init new list
scriba_list_t *scriba_list_init()
{
    scriba_list_t *new_list = (scriba_list_t *)malloc(sizeof (scriba_list_t));
    scriba_id_zero_init(&(new_list->id));
    new_list->text = NULL;
    new_list->next = NULL;
    new_list->init = 0;
    return new_list;
}

// add new item to the end of the list
void scriba_list_add(scriba_list_t *head, scriba_id_t id, char *text)
{
    if (head != NULL)
    {
        char *new_text = NULL;
        if (text != NULL)
        {
            int len = strlen(text);
            new_text = (char *)malloc(len + 1);
            strcpy(new_text, text);
            new_text[len] = 0;
        }

        if (head->init == 0)
        {
            // head is empty
            scriba_id_copy(&(head->id), &id);
            head->text = new_text;
            head->init = 1;
        }
        else
        {
            // head is not empty, create new node
            scriba_list_t *new_item = (scriba_list_t *)malloc(sizeof (scriba_list_t));
            memset((void *)new_item, 0, sizeof (scriba_list_t));
            scriba_id_copy(&(new_item->id), &id);
            new_item->text = new_text;
            new_item->init = 1;
            while (head->next != NULL)
            {
                head = head->next;
            }
            head->next = new_item;
        }

    }
}

// free memory occupied by the list
void scriba_list_delete(scriba_list_t *head)
{
    while (head != NULL)
    {
        scriba_list_t *item = head;
        head = head->next;

        if (item->text != NULL)
        {
            free(item->text);
        }
        free(item);
    }
}

// check whether list is empty or not
int scriba_list_is_empty(scriba_list_t *list)
{
    if ((list == NULL) || (list->init == 0))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
