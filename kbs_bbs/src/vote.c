/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include "bbs.h"
#include "vote.h"
extern int page, range;
struct votebal currvote;
struct votelimit currlimit;     /*Haohmaru.99.11.17.¸ù¾İ°æÖ÷ÉèµÄÏŞÖÆÌõ¼şÅĞ¶ÏÊÇ·ñÈÃ¸ÃÊ¹ÓÃÕßÍ¶Æ± */
char controlfile[STRLEN], limitfile[STRLEN];
unsigned int result[33];
int vnum;
int voted_flag;
FILE *sug;
static int mk_result(int num);
static int dele_vote(int num);
static int Show_Votes();
int cmpvuid(userid, uv)
char *userid;
struct ballot *uv;
{
    return !strncmp(userid, uv->uid,IDLEN);
}
int setvoteflag(char *bname, int flag)
{
    int pos;
    struct boardheader fh;

    pos = getboardnum(bname, &fh);
    if (pos) {
        if (flag == 0)
            fh.flag = fh.flag & ~BOARD_VOTEFLAG;
        else
            fh.flag = fh.flag | BOARD_VOTEFLAG;
        set_board(pos, &fh,NULL);
    }
    return 0;
}

void b_report(str)
char *str;
{
    char buf[STRLEN];

    sprintf(buf, "%s %s", currboard->filename, str);
    bbslog("user","%s",buf);
}

void makevdir(bname)
char *bname;
{
    struct stat st;
    char buf[STRLEN];

    sprintf(buf, "vote/%s", bname);
    if (stat(buf, &st) != 0)
        mkdir(buf, 0755);
}

void setvfile(buf, bname, filename)
char *buf, *bname, *filename;
{
    sprintf(buf, "vote/%s/%s", bname, filename);
}

void setcontrolfile()
{
    setvfile(controlfile, currboard->filename, "control");
}

int b_notes_edit()
{
    char buf[STRLEN];
    char ans[4];
    int aborted;
    int oldmode;

    if (!chk_currBM(currBM, currentuser)) {
        return 0;
    }
    clear();
    makevdir(currboard->filename);
    setvfile(buf, currboard->filename, "notes");
    getdata(1, 0, "(E)±à¼­ (D)É¾³ı ±¾ÌÖÂÛÇøµÄ±¸ÍüÂ¼? [E]: ", ans, 2,
            DOECHO, NULL, true);
    if (ans[0] == 'D' || ans[0] == 'd') {
        move(2, 0);
        if (askyn("ÕæµÄÒªÉ¾³ı±¾ÌÖÂÛÇøµÄ±¸ÍüÂ¼", 0)) {
            move(3, 0);
            prints("±¸ÍüÂ¼ÒÑ¾­É¾³ı...\n");
            pressanykey();
            unlink(buf);
            aborted = 1;
        } else
            aborted = -1;
    } else {
        oldmode = uinfo.mode;
        modify_user_mode(EDITUFILE);
        aborted = vedit(buf, false,NULL, NULL);
        modify_user_mode(oldmode);
    }
    if (aborted == -1) {
        pressreturn();
    } else {
        setvfile(buf, currboard->filename, "noterec");
        unlink(buf);
    }
    return FULLUPDATE;
}

int b_sec_notes_edit()
{
    char buf[STRLEN];
    char ans[4];
    int aborted;

    if (!chk_currBM(currBM, currentuser)) {
        return 0;
    }
    clear();
    makevdir(currboard->filename);
    setvfile(buf, currboard->filename, "secnotes");
    getdata(1, 0, "(E)±à¼­ (D)É¾³ı ±¾ÌÖÂÛÇøµÄÃØÃÜ±¸ÍüÂ¼? [E]: ", ans, 2,
            DOECHO, NULL, true);
    if (ans[0] == 'D' || ans[0] == 'd') {
        move(2, 0);
        if (askyn("ÕæµÄÒªÉ¾³ı±¾ÌÖÂÛÇøµÄÃØÃÜ±¸ÍüÂ¼", 0)) {
            move(3, 0);
            prints("ÃØÃÜ±¸ÍüÂ¼ÒÑ¾­É¾³ı...\n");
            pressanykey();
            unlink(buf);
            aborted = 1;
        } else
            aborted = -1;
    } else
        aborted = vedit(buf, false,NULL, NULL);
    if (aborted == -1) {
        pressreturn();
    } else {
        setvfile(buf, currboard->filename, "noterec");
        unlink(buf);
    }
    return FULLUPDATE;
}

int b_jury_edit()
{                               /* stephen 2001.11.1: ±à¼­°æÃæÖÙ²ÃÃûµ¥ */
    char buf[STRLEN];
    char ans[4];
    int aborted;

    if (!((HAS_PERM(currentuser, PERM_JURY)
           && HAS_PERM(currentuser, PERM_BOARDS))
          || HAS_PERM(currentuser, PERM_SYSOP))) {
        return 0;
    }
    clear();
    makevdir(currboard->filename);
    setvfile(buf, currboard->filename, "jury");
    getdata(1, 0, "(E)±à¼­ (D)É¾³ı ±¾ÌÖÂÛÇøÖÙ²ÃÎ¯Ô±Ãûµ¥? (C)È¡Ïû [C]: ",
            ans, 2, DOECHO, NULL, true);
    if (ans[0] == 'D' || ans[0] == 'd') {
        move(2, 0);
        if (askyn("ÕæµÄÒªÉ¾³ı±¾ÌÖÂÛÇøÖÙ²ÃÎ¯Ô±Ãûµ¥", 0)) {
            move(3, 0);
            prints("ÖÙ²ÃÎ¯Ô±Ãûµ¥ÒÑ¾­É¾³ı...\n");
            pressanykey();
            unlink(buf);
            aborted = 111;
        } else
            aborted = -1;
    } else if (ans[0] == 'E' || ans[0] == 'e')
        aborted = vedit(buf, false,NULL, NULL);
    else {
        prints("È¡Ïû");
        aborted = -1;
    }
    if (aborted == -1) {
        pressreturn();
    } else {
        char secu[STRLEN];

        if (aborted == 111) {
            sprintf(secu, "É¾³ı %s °æµÄÖÙ²ÃÎ¯Ô±Ãûµ¥", currboard->filename);
            securityreport(secu, NULL, NULL);
            post_file(currentuser, "", buf, "JuryMail", secu, 0, 2);
        } else {
            sprintf(secu, "ĞŞ¸Ä %s °æµÄÖÙ²ÃÎ¯Ô±Ãûµ¥", currboard->filename);
            securityreport(secu, NULL, NULL);
            post_file(currentuser, "", buf, "syssecurity", secu, 0, 2);
            post_file(currentuser, "", buf, "JuryMail", secu, 0, 2);
        }
        setvfile(buf, currboard->filename, "juryrec");
        unlink(buf);
    }
    return FULLUPDATE;
}

