#ifndef INNTOBBS_H
#define INNTOBBS_H

#include "daemon.h"

enum HeaderValue {
    SUBJECT_H, FROM_H, DATE_H, MID_H, NEWSGROUPS_H,
    NNTPPOSTINGHOST_H, NNTPHOST_H, CONTROL_H, PATH_H,
    ORGANIZATION_H, X_Auth_From_H, APPROVED_H, DISTRIBUTION_H,
    REFERENCES_H, KEYWORDS_H, SUMMARY_H,
    LASTHEADER,
};

extern char *HEADER[];
extern char *BODY;
extern char *FROM, *SUBJECT, *SITE, *DATE, *POSTHOST, *NNTPHOST, *PATH, *GROUPS, *MSGID, *CONTROL;
extern char *REMOTEHOSTNAME, *REMOTEUSERNAME;

int receive_article();
void echomaillog();
void feedfplog(newsfeeds_t *nf, char *filepath, int type);
void bbsfeedslog(char *filepath, int type);
void init_echomailfp();
void init_bbsfeedsfp();
void readlines(ClientType *client);

#endif
