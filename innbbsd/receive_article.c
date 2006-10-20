/*
      BBS implementation dependendent part

      The only two interfaces you must provide

      #include "inntobbs.h"
      int receive_article();
      0 success
      not 0 fail

      if (storeDB(HEADER[MID_H], hispaths) < 0) {
         .... fail
      }

      int cancel_article_front( char *msgid );
      0 success
      not 0 fail

      char *ptr = (char*)DBfetch(msgid);

     the post contents received (body) is in char *BODY,
     (header) in char *HEADER[]
     SUBJECT_H, FROM_H, DATE_H, MID_H, NEWSGROUPS_H,
     NNTPPOSTINGHOST_H, NNTPHOST_H, CONTROL_H, PATH_H,
     ORGANIZATION_H

     To filter input text, another set of HEADER is processed
     at first and BODY processed later.
*/

/*
   Sample Implementation

   receive_article()         --> post_article()   --> bbspost_write_post();
   cacnel_article_front(mid) --> cancel_article() --> bbspost_write_cancel();
*/

#include "innbbsconf.h"
#include "daemon.h"
#include "bbslib.h"
#include "inntobbs.h"
#include "lang.h"
#include "bbs.h"
#include "inn_funcs.h"
#include "nocem.h"

#define innbbslog(x) bbslog("3rror",x)
extern int Junkhistory;

static char *post_article (char *, char *, char *, int (*)(), char *, char *);
static int cancel_article (char *, char *);

#define FAILED goto failed

/* process post write */
static int bbspost_write_post(int fh, char *board, char *filename)
{
    char *fptr, *ptr;
    FILE *fhfd;
    char conv_buf[256];

    fhfd = fdopen(fh, "w");
    if (fhfd == NULL) {
        innbbsdlog("can't fdopen, maybe disk full\n");
        return -1;
    }
    if (strlen(SUBJECT) > 256)
        FAILED;
    str_decode((unsigned char*)conv_buf, (unsigned char*)SUBJECT);
    /* big 标题转码，original patch by dgwang @ 笔山书院 */
    if(strstr(SUBJECT,"=?big5?") || strstr(SUBJECT,"=?Big5?") ||
       strstr(SUBJECT,"=?BIG5?") ){
       int len;
       len = strlen(conv_buf);
       big2gb(conv_buf,&len,0,getSession());
    }

    if (fprintf(fhfd, "%s%s, %s%s\n", FromTxt, FROM, BoardTxt, board) == EOF
        || fprintf(fhfd, "%s%.70s\n", SubjectTxt, conv_buf) == EOF || fprintf(fhfd, "%s%.43s (%s)\n", OrganizationTxt, SITE, DATE) == EOF || fprintf(fhfd, "%s%s\n", PathTxt, PATH) == EOF)
        FAILED;

    if (POSTHOST != NULL) {
        if (fprintf(fhfd, "出  处: %.70s\n", POSTHOST) == EOF)
            FAILED;
    }
    if (fprintf(fhfd, "\n") == EOF)
        FAILED;
    for (fptr = BODY, ptr = strchr(fptr, '\r'); ptr != NULL && *ptr != '\0'; fptr = ptr + 1, ptr = strchr(fptr, '\r')) {
        int ch = *ptr;

        *ptr = '\0';
        /*
         * 990530.edwardc 这里应该要处理，采用留住单一 "." 的方式 
         */
        if (fptr[0] == '.' && fptr[1] == '.')
            fptr++;
        if (fputs(fptr, fhfd) == EOF)
            FAILED;
        *ptr = ch;
    }

    /*
     * 990530.edwardc 这里也是 
     */
    if (fptr[0] == '.' && fptr[1] == '.')
        fptr++;

    if (fputs(fptr, fhfd) == EOF)
        FAILED;

    fflush(fhfd);
    fclose(fhfd);
    return 0;
  failed:
    fclose(fhfd);
    return -1;
}