int b_suckinfile(fp, fname)
FILE *fp;
char *fname;
{
    char inbuf[256];
    FILE *sfp;

    if ((sfp = fopen(fname, "r")) == NULL)
        return -1;
    while (fgets(inbuf, sizeof(inbuf), sfp) != NULL)
        fputs(inbuf, fp);
    fclose(sfp);
    return 0;
}

int vote_close()
{
    time_t closetime;

    closetime = currvote.opendate + currvote.maxdays * 86400;
    if (closetime <= time(NULL)) {
        mk_result(vnum);
    }
    return 0;
}

int b_close(struct boardheader *fh,void* arg)
{
    int end;

    currboard= fh;
    currboardent=getbnum(fh->filename);
    setcontrolfile();
    end = get_num_records(controlfile, sizeof(currvote));
    for (vnum = end; vnum >= 1; vnum--) {
        get_record(controlfile, &currvote, sizeof(currvote), vnum);
        vote_close();
    }
    return 0;
}

int b_closepolls()
{
    struct stat st;
    FILE *cfp;
    char buf[80];
    time_t now;
    struct boardheader* bh;
    int bid;

    now = time(NULL);
    strcpy(buf, "vote/lastpolling");
    if (stat(buf, &st) != -1 && st.st_mtime > now - 3600) {
        return 0;
    }
    move(t_lines - 1, 0);
    prints("¶Ô²»Æğ£¬ÏµÍ³¹Ø±ÕÍ¶Æ±ÖĞ£¬ÇëÉÔºò...");
    refresh();
    if ((cfp = fopen(buf, "w")) == NULL) {
        bbslog("3error","%s","lastpoll write error");
        return 0;
    }
    fprintf(cfp, "%s", ctime(&now));
    bh=currboard;
    bid=currboardent;
    fclose(cfp);
    apply_boards(b_close,NULL);
    currboard=bh;
    currboardent=bid;
    return 0;
}
int count_result(struct ballot *ptr, int idx, char *arg)
{
    int i;

    if (ptr == NULL) {
        if (sug != NULL) {
            fclose(sug);
            sug = NULL;
        }
        return 0;
    }
    if (ptr->msg[0][0] != '\0') {
        if (currvote.type == VOTE_ASKING) {
            fprintf(sug, "\033[44m%s µÄ×÷´ğÈçÏÂ£º\033[m\n", ptr->uid);
        } else
            fprintf(sug, "\033[44m%s µÄ½¨ÒéÈçÏÂ£º\033[m\n", ptr->uid);
        for (i = 0; i < 3; i++)
            fprintf(sug, "%s\n", ptr->msg[i]);
    }
    result[32]++;
    if (currvote.type == VOTE_ASKING) {
        return 0;
    }
    if (currvote.type != VOTE_VALUE) {
        for (i = 0; i < 32; i++) {
            if ((ptr->voted >> i) & 1)
                (result[i])++;
        }
    } else {
        result[31] += ptr->voted;
        result[(ptr->voted * 10) / (currvote.maxtkt + 1)]++;
    }
    return 0;
}

