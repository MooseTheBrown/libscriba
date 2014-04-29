#ifndef SCRIBA_SCRIBA_H
#define SCRIBA_SCRIBA_H

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

#endif // SCRIBA_SCRIBA_H