static int bbspost_write_control(int fh, char *board, char *filename)
{
    char *fptr, *ptr;
    FILE *fhfd = fdopen(fh, "w");

    if (fhfd == NULL) {
        innbbsdlog("can't fdopen, maybe disk full\n");
        return -1;
    }
    if (fprintf(fhfd, "Path: %s!%s\n", MYBBSID, HEADER[PATH_H]) == EOF ||
        fprintf(fhfd, "From: %s\n", FROM) == EOF ||
        fprintf(fhfd, "Newsgroups: %s\n", GROUPS) == EOF ||
        fprintf(fhfd, "Subject: %s\n", SUBJECT) == EOF || fprintf(fhfd, "Date: %s\n", DATE) == EOF || fprintf(fhfd, "Organization: %s\n", SITE) == EOF) {
        fclose(fhfd);
        return -1;
    }
    if (POSTHOST != NULL) {
        if (fprintf(fhfd, "NNTP-Posting-Host: %.70s\n", POSTHOST) == EOF)
            FAILED;
    }
    if (HEADER[CONTROL_H] != NULL) {
        if (fprintf(fhfd, "Control: %s\n", HEADER[CONTROL_H]) == EOF)
            FAILED;
    }
    if (HEADER[APPROVED_H] != NULL) {
        if (fprintf(fhfd, "Approved: %s\n", HEADER[APPROVED_H]) == EOF)
            FAILED;
    }
    if (HEADER[DISTRIBUTION_H] != NULL) {
        if (fprintf(fhfd, "Distribution: %s\n", HEADER[DISTRIBUTION_H])
            == EOF)
            FAILED;
    }
    if (fprintf(fhfd, "\n") == EOF)
        FAILED;
    for (fptr = BODY, ptr = strchr(fptr, '\r'); ptr != NULL && *ptr != '\0'; fptr = ptr + 1, ptr = strchr(fptr, '\r')) {
        int ch = *ptr;

        *ptr = '\0';
        /*
         * 990530.edwardc 这里应该要处理，采用留住单一 "." 的方式 
         */
        if (fptr[0] == '.' && fptr[1] == '.')
            fptr++;
        if (fputs(fptr, fhfd) == EOF)
            FAILED;
        *ptr = ch;
    }

    if (fptr[0] == '.' && fptr[1] == '.')
        fptr++;
    if (fputs(fptr, fhfd) == EOF)
        FAILED;

    fflush(fhfd);
    fclose(fhfd);
    return 0;
  failed:
    fclose(fhfd);
    return -1;
}