int get_result_title()
{
    char buf[STRLEN];

    if (currlimit.numlogins || currlimit.numposts || currlimit.stay
        || currlimit.day) {
        fprintf(sug, "¡Ñ ´Ë´ÎÍ¶Æ±µÄÔÊĞí×Ê¸ñÎª:\n");
        fprintf(sug, "1. ÉÏÕ¾´ÎÊıĞè´óÓÚ %d ´Î .\n", currlimit.numlogins);
        fprintf(sug, "2. ÎÄÕÂÊıÄ¿Ğè´óÓÚ %d Æª.\n", currlimit.numposts);
        fprintf(sug, "3. ÉÏÕ¾×ÜÊ±ÊıĞè´óÓÚ %d Ğ¡Ê±.\n", currlimit.stay);
        fprintf(sug, "4. ÉÏÕ¾ÎïÀíÊ±¼äĞè´óÓÚ %d Ìì.\n", currlimit.day);
    }
    if (currvote.type < 1 || currvote.type > 5)
        currvote.type = 1;
    fprintf(sug, "¡Ñ Í¶Æ±¿ªÆôÓÚ£º%.24s  Àà±ğ£º%s\n",
            ctime(&currvote.opendate)
            , vote_type[currvote.type - 1]);
    fprintf(sug, "¡Ñ Ö÷Ìâ£º%s\n", currvote.title);
    if (currvote.type == VOTE_VALUE)
        fprintf(sug, "¡Ñ ´Ë´ÎÍ¶Æ±µÄÖµ²»¿É³¬¹ı£º%d\n\n", currvote.maxtkt);
    fprintf(sug, "¡Ñ Æ±Ñ¡ÌâÄ¿ÃèÊö£º\n\n");
    sprintf(buf, "vote/%s/desc.%lu", currboard->filename, currvote.opendate);
    b_suckinfile(sug, buf);
    return 0;
}
static int mk_result(int num)
{
    char fname[255], nname[255];
    char sugname[255];
    char title[255];
    int i;
    unsigned int total = 0;

    setcontrolfile();
    sprintf(fname, "vote/%s/flag.%lu", currboard->filename, currvote.opendate);
    count_result(NULL, NULL, 0);
    sprintf(sugname, "vote/%s/tmp.%d", currboard->filename, getpid());
    if ((sug = fopen(sugname, "w")) == NULL) {
        bbslog("3error","%s","open vote tmp file error");
        prints("Error: ½áÊøÍ¶Æ±´íÎó...\n");
        pressanykey();
    }
    (void) memset(result, 0, sizeof(result));
    if (apply_record
        (fname, (APPLY_FUNC_ARG) count_result, sizeof(struct ballot), 0, 0,
         false) == -1) {
        bbslog("user","%s","Vote apply flag error");
    }
    fprintf(sug,
            "[44m[36m¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©ÈÊ¹ÓÃÕß%s©À¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª[m\n\n\n",
            (currvote.type != VOTE_ASKING) ? "½¨Òé»òÒâ¼û" : "´Ë´ÎµÄ×÷´ğ");
    fclose(sug);
    sug = NULL;
    sprintf(nname, "vote/%s/results", currboard->filename);
    if ((sug = fopen(nname, "w")) == NULL) {
        bbslog("user","%s","open vote newresult file error");
        prints("Error: ½áÊøÍ¶Æ±´íÎó...\n");
    }
    /*    fprintf( sug, "** Í¶Æ±¿ªÆôì¶£º[1m%.24s[m  Àà±ğ£º[1m%s[m\n", ctime( &currvote.opendate )
       ,vote_type[currvote.type-1]);
       fprintf( sug, "** Ö÷Ìâ£º[1m%s[m\n",currvote.title);
       if(currvote.type==VOTE_VALUE)
       fprintf( sug, "** ´Ë´ÎÍ¶Æ±µÄÖµ²»¿É³¬¹ı£º[1m%d[m\n\n" ,currvote.maxtkt);
       fprintf( sug, "** Æ±Ñ¡ÌâÄ¿ÃèÊö£º\n\n" );
       sprintf( buf, "vote/%s/desc.%d",currboard->filename,currvote.opendate );
       b_suckinfile( sug, buf ); */
    get_result_title();
    fprintf(sug, "** Í¶Æ±½á¹û:\n\n");
    if (currvote.type == VOTE_VALUE) {
        total = result[32];
        for (i = 0; i < 10; i++) {
            fprintf(sug,
                    "[1m  %4d[m µ½ [1m%4d[m Ö®¼äÓĞ [1m%4d[m Æ±  Ô¼Õ¼ [1m%d%%[m\n",
                    (i * currvote.maxtkt) / 10 + ((i == 0) ? 0 : 1),
                    ((i + 1) * currvote.maxtkt) / 10, result[i]
                    , (result[i] * 100) / ((total <= 0) ? 1 : total));
        }
        fprintf(sug, "´Ë´ÎÍ¶Æ±½á¹ûÆ½¾ùÖµÊÇ: [1m%d[m\n",
                result[31] / ((total <= 0) ? 1 : total));
    } else if (currvote.type == VOTE_ASKING) {
        total = result[32];
    } else {
        for (i = 0; i < currvote.totalitems; i++) {
            total += result[i];
        }
        for (i = 0; i < currvote.totalitems; i++) {
            /*            fprintf(sug, "(%c) %-40s  %4d Æ±  Ô¼Õ¼ [1m%d%%[m\n",'A'+i,
               currvote.items[i], result[i] , (result[i]*100)/((total<=0)?1:total)); 
             */
            fprintf(sug, "(%c) %-40s  %4d Æ±  Ô¼Õ¼ [1m%d%%[m\n", 'A' + i,
                    currvote.items[i], result[i],
                    (result[i] * 100) /
                    ((result[32] <= 0) ? 1 : result[32]));
        }
    }
    fprintf(sug, "\nÍ¶Æ±×ÜÈËÊı = [1m%d[m ÈË\n", result[32]);
    fprintf(sug, "Í¶Æ±×ÜÆ±Êı =[1m %d[m Æ±\n\n", total);
    fprintf(sug,
            "[44m[36m¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©ÈÊ¹ÓÃÕß%s©À¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª[m\n\n\n",
            (currvote.type != VOTE_ASKING) ? "½¨Òé»òÒâ¼û" : "´Ë´ÎµÄ×÷´ğ");
    b_suckinfile(sug, sugname);
    unlink(sugname);
    fclose(sug);
    sug = NULL;
    sprintf(title, "[¹«¸æ] %s °æµÄÍ¶Æ±½á¹û", currboard->filename);
    mail_file("deliver", nname, currvote.userid, title, 0, NULL);
#ifdef NINE_BUILD
    if (1)
#else
    if (normal_board(currboard->filename))
#endif
    {
        post_file(currentuser, "", nname, "vote", title, 0, 1);
    }
    post_file(currentuser, "", nname, currboard->filename, title, 0, 1);
    dele_vote(num);
    return 0;
}
int check_result(int num)
{
    char fname[STRLEN], nname[STRLEN];
    char sugname[STRLEN];
    char title[STRLEN];
    int i;
    unsigned int total = 0;

    setcontrolfile();
    sprintf(fname, "vote/%s/flag.%lu", currboard->filename, currvote.opendate);
    count_result(NULL, 0, 0);
    sprintf(sugname, "vote/%s/tmp.%d", currboard->filename, getpid());
    if ((sug = fopen(sugname, "w")) == NULL) {
        bbslog("user","%s","open vote tmp file error");
        prints("Error: ¼ì²éÍ¶Æ±´íÎó...\n");
        pressanykey();
    }
    (void) memset(result, 0, sizeof(result));
    if (apply_record
        (fname, (APPLY_FUNC_ARG) count_result, sizeof(struct ballot), 0, 0,
         false) == -1) {
        bbslog("user","%s","Vote apply flag error");
    }
    fprintf(sug,
            "[44m[36m¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©ÈÊ¹ÓÃÕß%s©À¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª[m\n\n\n",
            (currvote.type != VOTE_ASKING) ? "½¨Òé»òÒâ¼û" : "´Ë´ÎµÄ×÷´ğ");
    fclose(sug);
    sug = NULL;
    sprintf(nname, "vote/%s/results", currboard->filename);
    if ((sug = fopen(nname, "w")) == NULL) {
        bbslog("user","%s","open vote newresult file error");
        prints("Error: ½áÊøÍ¶Æ±´íÎó...\n");
    }
    get_result_title();
    fprintf(sug, "** Í¶Æ±½á¹û:\n\n");
    if (currvote.type == VOTE_VALUE) {
        total = result[32];
        for (i = 0; i < 10; i++) {
            fprintf(sug,
                    "[1m  %4d[m µ½ [1m%4d[m Ö®¼äÓĞ [1m%4d[m Æ±  Ô¼Õ¼ [1m%d%%[m\n",
                    (i * currvote.maxtkt) / 10 + ((i == 0) ? 0 : 1),
                    ((i + 1) * currvote.maxtkt) / 10, result[i]
                    , (result[i] * 100) / ((total <= 0) ? 1 : total));
        }
        fprintf(sug, "´Ë´ÎÍ¶Æ±½á¹ûÆ½¾ùÖµÊÇ: [1m%d[m\n",
                result[31] / ((total <= 0) ? 1 : total));
    } else if (currvote.type == VOTE_ASKING) {
        total = result[32];
    } else {
        for (i = 0; i < currvote.totalitems; i++) {
            total += result[i];
        }
        for (i = 0; i < currvote.totalitems; i++) {
            fprintf(sug, "(%c) %-40s  %4d Æ±  Ô¼Õ¼ [1m%d%%[m\n", 'A' + i,
                    currvote.items[i], result[i],
                    (result[i] * 100) /
                    ((result[32] <= 0) ? 1 : result[32]));
        }
    }
    fprintf(sug, "\nÍ¶Æ±×ÜÈËÊı = [1m%d[m ÈË\n", result[32]);
    fprintf(sug, "Í¶Æ±×ÜÆ±Êı =[1m %d[m Æ±\n\n", total);
    fprintf(sug,
            "[44m[36m¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©ÈÊ¹ÓÃÕß%s©À¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª[m\n\n\n",
            (currvote.type != VOTE_ASKING) ? "½¨Òé»òÒâ¼û" : "´Ë´ÎµÄ×÷´ğ");
    b_suckinfile(sug, sugname);
    unlink(sugname);
    fclose(sug);
    sug = NULL;
    sprintf(title, "[¼ì²é] %s °æµÄÍ¶Æ±½á¹û", currboard->filename);
    mail_file(currentuser->userid, nname, currentuser->userid, title, BBSPOST_MOVE, NULL);
    return 0;
}

