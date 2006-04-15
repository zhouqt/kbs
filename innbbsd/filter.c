#include "innbbsconf.h"

#include "bbslib.h"
#include "inn_funcs.h"

/*
#  filter.conf
#node    in    out
mynode:b2g:g2b
*/

char *big2gb(), *gb2big();

void conv_init();

typedef struct Filternode_t {
    char *name;
    char *(*cmd) ();
    void (*cmd_init) ();
} filternode_t;

static filternode_t FILTERS[] = {
#ifdef BIG2GB
    {"big2gb", big2gb, conv_init},
#endif
#ifdef GB2BIG
    {"gb2big", gb2big, conv_init},
#endif
    {"\0", NULL, NULL},
};

int filtermatch(int result, char *target, char *pat)
{
    char *filterp = pat, *ptr;
    char *arg;

    for (filterp = pat, ptr = strchr(filterp, ','); filterp && *filterp; ptr = strchr(filterp, ',')) {
        if (ptr)
            *ptr = '\0';
        arg = filterp;
        if (*arg == '!') {
            if (wildmat(target, arg + 1)) {
                result = 0;
            }
        } else if (wildmat(target, arg)) {
            result = 1;
        }
        if (ptr) {
            *ptr = ',';
            filterp = ptr + 1;
        } else {
            break;
        }
    }
    return result;
}

FuncPtr search_filtercmd(cmd)
char *cmd;
{
    filternode_t *nodep;
    char *ptr;
    int savech;

    for (ptr = cmd; *ptr && strchr("\r\n\t\b ", *ptr) == NULL; ptr++);
    savech = *ptr;
    *ptr = '\0';
    for (nodep = FILTERS; nodep && nodep->name && *nodep->name; nodep++) {
        if (strcasecmp(nodep->name, cmd) == 0)
            return nodep->cmd;
    }
    *ptr = savech;
    return NULL;
}