/* process cancel write */
int receive_article()
{
    char *user, *userptr;
    char *ngptr, *pathptr;
    char **splitptr;
    static char userid[32];
    static char xdate[32];
    static char xpath[180];
    time_t datevalue;
    newsfeeds_t *nf;
    char *boardhome;
    char hispaths[4096];
    char firstpath[MAXPATHLEN], *firstpathbase;
    char *lesssym, *nameptrleft, *nameptrright;
    static char sitebuf[80];

    if (FROM == NULL) {
        innbbsdlog(":Err: article without usrid %s\n", MSGID);
        return 0;
    }
    if (strchr(FROM, '<') && (FROM[strlen(FROM) - 1] == '>'))
        user = (char *) strrchr(FROM, '@');
    else
        user = (char *) strchr(FROM, '@');
    lesssym = (char *) strchr(FROM, '<');
    nameptrleft = NULL, nameptrright = NULL;
    if (lesssym == NULL || lesssym >= user) {
        lesssym = FROM;
        nameptrleft = strchr(FROM, '(');
        if (nameptrleft != NULL)
            nameptrleft++;
        nameptrright = strrchr(FROM, ')');
    } else {
        nameptrleft = FROM;
        nameptrright = strrchr(FROM, '<');
        lesssym++;
    }
    if (user != NULL) {
        *user = '\0';
        userptr = (char *) strchr(FROM, '.');
        if (userptr != NULL) {
            *userptr = '\0';
            strncpy(userid, lesssym, sizeof userid);
            *userptr = '.';
        } else {
            strncpy(userid, lesssym, sizeof userid);
        }
        *user = '@';
    } else {
        strncpy(userid, lesssym, sizeof userid);
    }
    if (!isalnum(userid[0]))
        strcpy(userid, "Unknown");
    strcat(userid, ".");
    datevalue = parsedate(DATE, NULL);

    if (datevalue > 0) {
        char *p;

        strncpy(xdate, ctime(&datevalue), sizeof(xdate));
        p = (char *) strchr(xdate, '\n');
        if (p != NULL)
            *p = '\0';
        DATE = xdate;
    }
    if (SITE && strcasecmp("Computer Science & Information Engineering NCTU", SITE) == 0) {
        SITE = NCTUCSIETxt;
    } else if (SITE && strcasecmp("Dep. Computer Sci. & Information Eng., Chiao Tung Univ., Taiwan, R.O.C", SITE) == 0) {
        SITE = NCTUCSIETxt;
    } else if (SITE == NULL || *SITE == '\0') {
        if (nameptrleft != NULL && nameptrright != NULL) {
            char savech = *nameptrright;

            *nameptrright = '\0';
            strncpy(sitebuf, nameptrleft, sizeof sitebuf);
            *nameptrright = savech;
            SITE = sitebuf;
        } else
            /*
             * SITE = "(Unknown)"; 
             */
            SITE = "";
    }
    if (strlen(MYBBSID) > 70) {
        innbbsdlog(" :Err: your bbsid %s too long\n", MYBBSID);
        return 0;
    }
    sprintf(xpath, "%s!%.*s", MYBBSID, sizeof(xpath) - strlen(MYBBSID) - 2, PATH);
    PATH = xpath;
    for (pathptr = PATH; pathptr != NULL && (pathptr = strstr(pathptr, ".edu.tw")) != NULL;) {
        if (pathptr != NULL) {
            strcpy(pathptr, pathptr + 7);
        }
    }
    xpath[71] = '\0';

    echomaillog();
    *hispaths = '\0';
    splitptr = (char **) BNGsplit(GROUPS);
    firstpath[0] = '\0';
    firstpathbase = firstpath;
    /*
     * try to split newsgroups into separate group and check if any
     * duplicated
     */

    /*
     * try to use hardlink 
     */
    /*
     * for ( ngptr = GROUPS, nngptr = (char*) strchr(ngptr,','); ngptr !=
     * NULL && *ngptr != '\0'; nngptr = (char*)strchr(ngptr,',')) {
     */
    for (ngptr = *splitptr; ngptr != NULL; ngptr = *(++splitptr)) {
        char *boardptr, *nboardptr;

        /*
         * if (nngptr != NULL) { nngptr = '\0'; }
         */
        if (*ngptr == '\0')
            continue;
        nf = (newsfeeds_t *) search_group(ngptr);
        /*
         * printf("board %s\n",nf->board); 
         */
        if (nf == NULL) {
            innbbsdlog("unwanted \'%s\'\n", ngptr);
            /*
             * if( strstr( ngptr, "tw.bbs" ) != NULL ) { }
             */
            continue;
        }
        if (nf->board == NULL || !*nf->board)
            continue;
        if (nf->path == NULL || !*nf->path)
            continue;
        for (boardptr = nf->board, nboardptr = (char *) strchr(boardptr, ','); boardptr != NULL && *boardptr != '\0'; nboardptr = (char *) strchr(boardptr, ',')) {
            if (nboardptr != NULL) {
                *nboardptr = '\0';
            }
            if (*boardptr == '\t') {
                goto boardcont;
            }
            boardhome = (char *) fileglue("%s/boards/%s", BBSHOME, boardptr);
            if (!isdir(boardhome)) {
                innbbsdlog(":Err: unable to write %s\n", boardhome);
            } else {
                char *fname;

                /*
                 * if ( !isdir( boardhome )) { innbbsdlog( ":Err:
                 * unable to write %s\n",boardhome);
                 * testandmkdir(boardhome); }
                 */
                fname = (char *) post_article(boardhome, userid, boardptr, bbspost_write_post, NULL, firstpath);
                if (fname != NULL) {
                    fname = (char *) fileglue("%s/%s", boardptr, fname);
                    if (firstpath[0] == '\0') {
                        sprintf(firstpath, "%s/boards/%s", BBSHOME, fname);
                        firstpathbase = firstpath + strlen(BBSHOME) + strlen("/boards/");
                    }
                    if (strlen(fname) + strlen(hispaths) + 1 < sizeof(hispaths)) {
                        strcat(hispaths, fname);
                        strcat(hispaths, " ");
                    }
                } else {
                    innbbsdlog("fname is null %s\n", boardhome);
                    return -1;
                }
            }

          boardcont:
            if (nboardptr != NULL) {
                *nboardptr = ',';
                boardptr = nboardptr + 1;
            } else
                break;

        }                       /* for board1,board2,... */
        /*
         * if (nngptr != NULL) ngptr = nngptr + 1; else break;
         */
        if (*firstpathbase)
            feedfplog(nf, firstpathbase, 'P');
    }
    if (*hispaths)
        bbsfeedslog(hispaths, 'P');

    if (Junkhistory || *hispaths) {
        if (storeDB(HEADER[MID_H], hispaths) < 0) {
            innbbsdlog("store DB fail\n");
            /*
             * I suspect here will introduce duplicated articles 
             */
            /*
             * return -1; 
             */
        }
    }
    return 0;
}