int get_vitems(bal)
struct votebal *bal;
{
    int num;
    char buf[STRLEN];

    move(3, 0);
    prints("ÇëÒÀĞòÊäÈë¿ÉÑ¡ÔñÏî, °´ ENTER Íê³ÉÉè¶¨.\n");
    num = 0;
    for (num = 0; num < 32; num++) {
        sprintf(buf, "%c) ", num + 'A');
        getdata((num % 16) + 4, (num / 16) * 40, buf, bal->items[num], 36,
                DOECHO, NULL, true);
        if (strlen(bal->items[num]) == 0) {
            if (num != 0)
                break;
            num = -1;
        }
    }
    bal->totalitems = num;
    return num;
}

int vote_maintain(bname)
char *bname;
{
    char buf[STRLEN * 2];
    struct votebal *ball = &currvote;
    struct votelimit *v_limit = &currlimit;
    int aborted;

    setcontrolfile();
    if (!HAS_PERM(currentuser, PERM_SYSOP))
            if (!chk_currBM(currBM, currentuser)) {
                return 0;
            }
    stand_title("¿ªÆôÍ¶Æ±Ïä");
    makevdir(bname);
    for (;;) {
        getdata(2, 0,
                "(1)ÊÇ·Ç, (2)µ¥Ñ¡, (3)¸´Ñ¡, (4)ÊıÖµ (5)ÎÊ´ğ (6)È¡Ïû ? : ",
                genbuf, 2, DOECHO, NULL, true);
        genbuf[0] -= '0';
        if (genbuf[0] == 6) {
            prints("È¡Ïû´Ë´ÎÍ¶Æ±\n");
            sleep(1);
            return FULLUPDATE;
        }
        if (genbuf[0] < 1 || genbuf[0] > 5)
            continue;
        ball->type = (int) genbuf[0];
        break;
    }
    ball->opendate = time(NULL);
    prints("°´ÈÎºÎ¼ü¿ªÊ¼±à¼­´Ë´Î [Í¶Æ±µÄÃèÊö]: \n");
    pressanykey();
    setvfile(genbuf, bname, "desc");
    sprintf(buf, "%s.%lu", genbuf, ball->opendate);
    aborted = vedit(buf, false, NULL, NULL);
    if (aborted) {
        clear();
        prints("È¡Ïû´Ë´ÎÍ¶Æ±\n");
        pressreturn();
        return FULLUPDATE;
    }
    clear();
    getdata(0, 0, "´Ë´ÎÍ¶Æ±ËùĞëÌìÊı (²»¿É£°Ìì): ", buf, 4, DOECHO, NULL,
            true);
    if (*buf == '\n' || atoi(buf) == 0 || *buf == '\0')
        strcpy(buf, "1");
    ball->maxdays = atoi(buf);
    for (;;) {
        getdata(1, 0, "Í¶Æ±ÏäµÄ±êÌâ: ", ball->title, 61, DOECHO, NULL,
                true);
        if (strlen(ball->title) > 0)
            break;
        bell();
    }
    switch (ball->type) {
    case VOTE_YN:
        ball->maxtkt = 0;
        strcpy(ball->items[0], "ÔŞ³É  £¨ÊÇµÄ£©");
        strcpy(ball->items[1], "²»ÔŞ³É£¨²»ÊÇ£©");
        strcpy(ball->items[2], "Ã»Òâ¼û£¨²»Çå³ş£©");
        ball->maxtkt = 1;
        ball->totalitems = 3;
        break;
    case VOTE_SINGLE:
        get_vitems(ball);
        ball->maxtkt = 1;
        break;
    case VOTE_MULTI:
        get_vitems(ball);
        for (;;) {
            getdata(21, 0, "Ò»¸öÈË×î¶à¼¸Æ±? [1]: ", buf, 5, DOECHO, NULL,
                    true);
            ball->maxtkt = atoi(buf);
            if (ball->maxtkt <= 0)
                ball->maxtkt = 1;
            if (ball->maxtkt > ball->totalitems)
                continue;
            break;
        }
        break;
    case VOTE_VALUE:
        for (;;) {
            getdata(3, 0, "ÊäÈëÊıÖµ×î´ó²»µÃ³¬¹ı [100] : ", buf, 7, DOECHO,
                    NULL, true);
            ball->maxtkt = atoi(buf);
            if (ball->maxtkt <= 0)
                ball->maxtkt = 100;
            break;
        }
        break;
    case VOTE_ASKING:
        /*                    getdata(3,0,"´ËÎÊ´ğÌâ×÷´ğĞĞÊıÖ®ÏŞÖÆ :",buf,3,DOECHO,NULL,true) ;
           ball->maxtkt = atof(buf) ;
           if(ball->maxtkt <= 0) ball->maxtkt = 10; */
        ball->maxtkt = 0;
        currvote.totalitems = 0;
        break;
    }
    setvoteflag(bname, 1);
    clear();
    /*Haohmaru.99.11.17.¸ù¾İÍ¶Æ±¹ÜÀíÔ±ÉèµÄÏŞÖÆÌõ¼şÅĞ¶ÏÊÇ·ñÈÃ¸ÃÊ¹ÓÃÕßÍ¶Æ± */
    if (HAS_PERM(currentuser, PERM_SYSOP)
        || HAS_PERM(currentuser, PERM_JURY)) {
        getdata(1, 0, "ÊÇ·ñ¶ÔÍ¶Æ±×Ê¸ñ½øĞĞÏŞÖÆ(Y/N) [Y]:", buf, 3, DOECHO,
                NULL, true);
        if (buf[0] != 'N' && buf[0] != 'n') {
            getdata(2, 0,
                    "ÇëÊäÈë¶ÔÉÏÕ¾´ÎÊıµÄÏŞÖÆ(0ÎªÃ»ÓĞÏŞÖÆ),ÉÏÕ¾´ÎÊı´óÓÚ:",
                    buf, 5, DOECHO, NULL, true);
            v_limit->numlogins = atoi(buf);
            getdata(3, 0,
                    "ÇëÊäÈë¶ÔÎÄÕÂÊıÄ¿µÄÏŞÖÆ(0ÎªÃ»ÓĞÏŞÖÆ),ÎÄÕÂÊıÄ¿´óÓÚ:",
                    buf, 5, DOECHO, NULL, true);
            v_limit->numposts = atoi(buf);
            getdata(4, 0,
                    "ÇëÊäÈë¶ÔÉÏÕ¾×ÜÊ±ÊıµÄÏŞÖÆ(0ÎªÃ»ÓĞÏŞÖÆ),ÉÏÕ¾×ÜÊ±Êı´óÓÚ(Ğ¡Ê±):",
                    buf, 5, DOECHO, NULL, true);
            v_limit->stay = atoi(buf);
            getdata(5, 0,
                    "ÇëÊäÈë¶ÔÉÏÕ¾ÎïÀíÊ±¼äµÄÏŞÖÆ(0ÎªÃ»ÓĞÏŞÖÆ),ÉÏÕ¾ÎïÀíÊ±¼ä´óÓÚ(Ìì):",
                    buf, 5, DOECHO, NULL, true);
            v_limit->day = atoi(buf);
        } else {
            v_limit->numlogins = 0;
            v_limit->numposts = 0;
            v_limit->stay = 0;
            v_limit->day = 0;
        }
    } else {
        v_limit->numlogins = 0;
        v_limit->numposts = 0;
        v_limit->stay = 0;
        v_limit->day = 0;
    }
    clear();
    sprintf(limitfile, "vote/%s/limit.%lu", bname, ball->opendate);
    if (append_record(limitfile, v_limit, sizeof(struct votelimit)) == -1) {
        prints("·¢ÉúÑÏÖØµÄ´íÎó£¬ÎŞ·¨Ğ´ÈëÏŞÖÆÎÄ¼ş£¬ÇëÍ¨¸æÕ¾³¤");
        b_report("Append limit file Error!!");
    }
    /*Haohmaru.99.10.26.add below 8 lines */
    getdata(1, 0, "È·¶¨¿ªÆôÍ¶Æ±?[Y] :", buf, 3, DOECHO, NULL, true);
    if (buf[0] == 'N' || buf[0] == 'n') {
        clear();
        prints("È¡Ïû´Ë´ÎÍ¶Æ±\n");
        unlink(limitfile);
        pressreturn();
        return FULLUPDATE;
    }
    strcpy(ball->userid, currentuser->userid);
    if (append_record(controlfile, ball, sizeof(*ball)) == -1) {
        prints("·¢ÉúÑÏÖØµÄ´íÎó£¬ÎŞ·¨¿ªÆôÍ¶Æ±£¬ÇëÍ¨¸æÕ¾³¤");
        b_report("Append Control file Error!!");
    } else {
        char votename[STRLEN];
        int i;

        b_report("OPEN VOTE");
        prints("Í¶Æ±ÏäÒÑ¾­¿ªÆôÁË£¡\n");
        range++;
		gettmpfilename( votename, "votetmp" );
        //sprintf(votename, "tmp/votetmp.%d", getpid());
        if ((sug = fopen(votename, "w")) != NULL) {
            sprintf(buf, "[Í¨Öª] %s ¾Ù°ìÍ¶Æ±£º%s", bname, ball->title);
            get_result_title();
            if (ball->type != VOTE_ASKING && ball->type != VOTE_VALUE) {
                fprintf(sug, "\n¡¾Ñ¡ÏîÈçÏÂ¡¿\n");
                for (i = 0; i < ball->totalitems; i++) {
                    fprintf(sug, "([1m%c[m) %-40s\n", 'A' + i,
                            ball->items[i]);
                }
            }
            fclose(sug);
            sug = NULL;
#ifdef NINE_BUILD
            post_file(currentuser, "", votename, bname, buf, 0, 1);
            post_file(currentuser, "", votename, "vote", buf, 0, 1);
#else
            if (!normal_board(bname)) {
                post_file(currentuser, "", votename, bname, buf, 0, 1);
            } else {
                post_file(currentuser, "", votename, "vote", buf, 0, 1);
            }
#endif
            unlink(votename);
        }
    }
    pressreturn();
    return FULLUPDATE;
}
int vote_flag(char *bname, char val, int mode)
{
    char buf[STRLEN], flag;
    int fd, num, size;

    num = usernum - 1;
    switch (mode) {
    case 2:
        sprintf(buf, "Welcome.rec");    /*½øÕ¾µÄ Welcome »­Ãæ */
        break;
    case 1:
        setvfile(buf, bname, "noterec");        /*ÌÖÂÛÇø±¸ÍüÂ¼µÄÆì±ê */
        break;
    default:
        return -1;
    }
    if (num >= MAXUSERS) {
        bbslog("user","%s","Vote Flag, Out of User Numbers");
        return -1;
    }
    if ((fd = open(buf, O_RDWR | O_CREAT, 0600)) == -1) {
        char buf[STRLEN];

        sprintf(buf, "%s Flag file open Error.", bname);
        bbslog("user","%s",buf);
        return -1;
    }
    flock(fd, LOCK_EX);
    size = (int) lseek(fd, 0, SEEK_END);
    memset(buf, 0, sizeof(buf));
    while (size <= num) {
        write(fd, buf, sizeof(buf));
        size += sizeof(buf);
    }
    lseek(fd, num, SEEK_SET);
    read(fd, &flag, 1);
    if ((flag == 0 && val != 0)) {
        lseek(fd, num, SEEK_SET);
        write(fd, &val, 1);
    }
    flock(fd, LOCK_UN);
    close(fd);
    return flag;
}

