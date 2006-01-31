#include <fcntl.h>
#include "system.h"


int f_mv(const char *src, const char *dst)
{
    int ret;

    if ((ret = rename(src, dst))!=NULL) {
        ret = f_cp(src, dst, O_TRUNC);
        if (!ret)
            unlink(src);
    }
    return ret;
}