int receive_control()
{
    char *boardhome, *fname;
    char firstpath[MAXPATHLEN], *firstpathbase;
    char **splitptr, *ngptr;
    newsfeeds_t *nf;

    innbbsdlog("control post %s\n", HEADER[CONTROL_H]);
    boardhome = (char *) fileglue("%s/boards/control", BBSHOME);
    testandmkdir(boardhome);
    *firstpath = '\0';
    if (isdir(boardhome)) {
        fname = (char *) post_article(boardhome, FROM, "control", bbspost_write_control, NULL, firstpath);
        if (fname != NULL) {
            if (firstpath[0] == '\0')
                sprintf(firstpath, "%s/boards/control/%s", BBSHOME, fname);
            if (storeDB(HEADER[MID_H], (char *) fileglue("control/%s", fname)) < 0) {
            }
            bbsfeedslog(fileglue("control/%s", fname), 'C');
            firstpathbase = firstpath + strlen(BBSHOME) + strlen("/boards/");
            splitptr = (char **) BNGsplit(GROUPS);
            for (ngptr = *splitptr; ngptr != NULL; ngptr = *(++splitptr)) {
                if (*ngptr == '\0')
                    continue;
                nf = (newsfeeds_t *) search_group(ngptr);
                if (nf == NULL)
                    continue;
                if (nf->board == NULL)
                    continue;
                if (nf->path == NULL)
                    continue;
                feedfplog(nf, firstpathbase, 'C');
            }
        }
    }
    return 0;
}