int vote_check(bits)
int bits;
{
    int i, count;

    for (i = count = 0; i < 32; i++) {
        if ((bits >> i) & 1)
            count++;
    }
    return count;
}
struct _setperm_select {
    unsigned int pbits;
    unsigned int basic;
    unsigned int oldbits;
};
int vote_select(struct _select_def *conf)
{
    int count;
    struct _setperm_select *arg = (struct _setperm_select *) conf->arg;

    if (conf->pos == conf->item_count)
        return SHOW_QUIT;
    arg->pbits ^= (1 << (conf->pos - 1));
    count = vote_check(arg->pbits);
    if (count > currvote.maxtkt) {
        if (currvote.maxtkt == 1) {
            arg->pbits = (1 << (conf->pos - 1));
            return SHOW_REFRESH;
        } else
            arg->pbits ^= (1 << (conf->pos - 1));
        return SHOW_CONTINUE;
    }
    move(2, 0);
    clrtoeol();
    prints("ÄãÒÑ¾­Í¶ÁË %d Æ±", count);
    return SHOW_REFRESHSELECT;
}
int showvoteitems(struct _select_def *conf, int i)
{
    char buf[STRLEN];
    struct _setperm_select *arg = (struct _setperm_select *) conf->arg;

    i = i - 1;
    if (i == conf->item_count - 1) {
        prints("--ÍË³ö--");
    } else {
        sprintf(buf, "%c.%2.2s%-36.36s", 'A' + i,
                ((arg->pbits >> i) & 1 ? "¡ò" : "  "), currvote.items[i]);
        prints("%s", buf);
    }
    return SHOW_CONTINUE;
}

