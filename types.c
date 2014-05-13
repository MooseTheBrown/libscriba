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

#include "types.h"
#include <string.h>
#include <stdlib.h>

// get scriba ID value from string representation
scriba_id_t scriba_id_from_string(const char *str)
{
    return atoll(str);
}

/* INN type handling routines */

// create inn value from ASCII string representation
scriba_inn_t scriba_inn_from_string(const char *str)
{
    int len = 0;
    scriba_inn_t inn;

    memset((void *)&(inn.digits), 0, INN_DIGITS);

    if (str == NULL)
    {
        goto exit;
    }

    len = strlen(str);

    // string must be exactly INN_DIGITS long
    if (len == INN_DIGITS)
    {
        for (int i = 0; i < INN_DIGITS; i++)
        {
            inn.digits[i] = str[i] - '0';
        }
    }

exit:
    return inn;
}

// create ASCII string representation of inn value
char *scriba_inn_to_string(const scriba_inn_t *inn)
{
    if (inn == NULL)
    {
        return NULL;
    }

    char *ret = (char *)malloc(INN_DIGITS + 1);
    if (ret != NULL)
    {
        for (int i = 0; i < INN_DIGITS; i++)
        {
            ret[i] = inn->digits[i] + '0';
        }
        ret[INN_DIGITS] = 0;
    }

    return ret;
}

// compare two inn values; returns 1 (true) or 0 (false)
int scriba_inn_is_equal(const scriba_inn_t *inn1, const scriba_inn_t *inn2)
{
    int ret = 1;

    if ((inn1 != NULL) && (inn2 != NULL))
    {
        for (int i = 0; i < INN_DIGITS; i++)
        {
            if (inn1->digits[i] != inn2->digits[i])
            {
                ret = 0;
                break;
            }
        }
    }
    else
    {
        ret = 0;
    }

    return ret;
}

// copy inn value from source to destination
void scriba_copy_inn(scriba_inn_t *dest, const scriba_inn_t *src)
{
    if ((dest != NULL) && (src != NULL))
    {
        memcpy((void*)&(dest->digits), (void*)&(src->digits), INN_DIGITS);
    }
}

/* List type handling routines */

// init new list
scriba_list_t *scriba_list_init()
{
    scriba_list_t *new_list = (scriba_list_t *)malloc(sizeof (scriba_list_t));
    new_list->id = 0;
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
            head->id = id;
            head->text = new_text;
            head->init = 1;
        }
        else
        {
            // head is not empty, create new node
            scriba_list_t *new_item = (scriba_list_t *)malloc(sizeof (scriba_list_t));
            memset((void *)new_item, 0, sizeof (scriba_list_t));
            new_item->id = id;
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