int cancel_article_front(char *msgid)
{
    char *ptr = (char *) DBfetch(msgid);
    char *filelist, filename[2048];
    char histent[4096];
    char firstpath[MAXPATHLEN], *firstpathbase = NULL;

    if (ptr == NULL) {
        return 0;
    }
    strncpy(histent, ptr, sizeof histent);
    ptr = histent;
#ifdef DEBUG
    printf("**** try to cancel %s *****\n", ptr);
#endif
    innbbsdlog("**** try to cancel %s *****\n", ptr);
    filelist = strchr(ptr, '\t');
    if (filelist != NULL) {
        filelist++;
    }
    *firstpath = '\0';
    for (ptr = filelist; ptr && *ptr;) {
        char *file;

        for (; *ptr && isspace(*ptr); ptr++);
        if (*ptr == '\0')
            break;
        file = ptr;
        for (ptr++; *ptr && !isspace(*ptr); ptr++);
        if (*ptr != '\0') {
            *ptr++ = '\0';
        }
        sprintf(filename, "%s/boards/%s", BBSHOME, file);
        innbbsdlog("**** Get file %s ****\n", ptr);
        if (isfile(filename)) {
/*              FILE *fp = fopen(filename,"r");
              char buffer[1024];
              char *xfrom, *xpath, *boardhome;

              if (fp == NULL) continue;
              while (fgets(buffer,sizeof (buffer), fp) != NULL) {
                 char *hptr;
                 if (buffer[0]=='\n') break;
                 hptr = strchr(buffer,'\n');
                 if (hptr != NULL) *hptr = '\0';
                 if (strncmp(buffer,FromTxt,8)==0) {
                    char* n;
                    n = strrchr(buffer,',');
                    if (n!=NULL) *n = '\0';
                    xfrom = buffer+8;
                 } else if (strncmp(buffer,PathTxt,8)==0) {
                    xpath = buffer+8;
                 }
              }
              fclose(fp);
*/

            FILE *fp;
            char buffer[1024];
            char *xfrom;

            fp = fopen(filename, "r");
            if (fp != NULL) {
                fgets(buffer, sizeof(buffer), fp);
                fclose(fp);
            }
            xfrom = strtok(buffer, " ");
            if (xfrom!=NULL && strncmp(xfrom, FromTxt, 7) == 0) {
                xfrom = (char *) strtok(NULL, ",\r\n");
            } else {
                xfrom = "\0";
            }
            if (xfrom == NULL || (strcmp(HEADER[FROM_H], xfrom)
#ifdef USE_NCM_PATCH
                && !search_issuer(FROM)
#endif
	       )) {
                innbbsdlog("Invalid cancel %s, path: %s!%s\n", FROM, MYBBSID, PATH);
                return 0;
            }
            innbbsdlog("cancel post %s\n", filename);
            {
                char *fp = strchr(file, '/');

                if (fp != NULL) {
                    *fp = '\0';
                    cancel_article(file, fp + 1);
                    *fp = '/';
                }
            }
        }
    }
    if (*firstpath) {
        char **splitptr, *ngptr;
        newsfeeds_t *nf;

        splitptr = (char **) BNGsplit(GROUPS);
        for (ngptr = *splitptr; ngptr != NULL; ngptr = *(++splitptr)) {
            if (*ngptr == '\0')
                continue;
            nf = (newsfeeds_t *) search_group(ngptr);
            if (nf == NULL)
                continue;
            if (nf->board == NULL)
                continue;
            if (nf->path == NULL)
                continue;
            feedfplog(nf, firstpathbase, 'D');
        }
    }
    return 0;
}

#define NO_OSDEP_H              /* skip the os_dep.h, we have it already! */
#include "bbs.h"
#undef  OS_OSDEP_H

static int cmp_title(char *title, struct fileheader *fh1)
{
    char *p1;

    if (!strncasecmp(fh1->title, "Re: ", 4))
        p1 = fh1->title + 4;
    else if (!strncasecmp(fh1->title, "回复: ", 6))
        p1 = fh1->title + 6;
    else
        p1 = fh1->title;
    return (!strncmp(p1, title, ARTICLE_TITLE_LEN));
}

static int find_thread(struct fileheader *fh, char *board, char *title)
{
    char direct[255];
    char *p;
    int ret;
    int fd;

    setbfile(direct, board, DOT_DIR);
    if ((fd = open(direct, O_RDONLY, 0644)) == -1)
        return 0;
    if (!strncasecmp(title, "Re:", 3))
        p = title + 4;
    else if (!strncasecmp(title, "回复:", 5))
        p = title + 6;
    else
        p = title;
    ret = search_record_back_lite(fd, sizeof(struct fileheader), 0, 1000, (RECORD_FUNC_ARG) cmp_title, p, fh, 1);
    close(fd);
    return ret;
}

