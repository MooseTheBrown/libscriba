#ifndef SCRIBA_TYPES_H
#define SCRIBA_TYPES_H

#define INN_DIGITS 10   // number of digits in INN

typedef long long scriba_id_t;

// get scriba ID value from string representation
scriba_id_t scriba_id_from_string(const char *str);

// tax payer ID (INN) type
typedef struct
{
    unsigned char digits[INN_DIGITS];
} scriba_inn_t;

// create inn value from ASCII string representation
scriba_inn_t scriba_inn_from_string(const char *str);
/* create ASCII string representation of inn value
   memory for returned string is allocated dynamically, it is up to calling code to free it
   when it's no longer needed */
char *scriba_inn_to_string(const scriba_inn_t *inn);
// compare two inn values; returns 1 (true) or 0 (false)
int scriba_inn_is_equal(const scriba_inn_t *inn1, const scriba_inn_t *inn2);
// copy inn value from source to destination
void scriba_copy_inn(scriba_inn_t *dest, const scriba_inn_t *src);

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

#endif // SCRIBA_TYPES_H