void show_voteing_title()
{
    time_t closedate;
    char buf[STRLEN];

    if (currvote.type != VOTE_VALUE && currvote.type != VOTE_ASKING)
        sprintf(buf, "¿ÉÍ¶Æ±Êı: %d Æ±", currvote.maxtkt);
    else
        buf[0] = '\0';
    closedate = currvote.opendate + currvote.maxdays * 86400;
    prints("Í¶Æ±½«½áÊøÓÚ: %24s  %s  %s\n",
           ctime(&closedate), buf, (voted_flag) ? "(ĞŞ¸ÄÇ°´ÎÍ¶Æ±)" : "");
    prints("Í¶Æ±Ö÷ÌâÊÇ: [1m%-50s[mÀàĞÍ: [1m%s[m \n", currvote.title,
           vote_type[currvote.type - 1]);
}

int getsug(uv)
struct ballot *uv;
{
    int i, line;

    move(0, 0);
    clrtobot();
    if (currvote.type == VOTE_ASKING) {
        show_voteing_title();
        line = 3;
        prints("ÇëÌîÈëÄúµÄ×÷´ğ(ÈıĞĞ):\n");
    } else {
        line = 1;
        prints("ÇëÌîÈëÄú±¦¹óµÄÒâ¼û(ÈıĞĞ):\n");
    }
    move(line, 0);
    for (i = 0; i < 3; i++) {
        prints(": %s\n", uv->msg[i]);
    }
    for (i = 0; i < 3; i++) {
        getdata(line + i, 0, ": ", uv->msg[i], STRLEN - 2, DOECHO, NULL,
                false);
        if (uv->msg[i][0] == '\0')
            break;
    }
    return i;
}

int multivote(uv)
struct ballot *uv;
{
    unsigned int i;

    i = uv->voted;
    move(0, 0);
    show_voteing_title();
    uv->voted =
        setperms(uv->voted, 0, "Ñ¡Æ±", currvote.totalitems, showvoteitems,
                 vote_select);
    if (uv->voted == i)
        return -1;
    return 1;
}

int valuevote(uv)
struct ballot *uv;
{
    unsigned int chs;
    char buf[10];

