#include "innbbsconf.h"

#ifdef NO_getdtablesize
#include <sys/time.h>
#include <sys/resource.h>
getdtablesize()
{
    struct rlimit limit;

    if (getrlimit(RLIMIT_NOFILE, &limit) >= 0) {
        return limit.rlim_cur;
    }
    return -1;
}
#endif