static char *post_article(char *homepath, char *userid, char *board, int (*writebody)(), char *pathname, char *firstpath)
{
    struct fileheader header;
    struct fileheader threadfh;

/*    char        *subject = SUBJECT;  */
    char index[MAXPATHLEN];
    static char name[MAXPATHLEN];
    char article[MAXPATHLEN];
    FILE *fidx;
    int fh;
    int ret;
    int linkflag;
    char conv_buf[256];
    char old_path[255];

    getcwd(old_path, 255);
    sprintf(index, "%s/.DIR", homepath);
    if ((fidx = fopen(index, "r")) == NULL) {
        if ((fidx = fopen(index, "w")) == NULL) {
            innbbsdlog(":Err: Unable to post in %s.\n", homepath);
            return NULL;
        }
    }
    fclose(fidx);

    if (GET_POSTFILENAME(name, homepath) < 0)
	{
		innbbslog(" Err: can't get a postfile name\n");
		return NULL;
	}
	sprintf(article, "%s/%s", homepath, name);
	fh = open(article, O_WRONLY, 0644);

#ifdef DEBUG
    printf("post to %s\n", article);
#endif

    linkflag = 1;
    if (firstpath && *firstpath) {
        close(fh);
        unlink(article);
#ifdef DEBUGLINK
        innbbsdlog("try to link %s to %s", firstpath, article);
#endif
        linkflag = link(firstpath, article);
        if (linkflag) {
            fh = open(article, O_CREAT | O_EXCL | O_WRONLY, 0644);
        }
    }
    if (linkflag != 0) {
        if (writebody) {
            if ((*writebody) (fh, board, pathname) < 0)
                return NULL;
        } else {
            if (bbspost_write_post(fh, board, pathname) < 0)
                return NULL;
        }
        close(fh);
    }
    bzero((void *) &header, sizeof(header));
    strcpy(header.filename, name);
    strncpy(header.owner, userid, OWNER_LEN);
    header.owner[OWNER_LEN - 1] = 0;

    str_decode((unsigned char*)conv_buf, (unsigned char*)SUBJECT);
    /* big 标题转码，original patch by dgwang @ 笔山书院 */
    if(strstr(SUBJECT,"=?big5?") || strstr(SUBJECT,"=?Big5?") ||
       strstr(SUBJECT,"=?BIG5?") ){
       int len;
       len = strlen(conv_buf);
       big2gb(conv_buf,&len,0,getSession());
    }

    strnzhcpy(header.title, conv_buf, ARTICLE_TITLE_LEN);
    header.innflag[1] = 'M';
	set_posttime(&header);
    /*
     * if append record record, should return fail message 
     */
    chdir(BBSHOME);
    resolve_boards();
    linkflag = find_thread(&threadfh, board, header.title);
    header.eff_size = get_effsize(article);
    ret = after_post(NULL, &header, board, linkflag ? &threadfh : NULL, 0, getSession());
    if ((ret < 0) && (ret != -2)) {
        innbbslog(":Err:after_post Unable to post.\n");
        chdir(old_path);
        return NULL;
    }
    chdir(old_path);
    return name;
}

static int cmp_filename(void *arg, void *this_fh) {
    char *filename = (char *)arg;
    struct fileheader *fh = (struct fileheader *)this_fh;
    return (!strcmp(filename, fh->filename));
}
    
static int cancel_article(char *board, char *file)
{
    struct fileheader header;
    char dirname[MAXPATHLEN];
    char *basename;
    time_t time;
    int fd, ent;
    char old_path[255];

    if (file == NULL || strlen(file) < 3)
        return 0;
    basename = (file[1]=='/') ? (file + 2) : file;
    if (basename[0] != 'M' || basename[1] != '.' || (time = atoi(basename + 2)) <= 0)
        return 0;

    getcwd(old_path, 255);
    chdir(BBSHOME);
    setbdir(DIR_MODE_NORMAL,dirname,board);
    if ((fd = open(dirname, O_RDWR)) == -1) {
        chdir(old_path);
        return 0;
    }

    ent = search_record_back(fd, sizeof(struct fileheader),0,cmp_filename, file, &header,1);
    if (ent) {
        struct write_dir_arg delarg;
        struct userec user;
        init_write_dir_arg(&delarg);
        delarg.fd=fd;
        delarg.ent=ent;
        getCurrentUser() = &user;
        strcpy(user.userid, "<innd>");
        do_del_post(getCurrentUser(),&delarg,&header,board,DIR_MODE_NORMAL,0,getSession());
        free_write_dir_arg(&delarg);
    }
    close(fd);
    chdir(old_path);
    return 0;
}