    chs = uv->voted;
    move(0, 0);
    show_voteing_title();
    prints("´Ë´Î×÷´ğµÄÖµ²»ÄÜ³¬¹ı [1m%d[m", currvote.maxtkt);
    if (uv->voted != 0)
        sprintf(buf, "%d", uv->voted);
    else
        memset(buf, 0, sizeof(buf));
    do {
        getdata(3, 0, "ÇëÊäÈëÒ»¸öÖµ? [0]: ", buf, 5, DOECHO, NULL, false);
        uv->voted = abs(atoi(buf));
    } while ((int) uv->voted > currvote.maxtkt && buf[0] != '\n'
             && buf[0] != '\0');
    if (buf[0] == '\n' || buf[0] == '\0' || uv->voted == chs)
        return -1;
    return 1;
}
int user_vote(int num)
{
    char fname[STRLEN], bname[STRLEN];
    char buf[STRLEN];
    struct ballot uservote, tmpbal;
    struct votelimit userlimit;
    int votevalue;
    int aborted = false, pos;

    if (!haspostperm(currentuser,currboard->filename))
	    return -1;
    move(t_lines - 2, 0);
    get_record(controlfile, &currvote, sizeof(struct votebal), num);
    sprintf(fname, "vote/%s/flag.%lu", currboard->filename, currvote.opendate);
    if ((pos =
         search_record(fname, &uservote, sizeof(uservote),
                       (RECORD_FUNC_ARG) cmpvuid,
                       currentuser->userid)) <= 0) {
        (void) memset(&uservote, 0, sizeof(uservote));
        voted_flag = false;
    } else {
        voted_flag = true;
    }
    strcpy(uservote.uid, currentuser->userid);
    sprintf(bname, "desc.%lu", currvote.opendate);
    setvfile(buf, currboard->filename, bname);
    ansimore(buf, true);
    move(0, 0);
    /*Haohmaru.99.11.17.¸ù¾İ°æÖ÷ÉèµÄÏŞÖÆÌõ¼şÅĞ¶ÏÊÇ·ñÈÃ¸ÃÊ¹ÓÃÕßÍ¶Æ± */
    clear();
    userlimit.numlogins = 0;
    userlimit.numposts = 0;
    userlimit.stay = 0;
    userlimit.day = 0;
    sprintf(limitfile, "vote/%s/limit.%lu", currboard->filename, currvote.opendate);
    get_record(limitfile, &userlimit, sizeof(struct votelimit), 1);
    if ((currvote.type <= 0) || (currvote.type > 5))
        currvote.type = 1;
    if ((currentuser->numposts < userlimit.numposts
         || currentuser->numlogins < userlimit.numlogins
         || currentuser->stay < userlimit.stay * 60 * 60
         || (time(NULL) - currentuser->firstlogin) <
         userlimit.day * 24 * 60 * 60)) {
        prints
            ("¶Ô²»Æğ,Äã²»Âú×ã°æÖ÷¹æ¶¨µÄ´Ë´ÎÍ¶Æ±ËùĞèÌõ¼ş,ÎŞ·¨²Î¼ÓÍ¶Æ±,Ğ»Ğ»²ÎÓë,ÏÂ´ÎÔÙ¼û! :)");
        pressanykey();
        return -1;
    }
    clrtobot();
    switch (currvote.type) {
    case VOTE_SINGLE:
    case VOTE_MULTI:
    case VOTE_YN:
        votevalue = multivote(&uservote);
        if (votevalue == -1)
            aborted = true;
        break;
    case VOTE_VALUE:
        votevalue = valuevote(&uservote);
        if (votevalue == -1)
            aborted = true;
        break;
    case VOTE_ASKING:
        uservote.voted = 0;
        aborted = !getsug(&uservote);
        break;
    }
    clear();
    if (aborted == true) {
        prints("±£Áô ¡¾%s¡¿Ô­À´µÄµÄÍ¶Æ±¡£\n", currvote.title);
    } else {
        if (currvote.type != VOTE_ASKING)
            getsug(&uservote);
        pos =
            search_record(fname, &tmpbal, sizeof(tmpbal),
                          (RECORD_FUNC_ARG) cmpvuid, currentuser->userid);
        if (pos) {
            substitute_record(fname, &uservote, sizeof(uservote), pos);
        } else if (append_record(fname, &uservote, sizeof(uservote)) == -1) {
            move(2, 0);
            clrtoeol();
            prints("Í¶Æ±Ê§°Ü! ÇëÍ¨ÖªÕ¾³¤²Î¼ÓÄÇÒ»¸öÑ¡ÏîÍ¶Æ±\n");
            pressreturn();
        }
        prints("\nÒÑ¾­°ïÄã(¡õ)Í¶ÈëÆ±ÏäÖĞ...\n");
    }
    pressanykey();
    return 0;
}

void voteexp()
{
    clrtoeol();
    prints("[44m±àºÅ ¿ªÆôÍ¶Æ±ÏäÕß ¿ªÆôÈÕ %-40sÀà±ğ ÌìÊı ÈËÊı[m\n",
           "Í¶Æ±Ö÷Ìâ", "");
}
int printvote(struct votebal *ent, int idx, int *i)
{
    struct ballot uservote;
    char buf[STRLEN + 80], *date;
    char flagname[STRLEN];
    int num_voted;

    if (ent == NULL) {
        move(2, 0);
        voteexp();
        *i = 0;
        return 0;
    }
    (*i)++;
    if (*i > page + 19 || *i > range)
        return QUIT;
    else if (*i <= page)
        return 0;
    sprintf(buf, "flag.%lu", ent->opendate);
    setvfile(flagname, currboard->filename, buf);
    if (search_record
        (flagname, &uservote, sizeof(uservote), (RECORD_FUNC_ARG) cmpvuid,
         currentuser->userid) <= 0) {
        voted_flag = false;
    } else
        voted_flag = true;
    num_voted = get_num_records(flagname, sizeof(struct ballot));
    date = ctime(&ent->opendate) + 4;
    if ((ent->type <= 0) || (ent->type > 5))
        ent->type = 1;
    sprintf(buf, " %s%3d %-12.12s %-6.6s %-40.40s%-4.4s %3d %5d[m\n",
            (voted_flag == false) ? "[1m" : "", *i, ent->userid, date,
            ent->title, vote_type[ent->type - 1], ent->maxdays, num_voted);
/*
    sprintf(buf," %s%3d %-12.12s %-6.6s %-40.40s%-4.4s %3d  %4d[m\n",(voted_flag==false)?"[1m":"",*i,ent->userid,
            date,ent->title,vote_type[ent->type-1],ent->maxdays,num_voted);
*/
    prints("%s", buf);
    return 0;
}
static int dele_vote(int num)
{
    char buf[STRLEN];

    sprintf(buf, "vote/%s/flag.%lu", currboard->filename, currvote.opendate);
    unlink(buf);
    sprintf(buf, "vote/%s/desc.%lu", currboard->filename, currvote.opendate);
    unlink(buf);
    sprintf(buf, "vote/%s/limit.%lu", currboard->filename, currvote.opendate);    /*Haohmaru.99.11.18 */
    unlink(buf);
    if (delete_record(controlfile, sizeof(currvote), num, NULL, NULL) ==
        -1) {
        prints("·¢Éú´íÎó£¬ÇëÍ¨ÖªÕ¾³¤....");
        pressanykey();
    }
    range--;
    if (get_num_records(controlfile, sizeof(currvote)) == 0) {
        setvoteflag(currboard->filename, 0);
    }
    return 0;
}

