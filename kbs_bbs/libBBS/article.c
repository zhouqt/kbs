#include "bbs.h"
#include <utime.h>


void cancelpost(char *board, char *userid, struct fileheader *fh, int owned, int autoappend);
int outgo_post(struct fileheader *fh, char *board, char *title)
{
    FILE *foo;

    if ((foo = fopen("innd/out.bntp", "a")) != NULL) {
        fprintf(foo, "%s\t%s\t%s\t%s\t%s\n", board, fh->filename, currentuser->userid, currentuser->username, title);
        fclose(foo);
        return 0;
    }
    return -1;
}

extern char alphabet[];

int get_postfilename(char *filename, char *direct, int use_subdir)
{
    static const char post_sufix[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int fp;
    time_t now;
    int i;
    char fname[255];
    int pid = getpid();
	int rn;
	int len;

    /*
     * ×Ô¶¯Éú³É POST ÎÄ¼þÃû 
     */
    now = time(NULL);
	len = strlen(alphabet);
    for (i = 0; i < 10; i++)
	{
		if (use_subdir)
		{
			rn = 0 + (int) (len * 1.0 * rand() / (RAND_MAX + 1.0));
			sprintf(filename, "%c/M.%lu.%c%c", alphabet[rn], now, post_sufix[(pid + i) % 62], post_sufix[(pid * i) % 62]);
		}
		else
			sprintf(filename, "M.%lu.%c%c", now, post_sufix[(pid + i) % 62], post_sufix[(pid * i) % 62]);
        sprintf(fname, "%s/%s", direct, filename);
        if ((fp = open(fname, O_CREAT | O_EXCL | O_WRONLY, 0644)) != -1) {
            break;
        };
    }
    if (fp == -1)
        return -1;
    close(fp);
    return 0;
}

int isowner(struct userec *user, struct fileheader *fileinfo)
{
    time_t posttime;

    if (strcmp(fileinfo->owner, user->userid))
        return 0;
    posttime = get_posttime(fileinfo);
    if (posttime < user->firstlogin)
        return 0;
    return 1;
}

int cmpname(fhdr, name)         /* Haohmaru.99.3.30.±È½Ï Ä³ÎÄ¼þÃûÊÇ·ñºÍ µ±Ç°ÎÄ¼þ ÏàÍ¬ */
struct fileheader *fhdr;
char name[STRLEN];
{
    if (!strncmp(fhdr->filename, name, FILENAME_LEN))
        return 1;
    return 0;
}

int do_del_post(struct userec *user, int ent, struct fileheader *fileinfo, char *direct, char *board, int digestmode, int decpost)
{
    char buf[512];
    char *t;
    int owned, fail;

    strcpy(buf, direct);
    if ((t = strrchr(buf, '/')) != NULL)
        *t = '\0';
/* .post.X not use???! KCN
postbbslog("user","%s",fileinfo->title, -1, currboard); added by alex, 96.9.12 */
/*    if( keep <= 0 ) {*/
    if (fileinfo->id == fileinfo->groupid)
        setboardorigin(board, 1);
    setboardtitle(board, 1);
    //added by bad 2002.8.12
    fail = delete_record(direct, sizeof(struct fileheader), ent, (RECORD_FUNC_ARG) cmpname, fileinfo->filename);
/*
    } else {
        fail = update_file(direct,sizeof(struct fileheader),ent,cmpfilename,
                           cpyfilename);
    }
    */
    owned = isowner(user, fileinfo);
    if (!fail) {
        cancelpost(board, user->userid, fileinfo, owned, 1);
        updatelastpost(board);
/*
        sprintf(buf,"%s/%s",buf,fileinfo->filename) ;
        if(keep >0)  if ( (fn = fopen( buf, "w" )) != NULL ) {
            fprintf( fn, "\n\n\t\t±¾ÎÄÕÂÒÑ±» %s É¾³ý.\n",
                     currentuser->userid );
            fclose( fn );
        }
*/
        if (fileinfo->accessed[0] & FILE_MARKED)
            setboardmark(board, 1);
        if ((true != digestmode)        /* ²»¿ÉÒÔÓÃ ¡°NA ==¡± ÅÐ¶Ï£ºdigestmode ÈýÖµ */
            &&!((fileinfo->accessed[0] & FILE_MARKED)
                && (fileinfo->accessed[1] & FILE_READ)
                && (fileinfo->accessed[0] & FILE_FORWARDED))) { /* Leeward 98.06.17 ÔÚÎÄÕªÇøÉ¾ÎÄ²»¼õÎÄÕÂÊýÄ¿ */
            if (owned) {
                if ((int) user->numposts > 0 && !junkboard(board)) {
                    user->numposts--;   /*×Ô¼ºÉ¾³ýµÄÎÄÕÂ£¬¼õÉÙpostÊý */
                }
            } else if (!strstr(fileinfo->owner, ".") && BMDEL_DECREASE && decpost /*°æÖ÷É¾³ý,¼õÉÙPOSTÊý */ ) {
                struct userec *lookupuser;
                int id = getuser(fileinfo->owner, &lookupuser);

                if (id && (int) lookupuser->numposts > 0 && !junkboard(board) && strcmp(board, "sysmail")) {    /* SYSOP MAIL°æÉ¾ÎÄ²»¼õÎÄÕÂ Bigman: 2000.8.12 *//* Leeward 98.06.21 adds above later 2 conditions */
                    lookupuser->numposts--;
                }
            }
        }
        utime(fileinfo->filename, 0);
        if (user != NULL)
            bmlog(user->userid, board, 8, 1);
        newbbslog(LOG_USER, "Del '%s' on '%s'", fileinfo->title, board);     /* bbslog */
        return 0;
    }
    return -1;
}

/* by ylsdd 
   unlink action is taked within cancelpost if in mail mode,
   otherwise this item is added to the file '.DELETED' under
   the board's directory, the filename is not changed. 
   Unlike the fb code which moves the file to the deleted
   board.
*/
void cancelpost(board, userid, fh, owned, autoappend)
char *board, *userid;
struct fileheader *fh;
int owned;
int autoappend;
{
    struct fileheader postfile;
    char oldpath[sizeof(genbuf)];
    struct fileheader *ph;
    time_t now;

#ifdef BBSMAIN
    if (uinfo.mode == RMAIL) {
        sprintf(oldpath, "mail/%c/%s/%s", toupper(currentuser->userid[0]), currentuser->userid, fh->filename);
        unlink(oldpath);
        return;
    }
#endif
    if (autoappend)
        ph = &postfile;
    else
        ph = fh;
/*
    sprintf(oldpath, "/board/%s/%s.html", board, fh->filename);
    ca_expire_file(oldpath);*/

    if (autoappend) {
        bzero(&postfile, sizeof(postfile));
        strcpy(postfile.filename, fh->filename);
        strncpy(postfile.owner, fh->owner, OWNER_LEN-1);
        postfile.owner[OWNER_LEN-1] = 0;
        postfile.id = fh->id;
        postfile.groupid = fh->groupid;
        postfile.reid = fh->reid;
		set_posttime2(&postfile, fh);
    };
    now = time(NULL);
    sprintf(oldpath, "%-32.32s - %s", fh->title, userid);
    strncpy(ph->title, oldpath, STRLEN);
    ph->title[STRLEN - 1] = 0;
    ph->accessed[11] = now / (3600 * 24) % 100; /*localtime(&now)->tm_mday; */
    if (autoappend) {
        setbdir((owned) ? 5 : 4, oldpath, board);
        append_record(oldpath, &postfile, sizeof(postfile));
    }
    if ((fh->innflag[1] == 'S')
        && (fh->innflag[0] == 'S')
        && (get_posttime(fh) > now - 14 * 86400)) {
        FILE *fp;
        char buf[256];
        char from[STRLEN];
        int len;
        char *ptr;

        setbfile(buf, board, fh->filename);
        if ((fp = fopen(buf, "r")) == NULL)
            return;
        while (fgets(buf, sizeof(buf), fp) != NULL) {
            /*
             * Ê×ÏÈÂËµô»»ÐÐ·û 
             */
            len = strlen(buf) - 1;
            buf[len] = '\0';
            if (len < 8)
                break;
            if (strncmp("·¢ÐÅÈË: ", buf, 8) == 0) {
                if ((ptr = strrchr(buf, ')')) == NULL)
                    break;
                *ptr = '\0';
                if ((ptr = strrchr(buf, '(')) == NULL)
                    break;
                strncpy(from, ptr + 1, sizeof(from) - 1);
                from[sizeof(from) - 1] = '\0';
                break;
            }
        }
        fclose(fp);
        sprintf(buf, "%s\t%s\t%s\t%s\t%s\n", board, fh->filename, fh->owner, from, fh->title);
        if ((fp = fopen("innd/cancel.bntp", "a")) != NULL) {
            fputs(buf, fp);
            fclose(fp);
        }
    }
}


void add_loginfo(char *filepath, struct userec *user, char *currboard, int Anony)
{                               /* POST ×îºóÒ»ÐÐ Ìí¼Ó */
    FILE *fp;
    int color, noidboard;
    char fname[STRLEN];

    noidboard = (anonymousboard(currboard) && Anony);   /* etc/anonymousÎÄ¼þÖÐ ÊÇÄäÃû°æ°æÃû */
    color = (user->numlogins % 7) + 31; /* ÑÕÉ«Ëæ»ú±ä»¯ */
    sethomefile(fname, user->userid, "signatures");
    fp = fopen(filepath, "a");
    if (!dashf(fname) ||        /* ÅÐ¶ÏÊÇ·ñÒÑ¾­ ´æÔÚ Ç©Ãûµµ */
        user->signature == 0 || noidboard) {
        fputs("\n--\n", fp);
    } else {                    /*Bigman 2000.8.10ÐÞ¸Ä,¼õÉÙ´úÂë */
        fprintf(fp, "\n");
    }
    /*
     * ÓÉBigmanÔö¼Ó:2000.8.10 Announce°æÄäÃû·¢ÎÄÎÊÌâ 
     */
    if (!strcmp(currboard, "Announce"))
        fprintf(fp, "[m[1;%2dm¡ù À´Ô´:¡¤%s %s¡¤[FROM: %s][m\n", color, BBS_FULL_NAME, email_domain(), NAME_BBS_CHINESE " BBSÕ¾");
    else
        fprintf(fp, "\n[m[1;%2dm¡ù À´Ô´:¡¤%s %s¡¤[FROM: %s][m\n", color, BBS_FULL_NAME, email_domain(), (noidboard) ? NAME_ANONYMOUS_FROM : fromhost);
    fclose(fp);
    return;
}

void addsignature(FILE * fp, struct userec *user, int sig)
{
    FILE *sigfile;
    int i, valid_ln = 0;
    char tmpsig[MAXSIGLINES][256];
    char inbuf[256];
    char fname[STRLEN];

    if (sig == 0)
        return;
    sethomefile(fname, user->userid, "signatures");
    if ((sigfile = fopen(fname, "r")) == NULL) {
        return;
    }
    fputs("\n--\n", fp);
    for (i = 1; i <= (sig - 1) * MAXSIGLINES && sig != 1; i++) {
        if (!fgets(inbuf, sizeof(inbuf), sigfile)) {
            fclose(sigfile);
            return;
        }
    }
    for (i = 1; i <= MAXSIGLINES; i++) {
        if (fgets(inbuf, sizeof(inbuf), sigfile)) {
            if (inbuf[0] != '\n')
                valid_ln = i;
            strcpy(tmpsig[i - 1], inbuf);
        } else
            break;
    }
    fclose(sigfile);
    for (i = 1; i <= valid_ln; i++)
        fputs(tmpsig[i - 1], fp);
}

int write_posts(char *id, char *board, char *title)
{
    char *ptr;
    time_t now;
    struct posttop postlog, pl;

    if (junkboard(board) || normal_board(board) != 1)
        return 0;
    now = time(0);
    strcpy(postlog.author, id);
    strcpy(postlog.board, board);
    ptr = title;
    if (!strncmp(ptr, "Re: ", 4))
        ptr += 4;
    strncpy(postlog.title, ptr, 65);
    postlog.date = now;
    postlog.number = 1;

    {                           /* added by Leeward 98.04.25 
                                 * TODO: Õâ¸öµØ·½ÓÐµã²»Í×,Ã¿´Î·¢ÎÄÒª±éÀúÒ»´Î,±£´æµ½.XpostÖÐ,
                                 * ÓÃÀ´Íê³ÉÊ®´ó·¢ÎÄÍ³¼ÆÕë¶ÔID¶ø²»ÊÇÎÄÕÂ.²»ºÃ
                                 * KCN */
        int log = 1;
        FILE *fp = fopen(".Xpost", "r");

        if (fp) {
            while (!feof(fp)) {
                fread(&pl, sizeof(pl), 1, fp);
                if (feof(fp))
                    break;

                if (!strcmp(pl.title, postlog.title)
                    && !strcmp(pl.author, postlog.author)
                    && !strcmp(pl.board, postlog.board)) {
                    log = 0;
                    break;
                }
            }
            fclose(fp);
        }

        if (log) {
            append_record(".Xpost", &postlog, sizeof(postlog));
            append_record(".post", &postlog, sizeof(postlog));
        }
    }

/*    append_record(".post.X", &postlog, sizeof(postlog));
*/
    return 0;
}

void write_header(FILE * fp, struct userec *user, int in_mail, char *board, char *title, int Anony, int mode)
{
    int noname;
    char uid[20];
    char uname[40];
    time_t now;

    now = time(0);
    strncpy(uid, user->userid, 20);
    uid[19] = '\0';
    if (in_mail)
#if defined(MAIL_REALNAMES)
        strncpy(uname, user->realname, NAMELEN);
#else
        strncpy(uname, user->username, NAMELEN);
#endif
    else
#if defined(POSTS_REALNAMES)
        strncpy(uname, user->realname, NAMELEN);
#else
        strncpy(uname, user->username, NAMELEN);
#endif
    /*
     * uid[39] = '\0' ; SO FUNNY:-) ¶¨ÒåµÄ 20 ÕâÀïÈ´ÓÃ 39 !
     * Leeward: 1997.12.11 
     */
    uname[39] = 0;              /* ÆäÊµÊÇÐ´´í±äÁ¿ÃûÁË! ºÙºÙ */
    noname = anonymousboard(board);
    if (in_mail)
        fprintf(fp, "¼ÄÐÅÈË: %s (%s)\n", uid, uname);
    else {
        if (mode == 0 && !(noname && Anony)) {
            write_posts(user->userid, board, title);
        }

        if (!strcmp(board, "Announce"))
            /*
             * added By Bigman 
             */
            fprintf(fp, "·¢ÐÅÈË: %s (%s), ÐÅÇø: %s       \n", "SYSOP", NAME_SYSOP, board);
        else
            fprintf(fp, "·¢ÐÅÈË: %s (%s), ÐÅÇø: %s       \n", (noname && Anony) ? board : uid, (noname && Anony) ? NAME_ANONYMOUS : uname, board);
    }

    fprintf(fp, "±ê  Ìâ: %s\n", title);
    /*
     * Ôö¼Ó×ªÐÅ±ê¼Ç czz 020819 
     */
    if (mode != 2)
        fprintf(fp, "·¢ÐÅÕ¾: %s (%24.24s), Õ¾ÄÚÐÅ¼þ\n", BBS_FULL_NAME, ctime(&now));
    else
        fprintf(fp, "·¢ÐÅÕ¾: %s (%24.24s), ×ªÐÅ\n", BBS_FULL_NAME, ctime(&now));
//    fprintf(fp, "·¢ÐÅÕ¾: %s (%24.24s)\n", BBS_FULL_NAME, ctime(&now));
    if (in_mail)
        fprintf(fp, "À´  Ô´: %s \n", fromhost);
    fprintf(fp, "\n");

}

void getcross(char *filepath, char *quote_file, struct userec *user, int in_mail, char *board, char *title, int Anony, int mode, char *sourceboard)
{                               /* °Ñquote_file¸´ÖÆµ½filepath (×ªÌù»ò×Ô¶¯·¢ÐÅ) */
    FILE *inf, *of;
    char buf[256];
    char owner[256];
    int count;
    time_t now;

    now = time(0);
    inf = fopen(quote_file, "r");
    of = fopen(filepath, "w");
    if (inf == NULL || of == NULL) {
        /*---	---*/
        if (NULL != inf)
            fclose(inf);
        if (NULL != of)
            fclose(of);
        /*---	---*/
#ifdef BBSMAIN
        bbslog("user","%s","Cross Post error");
#endif
        return;
    }
    if (mode == 0 /*×ªÌù */ ) {
        int normal_file;
        int header_count;

        normal_file = 1;

        write_header(of, user, in_mail, sourceboard, title, Anony, 1 /*²»Ð´Èë .posts */ );
        if (fgets(buf, 256, inf) != NULL) {
            for (count = 8; buf[count] != ' ' && count < 256; count++)
                owner[count - 8] = buf[count];
        }
        owner[count - 8] = '\0';
        if (in_mail == true)
            fprintf(of, "[1;37m¡¾ ÒÔÏÂÎÄ×Ö×ªÔØ×Ô [32m%s [37mµÄÐÅÏä ¡¿\n", user->userid);
        else
            fprintf(of, "¡¾ ÒÔÏÂÎÄ×Ö×ªÔØ×Ô %s ÌÖÂÛÇø ¡¿\n", board);
        if (id_invalid(owner))
            normal_file = 0;
        if (normal_file) {
            for (header_count = 0; header_count < 3; header_count++) {
                if (fgets(buf, 256, inf) == NULL)
                    break;      /*Clear Post header */
            }
            if ((header_count != 2) || (buf[0] != '\n'))
                normal_file = 0;
        }
        if (normal_file)
            fprintf(of, "¡¾ Ô­ÎÄÓÉ %s Ëù·¢±í ¡¿\n", owner);
        else
            fseek(inf, 0, SEEK_SET);

    } else if (mode == 1 /*×Ô¶¯·¢ÐÅ */ ) {
        fprintf(of, "·¢ÐÅÈË: deliver (×Ô¶¯·¢ÐÅÏµÍ³), ÐÅÇø: %s\n", board);
        fprintf(of, "±ê  Ìâ: %s\n", title);
        fprintf(of, "·¢ÐÅÕ¾: %s×Ô¶¯·¢ÐÅÏµÍ³ (%24.24s)\n\n", BBS_FULL_NAME, ctime(&now));
        fprintf(of, "¡¾´ËÆªÎÄÕÂÊÇÓÉ×Ô¶¯·¢ÐÅÏµÍ³ËùÕÅÌù¡¿\n\n");
    } else if (mode == 2) {
        write_header(of, user, in_mail, sourceboard, title, Anony, 0 /*Ð´Èë .posts */ );
    }
    while (fgets(buf, 256, inf) != NULL) {
        if ((strstr(buf, "¡¾ ÒÔÏÂÎÄ×Ö×ªÔØ×Ô ") && strstr(buf, "ÌÖÂÛÇø ¡¿")) || (strstr(buf, "¡¾ Ô­ÎÄÓÉ") && strstr(buf, "Ëù·¢±í ¡¿")))
            continue;           /* ±ÜÃâÒýÓÃÖØ¸´ */
        else
            fprintf(of, "%s", buf);
    }
    fclose(inf);
    fclose(of);
    /*
     * don't know why 
     * *quote_file = '\0';
     */
}

/* Add by SmallPig */
int post_cross(struct userec *user, char *toboard, char *fromboard, char *title, char *filename, int Anony, int in_mail, char islocal, int mode)
{                               /* (×Ô¶¯Éú³ÉÎÄ¼þÃû) ×ªÌù»ò×Ô¶¯·¢ÐÅ */
    struct fileheader postfile;
    char filepath[STRLEN];
    char buf4[STRLEN], whopost[IDLEN], save_title[STRLEN];
    int aborted, local_article;

    if (!haspostperm(user, toboard) && !mode) {
#ifdef BBSMAIN
        move(1, 0);
        prints("ÄúÉÐÎÞÈ¨ÏÞÔÚ %s ·¢±íÎÄÕÂ.\n", toboard);
        prints("Èç¹ûÄúÉÐÎ´×¢²á£¬ÇëÔÚ¸öÈË¹¤¾ßÏäÄÚÏêÏ¸×¢²áÉí·Ý\n");
        prints("Î´Í¨¹ýÉí·Ý×¢²áÈÏÖ¤µÄÓÃ»§£¬Ã»ÓÐ·¢±íÎÄÕÂµÄÈ¨ÏÞ¡£\n");
        prints("Ð»Ð»ºÏ×÷£¡ :-) \n");
#endif
        return -1;
    }

    memset(&postfile, 0, sizeof(postfile));

    if (!mode) {
        if (!strstr(title, "(×ªÔØ)"))
            sprintf(buf4, "%s (×ªÔØ)", title);
        else
            strcpy(buf4, title);
    } else
        strcpy(buf4, title);
    strncpy(save_title, buf4, STRLEN);

    setbfile(filepath, toboard, "");

    if ((aborted = GET_POSTFILENAME(postfile.filename, filepath)) != 0) {
#ifdef BBSMAIN
        move(3, 0);
        clrtobot();
        prints("\n\nÎÞ·¨´´½¨ÎÄ¼þ:%d...\n", aborted);
        pressreturn();
        clear();
#endif
        return FULLUPDATE;
    }

    if (mode == 1)
        strcpy(whopost, "deliver");     /* mode==1Îª×Ô¶¯·¢ÐÅ */
    else
        strcpy(whopost, user->userid);

    strncpy(postfile.owner, whopost, OWNER_LEN);
    postfile.owner[OWNER_LEN-1]=1;
    setbfile(filepath, toboard, postfile.filename);

    local_article = 1; /* default is local article */
    if (islocal != 'l' && islocal != 'L')
	{
		if (is_outgo_board(toboard))
			local_article = 0;
	}

#ifdef BBSMAIN
    modify_user_mode(POSTING);
#endif
    getcross(filepath, filename, user, in_mail, fromboard, title, Anony, mode, toboard);        /*¸ù¾ÝfnameÍê³É ÎÄ¼þ¸´ÖÆ */

    /*
     * Changed by KCN,disable color title 
     */
    if (mode != 1) {
        int i;

        for (i = 0; (i < strlen(save_title)) && (i < STRLEN - 1); i++)
            if (save_title[i] == 0x1b)
                postfile.title[i] = ' ';
            else
                postfile.title[i] = save_title[i];
        postfile.title[i] = 0;
    } else
        strncpy(postfile.title, save_title, STRLEN);
    if (local_article == 1) {   /* local save */
        postfile.innflag[1] = 'L';
        postfile.innflag[0] = 'L';
    } else {
        postfile.innflag[1] = 'S';
        postfile.innflag[0] = 'S';
        outgo_post(&postfile, toboard, save_title);
    }
    /*
     * setbdir(digestmode, buf, currboard );Haohmaru.99.11.26.¸Ä³ÉÏÂÃæÒ»ÐÐ£¬ÒòÎª²»¹ÜÊÇ×ªÌù»¹ÊÇ×Ô¶¯·¢ÎÄ¶¼²»»áÓÃµ½ÎÄÕªÄ£Ê½ 
     */
    if (!strcmp(toboard, "syssecurity")
        && strstr(title, "ÐÞ¸Ä ")
        && strstr(title, " µÄÈ¨ÏÞ"))
        postfile.accessed[0] |= FILE_MARKED;    /* Leeward 98.03.29 */
    if (strstr(title, "·¢ÎÄÈ¨ÏÞ") && mode == 2) {
#ifndef NINE_BUILD
        postfile.accessed[0] |= FILE_MARKED;    /* Haohmaru 99.11.10 */
        postfile.accessed[1] |= FILE_READ;
#endif
        postfile.accessed[0] |= FILE_FORWARDED;
    }
    after_post(user, &postfile, toboard, NULL);
    return 1;
}


int post_file(struct userec *user, char *fromboard, char *filename, char *nboard, char *posttitle, int Anony, int mode)
/* ½«Ä³ÎÄ¼þ POST ÔÚÄ³°æ */
{
    if (getboardnum(nboard, NULL) <= 0) {       /* ËÑË÷ÒªPOSTµÄ°æ ,ÅÐ¶ÏÊÇ·ñ´æÔÚ¸Ã°æ */
        return -1;
    }
    post_cross(user, nboard, fromboard, posttitle, filename, Anony, false, 'l', mode);  /* post ÎÄ¼þ */
    return 0;
}

int after_post(struct userec *user, struct fileheader *fh, char *boardname, struct fileheader *re)
{
    char buf[256];
    int fd, err = 0, nowid = 0;
    char* p;

    if ((re == NULL) && (!strncmp(fh->title, "Re:", 3))) {
        strncpy(fh->title, fh->title + 4, STRLEN);
    }
    setbfile(buf, boardname, DOT_DIR);

    if ((fd = open(buf, O_WRONLY | O_CREAT, 0664)) == -1) {
#ifdef BBSMAIN
        perror(buf);
#endif
        err = 1;
    }
    /*¹ýÂË²ÊÉ«±êÌâ*/
    for (p=fh->title;*p;p++) if (*p=='\x1b') *p=' ';
	  
    if (!err) {
        flock(fd, LOCK_EX);
        nowid = get_nextid(boardname);
        fh->id = nowid;
        if (re == NULL) {
            fh->groupid = fh->id;
            fh->reid = fh->id;
        } else {
            fh->groupid = re->groupid;
            fh->reid = re->id;
        }
		set_posttime(fh);
        lseek(fd, 0, SEEK_END);
        if (safewrite(fd, fh, sizeof(fileheader)) == -1) {
            bbslog("user","%s","apprec write err!");
            err = 1;
        }
        flock(fd, LOCK_UN);
        close(fd);
    }
    if (err) {
        bbslog("3error", "Posting '%s' on '%s': append_record failed!", fh->title, boardname);
        setbfile(buf, boardname, fh->filename);
        unlink(buf);
#ifdef BBSMAIN
        pressreturn();
        clear();
#endif
        return 1;
    }
    updatelastpost(boardname);
    brc_add_read(fh->id);
    sprintf(buf, "posted '%s' on '%s'", fh->title, boardname);
    newbbslog(LOG_USER, "%s", buf);

    if (fh->id == fh->groupid)
        setboardorigin(boardname, 1);
    setboardtitle(boardname, 1);
    if (fh->accessed[0] & FILE_MARKED)
        setboardmark(boardname, 1);
    if (user != NULL)
        bmlog(user->userid, boardname, 2, 1);
    return 0;
}

int dele_digest(char *dname, char *direc)
{                               /* É¾³ýÎÄÕªÄÚÒ»ÆªPOST, dname=postÎÄ¼þÃû,direc=ÎÄÕªÄ¿Â¼Ãû */
    char digest_name[STRLEN];
    char new_dir[STRLEN];
    char buf[STRLEN];
    char *ptr;
    struct fileheader fh;
    int pos;

    strcpy(digest_name, dname);
    strcpy(new_dir, direc);

    digest_name[0] = 'G';
    ptr = strrchr(new_dir, '/') + 1;
    strcpy(ptr, DIGEST_DIR);
    pos = search_record(new_dir, &fh, sizeof(fh), (RECORD_FUNC_ARG) cmpname, digest_name);      /* ÎÄÕªÄ¿Â¼ÏÂ .DIRÖÐ ËÑË÷ ¸ÃPOST */
    if (pos <= 0) {
        return -1;
    }
    delete_record(new_dir, sizeof(struct fileheader), pos, (RECORD_FUNC_ARG) cmpname, digest_name);
    *ptr = '\0';
    sprintf(buf, "%s%s", new_dir, digest_name);
    unlink(buf);
    return 0;
}

int mmap_search_apply(int fd, struct fileheader *buf, DIR_APPLY_FUNC func)
{
    struct fileheader *data;
    size_t filesize;
    int total;
    int low, high;
    int ret;

    if (flock(fd, LOCK_EX) == -1)
        return 0;
    BBS_TRY {
        if (safe_mmapfile_handle(fd, O_RDWR, PROT_READ | PROT_WRITE, MAP_SHARED, (void **) &data, &filesize) == 0) {
            flock(fd, LOCK_UN);
            BBS_RETURN(0);
        }
        total = filesize / sizeof(struct fileheader);
        low = 0;
        high = total - 1;
        while (low <= high) {
            int mid, comp;

            mid = (high + low) / 2;
            comp = (buf->id) - ((data + mid)->id);
            if (comp == 0) {
                ret = (*func) (fd, data, mid + 1, total, data, true);
                end_mmapfile((void *) data, filesize, -1);
                flock(fd, LOCK_UN);
                BBS_RETURN(ret);
            } else if (comp < 0)
                high = mid - 1;
            else
                low = mid + 1;
        }
        ret = (*func) (fd, data, low + 1, total, buf, false);
    }
    BBS_CATCH {
    }
    BBS_END end_mmapfile((void *) data, filesize, -1);
    flock(fd, LOCK_UN);
    return ret;
}

int change_dir_post_flag(struct userec *currentuser, char *currboard, int ent, struct fileheader *fileinfo, int flag)
{
    /*---	---*/
    int newent = 0, ret = 1;
    char *ptr, buf[STRLEN];
    char ans[256];
    char genbuf[1024], direct[256];
    struct fileheader mkpost;
    struct flock ldata;
    int fd, size = sizeof(fileheader);

    setbdir(0, direct, currboard);
    strcpy(buf, direct);
    ptr = strrchr(buf, '/') + 1;
    ptr[0] = '\0';
    sprintf(&genbuf[512], "%s%s", buf, fileinfo->filename);
    if (!dashf(&genbuf[512]))
        ret = 0;                /* ½èÓÃÒ»ÏÂnewent :PP   */

    if (ret)
        if ((fd = open(direct, O_RDWR | O_CREAT, 0644)) == -1)
            ret = 0;
    if (ret) {
        ldata.l_type = F_RDLCK;
        ldata.l_whence = 0;
        ldata.l_len = size;
        ldata.l_start = size * (ent - 1);
        if (fcntl(fd, F_SETLKW, &ldata) == -1) {
            bbslog("user","%s","reclock error");
            close(fd);
                                /*---	period	2000-10-20	file should be closed	---*/
            ret = 0;
        }
    }
    if (ret) {
        if (lseek(fd, size * (ent - 1), SEEK_SET) == -1) {
            bbslog("user","%s","subrec seek err");
            /*---	period	2000-10-24	---*/
            ldata.l_type = F_UNLCK;
            fcntl(fd, F_SETLK, &ldata);
            close(fd);
            ret = 0;
        }
    }
    if (ret) {
        if (get_record_handle(fd, &mkpost, sizeof(mkpost), ent) == -1) {
            bbslog("user","%s","subrec read err");
            ret = 0;
        }
        if (ret)
            if (strcmp(mkpost.filename, fileinfo->filename))
                ret = 0;
        if (!ret) {
            newent = search_record_back(fd, sizeof(struct fileheader), ent, (RECORD_FUNC_ARG) cmpfileinfoname, fileinfo->filename, &mkpost, 1);
            ret = (newent > 0);
            if (ret)
                memcpy(fileinfo, &mkpost, sizeof(mkpost));
            else {
                ldata.l_type = F_UNLCK;
                fcntl(fd, F_SETLK, &ldata);
                close(fd);
            }
            ent = newent;
        }
    }
    if (!ret)
        return DIRCHANGED;
    switch (flag) {
    case FILE_MARK_FLAG:
        if (fileinfo->accessed[0] & FILE_MARKED)
            fileinfo->accessed[0] = (fileinfo->accessed[0] & ~FILE_MARKED);
        else
            fileinfo->accessed[0] = fileinfo->accessed[0] | FILE_MARKED;
        break;
    case FILE_NOREPLY_FLAG:
        if (fileinfo->accessed[1] & FILE_READ)
            fileinfo->accessed[1] &= ~FILE_READ;
        else
            fileinfo->accessed[1] |= FILE_READ;
        break;
    case FILE_SIGN_FLAG:
        if (fileinfo->accessed[0] & FILE_SIGN)
            fileinfo->accessed[0] &= ~FILE_SIGN;
        else
            fileinfo->accessed[0] |= FILE_SIGN;
        break;
    case FILE_DELETE_FLAG:
        if (fileinfo->accessed[1] & FILE_DEL)
            fileinfo->accessed[1] &= ~FILE_DEL;
        else
            fileinfo->accessed[1] |= FILE_DEL;
        break;
    case FILE_DIGEST_FLAG:
        if (fileinfo->accessed[0] & FILE_DIGEST)
            fileinfo->accessed[0] = (fileinfo->accessed[0] & ~FILE_DIGEST);
        else
            fileinfo->accessed[0] = fileinfo->accessed[0] | FILE_DIGEST;
        break;
    case FILE_IMPORT_FLAG:
        fileinfo->accessed[0] |= FILE_IMPORTED;
        break;
    }

    if (lseek(fd, size * (ent - 1), SEEK_SET) == -1) {
        bbslog("user","%s","subrec seek err");
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &ldata);
        close(fd);
        return DONOTHING;
    }
    if (safewrite(fd, fileinfo, size) != size) {
        bbslog("user","%s","subrec write err");
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &ldata);
        close(fd);
        return DONOTHING;
    }

    ldata.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &ldata);
    close(fd);

    return newent ? DIRCHANGED : PARTUPDATE;
}

