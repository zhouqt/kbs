
#ifndef _LIBAUTH_H_
#define _LIBAUTH_H_

struct bbsauth {
    char userid[16];
    char perm[33];
};

#ifdef __cplusplus
extern "C"
{
#endif

    // return: uid
    int bbs_auth(struct bbsauth *a, const char *userid, const char *passwd, char *fromip);
#ifdef __cplusplus
}
#endif

#endif