int vote_results(bname)
char *bname;
{
    char buf[STRLEN];

    setvfile(buf, bname, "results");
    if (ansimore(buf, true) == -1) {
        move(3, 0);
        prints("Ä¿Ç°Ã»ÓĞÈÎºÎÍ¶Æ±µÄ½á¹û¡£\n");
        clrtobot();
        pressreturn();
    } else
        clear();
    return FULLUPDATE;
}

int b_vote_maintain()
{
    return vote_maintain(currboard->filename);
}

int vote_title()
{
    docmdtitle("[Í¶Æ±ÏäÁĞ±í]",
                "[mÀë¿ª[[1;32m¡û[m,[1;32me[m] ÇóÖú[[1;32mh[m] ½øĞĞÍ¶Æ±[[1;32m¡ú[m,[1;32mr <cr>[m] ÉÏ,ÏÂÑ¡Ôñ[[1;32m¡ü[m,[1;32m¡ı[m] [1m¸ßÁÁ¶È[m±íÊ¾ÉĞÎ´Í¶Æ±");
    move(2, 0);
    prints("[0;1;37;44m±àºÅ ¿ªÆôÍ¶Æ±ÏäÕß ¿ªÆôÈÕ Í¶Æ±Ö÷Ìâ                                Àà±ğ ÌìÊı ÈËÊı");
    clrtoeol();
    update_endline();
    return 0;
}

int vote_key(ch, allnum, pagenum)
int ch;
int allnum, pagenum;
{
    int deal = 0, ans;
    char buf[STRLEN];

    switch (ch) {
    case 'v':
    case 'V':
    case '\n':
    case '\r':
    case 'r':
    case KEY_RIGHT:
        user_vote(allnum + 1);
        deal = 1;
        break;
    case 'R':
        vote_results(currboard->filename);
        deal = 1;
        break;
    case 'H':
    case 'h':
        show_help("help/votehelp");
        deal = 1;
        break;
    case 'A':
    case 'a':
        if (!chk_currBM(currBM, currentuser))
            return true;
        vote_maintain(currboard->filename);
        deal = 1;
        break;
    case 'O':
    case 'o':
        if (!chk_currBM(currBM, currentuser))
            return true;
        clear();
        deal = 1;
        get_record(controlfile, &currvote, sizeof(struct votebal),
                   allnum + 1);
        prints("[31m¾¯¸æ!![m\n");
        prints("Í¶Æ±Ïä±êÌâ£º[1m%s[m\n", currvote.title);
        ans = askyn("ÄãÈ·¶¨ÒªÌáÔç½áÊøÕâ¸öÍ¶Æ±Âğ", 0);
        if (ans != 1) {
            move(2, 0);
            prints("È¡ÏûÉ¾³ıĞĞ¶¯\n");
            pressreturn();
            clear();
            break;
        }
        mk_result(allnum + 1);
        sprintf(buf, "ÌáÔç½áÊøÍ¶Æ± %s", currvote.title);
        securityreport(buf, NULL, NULL);
        bbslog("user","%s",buf);
        break;
    case '@':
        if (!HAS_PERM(currentuser, PERM_SYSOP))
            return true;
        clear();
        deal = 1;
        get_record(controlfile, &currvote, sizeof(struct votebal),
                   allnum + 1);
        prints("¼ì²éÍ¶Æ±£º\x1b[1m%s\x1b[m\n", currvote.title);
        check_result(allnum + 1);
        break;
    case 'D':
    case 'd':
        if (!chk_currBM(currBM, currentuser)) {
            return 1;
        }
        deal = 1;
        get_record(controlfile, &currvote, sizeof(struct votebal),
                   allnum + 1);
        clear();
        prints("[31m¾¯¸æ!![m\n");
        prints("Í¶Æ±Ïä±êÌâ£º[1m%s[m\n", currvote.title);
        ans = askyn("ÄãÈ·¶¨ÒªÇ¿ÖÆ¹Ø±ÕÕâ¸öÍ¶Æ±Âğ", 0);
        if (ans != 1) {
            move(2, 0);
            prints("È¡ÏûÉ¾³ıĞĞ¶¯\n");
            pressreturn();
            clear();
            break;
        }
        sprintf(buf, "Ç¿ÖÆ¹Ø±ÕÍ¶Æ± %s", currvote.title);
        securityreport(buf, NULL, NULL);
        bbslog("user","%s",buf);
        dele_vote(allnum + 1);
        break;
    default:
        return 0;
    }
    if (deal) {
        Show_Votes();
        vote_title();
    }
    return 1;
}
static int Show_Votes()
{
    int i;

    move(3, 0);
    clrtobot();
    i = 0;
    setcontrolfile();
    if (apply_record
        (controlfile, (APPLY_FUNC_ARG) printvote, sizeof(struct votebal),
         &i, 0, false) == -1) {
        prints("´íÎó£¬Ã»ÓĞÍ¶Æ±Ïä¿ªÆô....");
        pressreturn();
        return 0;
    }
    clrtobot();
    return 0;
}

int b_vote()
{
    int num_of_vote;
    int voting;

    if (!HAS_PERM(currentuser, PERM_LOGINOK))
        return 0;               /* Leeward 98.05.15 */
    setcontrolfile();
    num_of_vote = get_num_records(controlfile, sizeof(struct votebal));
    if (num_of_vote == 0) {
        move(3, 0);
        clrtobot();
        prints("±§Ç¸, Ä¿Ç°²¢Ã»ÓĞÈÎºÎÍ¶Æ±¾ÙĞĞ¡£\n");
        pressreturn();
        setvoteflag(currboard->filename, 0);
        return FULLUPDATE;
    }
    setlistrange(num_of_vote);
    clear();
    voting = choose(false, 0, vote_title, vote_key, Show_Votes, user_vote);
    clear();
    return /*user_vote( currboard->filename ) */ FULLUPDATE;
}

int b_results()
{
    return vote_results(currboard->filename);
}

int m_vote()
{
    modify_user_mode(ADMIN);
    vote_maintain(DEFAULTBOARD);
    return 0;
}

int x_vote()
{
    char buf[STRLEN];
    struct boardheader* bh;
    int bid;

    modify_user_mode(XMENU);
    bh=currboard;
    bid=currboardent;
    b_vote();
    currboard=bh;
    currboardent=bid;
    return 0;
}

int x_results()
{
    modify_user_mode(XMENU);
    return vote_results(DEFAULTBOARD);
}