int change_post_flag(char *currBM, struct userec *currentuser, int digestmode, char *currboard, int ent, struct fileheader *fileinfo, char *direct, int flag, int prompt)
{
    /*---	---*/
    int newent = 0, ret = 1;
    char *ptr, buf[STRLEN];
    char ans[256];
    char genbuf[1024];
    struct fileheader mkpost, mkpost2;
    struct flock ldata;
    int fd, size = sizeof(fileheader), orgent;

    /*---	---*/

    if (!chk_currBM(currBM, currentuser))
        return DONOTHING;

    if (flag == FILE_DIGEST_FLAG && (digestmode == 1 || digestmode == 4 || digestmode == 5))
        return DONOTHING;
    if (flag == FILE_MARK_FLAG && (digestmode == 1 || digestmode == 4 || digestmode == 5))
        return DONOTHING;
    if (flag == FILE_IMPORT_FLAG && (digestmode == 4 || digestmode == 5))
        return DONOTHING;
    if (flag == FILE_DELETE_FLAG && (digestmode == 4 || digestmode == 5))
        return DONOTHING;
    if ((flag == FILE_MARK_FLAG || flag == FILE_DELETE_FLAG) && (!strcmp(currboard, "syssecurity")
                                                                 || !strcmp(currboard, "Filter")))
        return DONOTHING;       /* Leeward 98.03.29 */
    /*
     * Haohmaru.98.10.12.Ö÷ÌâÄ£Ê½ÏÂ²»ÔÊÐímarkÎÄÕÂ 
     */
    if (flag == FILE_TITLE_FLAG && digestmode != 0)
        return DONOTHING;
    if (flag == FILE_NOREPLY_FLAG && digestmode != 0)
        return DONOTHING;

    if ((digestmode != DIR_MODE_NORMAL) && (digestmode != DIR_MODE_DIGEST)) {
        setbdir(0, genbuf, currboard);
        orgent = search_record(genbuf, &mkpost2, sizeof(struct fileheader), (RECORD_FUNC_ARG) cmpfileinfoname, fileinfo->filename);
        if (!orgent) {
#ifdef BBSMAIN
            move(2, 0);
            prints(" ¸ÃÎÄ¼þ¿ÉÄÜÒÑ¾­±»É¾³ý\n");
            clrtobot();
            pressreturn();
#endif
            return FULLUPDATE;
        }
    }
    strcpy(buf, direct);
    ptr = strrchr(buf, '/') + 1;
    ptr[0] = '\0';
    sprintf(&genbuf[512], "%s%s", buf, fileinfo->filename);
    if (!dashf(&genbuf[512]))
        ret = 0;                /* ½èÓÃÒ»ÏÂnewent :PP   */

    if (ret)
        if ((fd = open(direct, O_RDWR | O_CREAT, 0644)) == -1)
            ret = 0;
    if (ret) {
        ldata.l_type = F_RDLCK;
        ldata.l_whence = 0;
        ldata.l_len = size;
        ldata.l_start = size * (ent - 1);
        if (fcntl(fd, F_SETLKW, &ldata) == -1) {
            bbslog("user","%s","reclock error");
            close(fd);
                                /*---	period	2000-10-20	file should be closed	---*/
            ret = 0;
        }
    }
    if (ret) {
        if (lseek(fd, size * (ent - 1), SEEK_SET) == -1) {
            bbslog("user","%s","subrec seek err");
            /*---	period	2000-10-24	---*/
            ldata.l_type = F_UNLCK;
            fcntl(fd, F_SETLK, &ldata);
            close(fd);
            ret = 0;
        }
    }
    if (ret) {
        if (get_record_handle(fd, &mkpost, sizeof(mkpost), ent) == -1) {
            bbslog("user","%s","subrec read err");
            ret = 0;
        }
        if (ret)
            if (strcmp(mkpost.filename, fileinfo->filename))
                ret = 0;
        if (!ret) {
            newent = search_record_back(fd, sizeof(struct fileheader), ent, (RECORD_FUNC_ARG) cmpfileinfoname, fileinfo->filename, &mkpost, 1);
            ret = (newent > 0);
            if (ret)
                memcpy(fileinfo, &mkpost, sizeof(mkpost));
            else {
                ldata.l_type = F_UNLCK;
                fcntl(fd, F_SETLK, &ldata);
                close(fd);
            }
            ent = newent;
        }
    }
    if (!ret) {
#ifdef BBSMAIN
        move(2, 0);
        prints(" ÎÄÕÂÁÐ±í·¢Éú±ä¶¯£¬ÎÄÕÂ[%s]¿ÉÄÜÒÑ±»É¾³ý£®\n", fileinfo->title);
        clrtobot();
        pressreturn();
#endif
        return DIRCHANGED;
    }
    switch (flag) {
    case FILE_MARK_FLAG:
        if (fileinfo->accessed[0] & FILE_MARKED) {      //added by bad 2002.8.7 mark file mode added
            fileinfo->accessed[0] = (fileinfo->accessed[0] & ~FILE_MARKED);
            bmlog(currentuser->userid, currboard, 7, 1);
        } else {
            fileinfo->accessed[0] = fileinfo->accessed[0] | FILE_MARKED;
            bmlog(currentuser->userid, currboard, 6, 1);
        }
        setboardmark(currboard, 1);
        break;
    case FILE_NOREPLY_FLAG:
        if (fileinfo->accessed[1] & FILE_READ) {
            fileinfo->accessed[1] &= ~FILE_READ;
#ifdef BBSMAIN
            if (prompt)
                a_prompt(-1, " ¸ÃÎÄÕÂÒÑÈ¡Ïû²»¿ÉreÄ£Ê½, Çë°´ Enter ¼ÌÐø << ", ans);
#endif
        } else {
            fileinfo->accessed[1] |= FILE_READ;
#ifdef BBSMAIN
            if (prompt)
                a_prompt(-1, " ¸ÃÎÄÕÂÒÑÉèÎª²»¿ÉreÄ£Ê½, Çë°´ Enter ¼ÌÐø << ", ans);
#endif
            /*
             * Bigman:2000.8.29 sysmail°æ´¦ÀíÌí¼Ó°æÎñÐÕÃû 
             */
            if (!strcmp(currboard, "sysmail")) {
                sprintf(ans, "¡¼%s¡½ ´¦Àí: %s", currentuser->userid, fileinfo->title);
                strncpy(fileinfo->title, ans, STRLEN);
                fileinfo->title[STRLEN - 1] = 0;
            }
        }
        break;
    case FILE_SIGN_FLAG:
        if (fileinfo->accessed[0] & FILE_SIGN) {
            fileinfo->accessed[0] &= ~FILE_SIGN;
#ifdef BBSMAIN
            if (prompt)
                a_prompt(-1, " ¸ÃÎÄÕÂÒÑ³·Ïû±ê¼ÇÄ£Ê½, Çë°´ Enter ¼ÌÐø << ", ans);
#endif
        } else {
            fileinfo->accessed[0] |= FILE_SIGN;
#ifdef BBSMAIN
            if (prompt)
                a_prompt(-1, " ¸ÃÎÄÕÂÒÑÉèÎª±ê¼ÇÄ£Ê½, Çë°´ Enter ¼ÌÐø << ", ans);
#endif
        }
        break;
    case FILE_DELETE_FLAG:
        if (fileinfo->accessed[1] & FILE_DEL)
            fileinfo->accessed[1] &= ~FILE_DEL;
        else
            fileinfo->accessed[1] |= FILE_DEL;
        break;
    case FILE_DIGEST_FLAG:
        if (fileinfo->accessed[0] & FILE_DIGEST) {      /* Èç¹ûÒÑ¾­ÊÇÎÄÕªµÄ»°£¬Ôò´ÓÎÄÕªÖÐÉ¾³ý¸Ãpost */
            fileinfo->accessed[0] = (fileinfo->accessed[0] & ~FILE_DIGEST);
            bmlog(currentuser->userid, currboard, 4, 1);
            dele_digest(fileinfo->filename, direct);
        } else {
            struct fileheader digest;
            char *ptr, buf[64];

            memcpy(&digest, fileinfo, sizeof(digest));
            if (digestmode)
                strncpy(digest.title, mkpost2.title, STRLEN);
            digest.filename[0] = 'G';
            strcpy(buf, direct);
            ptr = strrchr(buf, '/') + 1;
            ptr[0] = '\0';
            sprintf(genbuf, "%s%s", buf, digest.filename);
            bmlog(currentuser->userid, currboard, 3, 1);
            if (dashf(genbuf)) {
                fileinfo->accessed[0] = fileinfo->accessed[0] | FILE_DIGEST;
            } else {
                digest.accessed[0] = 0;
                sprintf(&genbuf[512], "%s%s", buf, fileinfo->filename);
                link(&genbuf[512], genbuf);
                strcpy(ptr, DIGEST_DIR);
                if (get_num_records(buf, sizeof(digest)) > MAX_DIGEST) {
                    ldata.l_type = F_UNLCK;
                    fcntl(fd, F_SETLK, &ldata);
                    close(fd);
#ifdef BBSMAIN
                    move(3, 0);
                    clrtobot();
                    move(4, 10);
                    prints("±§Ç¸£¬ÄãµÄÎÄÕªÎÄÕÂÒÑ¾­³¬¹ý %d Æª£¬ÎÞ·¨ÔÙ¼ÓÈë...\n", MAX_DIGEST);
                    pressanykey();
#endif
                    return PARTUPDATE;
                }
                append_record(buf, &digest, sizeof(digest));    /* ÎÄÕªÄ¿Â¼ÏÂÌí¼Ó .DIR */
                fileinfo->accessed[0] = fileinfo->accessed[0] | FILE_DIGEST;
            }
        }
        break;
    case FILE_TITLE_FLAG:
        fileinfo->groupid = fileinfo->id;
        fileinfo->reid = fileinfo->id;
        if (!strncmp(fileinfo->title, "Re:", 3)) {
            strcpy(buf, fileinfo->title + 4);
            strcpy(fileinfo->title, buf);
        }
        break;
    case FILE_IMPORT_FLAG:
        fileinfo->accessed[0] |= FILE_IMPORTED;
        break;
    }

    if (lseek(fd, size * (ent - 1), SEEK_SET) == -1) {
        bbslog("user","%s","subrec seek err");
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &ldata);
        close(fd);
        return DONOTHING;
    }
    if (safewrite(fd, fileinfo, size) != size) {
        bbslog("user","%s","subrec write err");
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &ldata);
        close(fd);
        return DONOTHING;
    }

    ldata.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &ldata);
    close(fd);
    if ((digestmode != DIR_MODE_NORMAL) && (DIR_MODE_DIGEST))
        change_dir_post_flag(currentuser, currboard, orgent, &mkpost2, flag);

    return newent ? DIRCHANGED : PARTUPDATE;
}

