#include "bbs.h"
int countlogouts(char filename[STRLEN])
{
    FILE *fp;
    char buf[256];
    int count = 0;

    if ((fp = fopen(filename, "r")) == NULL)
        return 0;
    while (fgets(buf, 255, fp) != NULL) {
        if (strstr(buf, "@logout@") || strstr(buf, "@issue@"))
            count++;
    }
    return count + 1;
}
void user_display(char *filename, int number, int mode)
{
    FILE *fp;
    char buf[256];
    int count = 1;

    clear();
    move(1, 0);
    if ((fp = fopen(filename, "r")) == NULL)
        return;
    while (fgets(buf, 255, fp) != NULL) {
        if (strstr(buf, "@logout@") || strstr(buf, "@issue@")) {
            count++;
            continue;
        }
        if (count == number) {
            if (mode == true)
                showstuff(buf);

            else {
                prints("%s", buf);
            }
        }

        else if (count > number)
            break;

        else
            continue;
    }
    fclose(fp);
    return;
}

void showstuff(buf)
char buf[256];
{
    struct userec douser;
    int frg, i, matchfrg, strlength;
    enum { ST_USERID, ST_USERNAME, ST_REALNAME, ST_ADDRESS, ST_EMAIL, ST_REALEMAIL, ST_IDENT, ST_RGTDAY, ST_NUMLOGINS, ST_NUMPOSTS, ST_LASTTIME, ST_LASTHOST, ST_THISTIME,
        ST_BOARDNAME,
        ST_STAY, ST_ALLTIME,
#ifdef _DETAIL_UEXP_
        ST_TIN, ST_EXP, ST_CEXP, ST_PERF, ST_CCPERF,
#endif                          /*  */
        ST_END
    };
    char numlogins[10], numposts[10], rgtday[35];
    char lasttime[35], thistime[35], stay[10];
    char alltime[20];


#ifdef _DETAIL_UEXP_
    char tin[10], exper[10], ccperf[20], perf[10], exp[10], ccexp[20];


#endif                          /*  */
    char *stuffstr[ST_END];
    char *ptr, *ptr2;

    time_t now;
    static char *loglst[] = { "userid", "username", "realname", "address", "email", "realemail", "ident", "rgtday", "bbslog", "pst", "lastlogin", "lasthost", "now", "bbsname", "stay", "alltime",
#ifdef _DETAIL_UEXP_
        "exp", "cexp", "perf", "cperf",
#endif                          /*  */
        NULL, NULL,
    };
	struct userdata ud;

    if (currentuser) {
        douser = *currentuser;
//        memcpy(&ud,&curruserdata,sizeof(curruserdata));
		memcpy(&ud, &(currentmemo->ud), sizeof(ud) );
    }
    else {
        bzero(&douser, sizeof(struct userec));
	bzero(&ud, sizeof(ud));
    }
    stuffstr[ST_USERID] = douser.userid;
    stuffstr[ST_USERNAME] = douser.username;
    stuffstr[ST_REALNAME] = ud.realname;
    stuffstr[ST_ADDRESS] = ud.address;
    stuffstr[ST_EMAIL] = ud.email;
    stuffstr[ST_REALEMAIL] = ud.realemail;
    stuffstr[ST_IDENT] = "";
    stuffstr[ST_RGTDAY] = rgtday;
    stuffstr[ST_NUMLOGINS] = numlogins;
    stuffstr[ST_NUMPOSTS] = numposts;
    stuffstr[ST_LASTTIME] = lasttime;
    stuffstr[ST_LASTHOST] = douser.lasthost;
    stuffstr[ST_THISTIME] = thistime;
    stuffstr[ST_BOARDNAME] = BBS_FULL_NAME;
    stuffstr[ST_STAY] = stay;
    stuffstr[ST_ALLTIME] = alltime;

#ifdef _DETAIL_UEXP_
    stuffstr[ST_EXP] = exp;
    stuffstr[ST_CEXP] = ccexp;
    stuffstr[ST_PERF] = perf;
    stuffstr[ST_CCPERF] = ccperf;

#endif                          /*  */
    now = time(0);

    /*---	modified by period	hide posts/logins	2000-11-02	---*/
#ifdef _DETAIL_UEXP_
    tmpnum = countexp(currentuser);
    sprintf(exp, "%d", tmpnum);
    strcpy(ccexp, cexp(tmpnum));
    tmpnum = countperf(currentuser);
    sprintf(perf, "%d", tmpnum);
    strcpy(ccperf, cperf(tmpnum));

#endif                          /*  */
    sprintf(alltime, "%lu–° ±%lu∑÷÷”", douser.stay / 3600, (douser.stay / 60) % 60);
    sprintf(rgtday, "%24.24s", ctime(&douser.firstlogin));
    sprintf(lasttime, "%24.24s", ctime(&douser.lastlogin));
    sprintf(thistime, "%24.24s", ctime(&now));
    sprintf(stay, "%lu", (time(0) - login_start_time) / 60);

    /*---	modified by period	hide posts/logins	2000-11-02	---*/
#ifndef _DETAIL_UINFO_
    if (uinfo.mode == RMAIL && (!HAS_PERM(currentuser, PERM_ADMINMENU))) {
        strcpy(numlogins, "$log");
        strcpy(numposts, "$pst");
    } else
#endif                          /*  */
    {
        sprintf(numlogins, "%d", douser.numlogins);
        sprintf(numposts, "%d", douser.numposts);
    }
    frg = 1;
    ptr2 = buf;

    do {
        if ((ptr = strchr(ptr2, '$')) != NULL) {
            matchfrg = 0;
            *ptr = '\0';
            outs(ptr2);
            ptr++;
            for (i = 0; i < ST_END; i++) {
                strlength = strlen(loglst[i]);
                if (!strncmp(ptr, loglst[i], strlength)) {

                    /*
                     * ptr2 = ptr+strlength;
                     * for(cnt=0; *(ptr2+cnt) == ' '; cnt++);
                     * sprintf(buf2,"%-*.*s", cnt?strlength+cnt:strlength+1, strlength+cnt,stuffstr[i]);
                     * outs(buf2);
                     * ptr2 += (cnt?(cnt-1):cnt);
                     */
                    outs(stuffstr[i]);
                    ptr2 = ptr + strlength;
                    matchfrg = 1;
                    break;
                }
            }
            if (!matchfrg) {
                outc('$');
                ptr2 = ptr;
            }
        }

        else {
            outs(ptr2);
            frg = 0;
        }
    } while (frg);
    return;
}