char get_article_flag(struct fileheader *ent, struct userec *user, int is_bm)
{
    char unread_mark = (DEFINE(user, DEF_UNREADMARK) ? '*' : 'N');
    char type;

    if (strcmp(user->userid,"guest"))
        type = brc_unread(ent->id) ? unread_mark : ' ';
    else
        type = ' ';
    if ((ent->accessed[0] & FILE_DIGEST)) {
        if (type == ' ')
            type = 'g';
        else
            type = 'G';
    }
    if (ent->accessed[0] & FILE_MARKED) {
        switch (type) {
        case ' ':
            type = 'm';
            break;
        case '*':
        case 'N':
            type = 'M';
            break;
        case 'g':
            type = 'b';
            break;
        case 'G':
            type = 'B';
            break;
        }
    }
    if (is_bm && (ent->accessed[1] & FILE_READ)) {
        switch (type) {
        case 'g':
        case 'G':
            type = 'O';
            break;
        case 'm':
        case 'M':
            type = 'U';
            break;
        case 'b':
        case 'B':
            type = '8';
            break;
        case ' ':
        case '*':
        case 'N':
        default:
            type = ';';
            break;
        }
    } else if (HAS_PERM(user, PERM_OBOARDS) && (ent->accessed[0] & FILE_SIGN)) {
        type = '#';
    }

    if (is_bm && (ent->accessed[1] & FILE_DEL)) {
        type = 'X';
    }

    return type;
}
