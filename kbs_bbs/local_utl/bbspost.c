/* Many places of this file are modified by Leeward
   for SMTH BBS's WWW interface services (1997-1998)
*/

/*
 * File: scanboard.c
 */
char *ProgramUsage = "\
                     bbspost (list|visit) bbs_home\n\
                     post board_path < uid + title + Article...\n\
                     mail board_path < uid + title + passwd + realfrom + Article...\n\
                     cancel bbs_home board filename\n\
                     expire bbs_home board days [max_posts] [min_posts]\n";

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "bbs.h"
/*#include "../bbs.h"  Leeward 98.04.27 For macro LEEWARD_X_RECORD*/
#define MAXLEN          1024

char currboard[1];

char            *crypt();
char            *homepath;
int             visitflag;
char emailad[STRLEN];
char fromhost[IPLEN]; /* Leeward: 97.12.20: ÈÃ 3W ·¢ÎÄÓ°ÏìÉúÃüÁ¦ */
char EMode = ' '; /* Leeward: 97.12.23: for 3W modify article */
int 		sign;/*Haohmaru.µÚsign¸öÇ©Ãûµµ*/

struct userec alluser;

int
CheckBM(BMstr,record)   /* ¸ù¾ÝÊäÈëµÄ°æÖ÷Ãûµ¥ ÅÐ¶Ïµ±Ç°userÊÇ·ñÊÇ°æÖ÷ added by dong, 1998.9.28*/
char *BMstr;
struct userec record;
{
    char *ptr;
    char BMstrbuf[STRLEN-1];

    if(HAS_PERM(currentuser,PERM_OBOARDS)||HAS_PERM(currentuser,PERM_SYSOP))
        return 1;
    if(!HAS_PERM(currentuser,PERM_BOARDS))
        return 0;
    strcpy(BMstrbuf,BMstr);
    ptr=strtok(BMstrbuf,",: ;|&()\0\n");
    while(1)
    {
        if(ptr==NULL)
            return 0;
        if(!strcmp(ptr,record.userid))
            return 1;
        ptr=strtok(NULL,",: ;|&()\0\n");
    }
}

void
AddSignature(FILE *fp, char *username, int fh) /* Leeward: 98.05.17 */
{
    /* Leeward: 97.12.22: WWW-POST ×Ô¶¯Ìí¼ÓÇ©Ãûµµ; 98.05.17 fixing a bug */
    FILE *sig;
    char szSig[128];
    int  i;

    /* by zixia: ÓÃÏà¶ÔÂ·¾­ sprintf(szSig, "%shome/%c/%s/signatures", BBSHOME, toupper(username[0]), username);*/
    sprintf(szSig, "home/%c/%s/signatures", toupper(username[0]), username);
    sig = fopen(szSig, "rt");
    /*Haohmaru.99.11.24.below 9 lines*/
    if (!sign)
    {
        if (fp) fputs("\n\n--", fp);
        else if (fh) write(fh, "\n\n--", 4);
        return;
    }

    for(i = 0; i < 6*(sign - 1); i++)
        if (fgets(szSig, 128, sig))
            continue;
        else
            break;

    if (sig)
    {
        if (fp)
        {
            fputs("\n--\n", fp);
            for (i = 0; i < 6; i ++)
                if (fgets(szSig, 128, sig))
                    fputs(szSig, fp);
                else
                    break;
        }
        else if (fh)
        {
            write(fh, "\n--\n", 4);
            for (i = 0; i < 6; i ++)
                if (fgets(szSig, 128, sig))
                    write(fh, szSig, strlen(szSig));
                else
                    break;
        }
        fclose(sig);
    }
    else
    { /* Leeward 98.06.12 processes the situation of no signature */
        if (fp) fputs("\n\n--", fp);
        else if (fh) write(fh, "\n\n--", 4);
    }
}

usage()
{
    printf( ProgramUsage );
    exit( 0 );
}

static int
cmpbnames( bname, brec)
char *bname;
struct fileheader *brec;
{
    if (!strncasecmp( bname, brec->filename, sizeof(brec->filename)))
        return 1;
    else
        return 0;
}

void search_article( brdname )
char    *brdname;
{
    struct fileheader   head;
    struct stat         state;
    char        index[ MAXLEN ], article[ MAXLEN ];
    int         fd, num, offset, type;
    char        send;

    offset = (int) &(head.filename[ STRLEN - 1 ]) - (int) &head;
    sprintf( index, "%s/boards/%s/.DIR", homepath, brdname );
    if( (fd = open( index, O_RDWR )) < 0 ) {
        return;
    }
    fstat( fd, &state );
    num = (state.st_size / sizeof( head )) - 1;
    while( num >= 0 ) {
        lseek( fd, num * sizeof( head ) + offset, 0 );
        if( read( fd, &send, 1 ) > 0 && send == '%' )  break;
        num -= 4;
    }
    num++;
    if( num < 0 )  num = 0;
    lseek( fd, num * sizeof( head ), 0 );
    for( send = '%'; read( fd, &head, sizeof( head )) > 0; num++ ) {
        type = head.filename[ STRLEN - 1 ];
        if( type != send && visitflag ) {
            lseek( fd, num * sizeof( head ) + offset, 0 );
            write( fd, &send, 1 );
            lseek( fd, (num + 1) * sizeof( head ), 0 );
        }
        if( type == '\0' ) {
            printf( "%s\t%s\t%s\t%s\n", brdname,
                    head.filename, head.owner, head.title );
        }
    }
    close( fd );
}

void search_boards( visit )
int visit;
{
    struct dirent       *de;
    DIR                 *dirp;
    char        buf[ 8192 ], *ptr;
    int         fd, len;

    visitflag = visit;
    sprintf( buf, "%s/boards", homepath );

    if ((dirp = opendir( buf )) == NULL ) {
        printf( ":Err: unable to open %s\n", buf );
        return;
    }

    printf( "New article listed:\n" );

    while ((de = readdir( dirp )) != NULL ) {
        if( de->d_name[0] > ' ' && de->d_name[0] != '.' )
            search_article( de->d_name );
    }

    closedir( dirp );
}

check_password( record )
struct userec   *record;
{
    FILE        *fn;
    char        *pw;
    char        passwd[ MAXLEN ];
    char        realfrom[ MAXLEN ];
    char        genbuf[ MAXLEN ];

    if (NULL!=fgets(passwd, MAXLEN, stdin))
	passwd[strlen(passwd)-1]=0;
    
    if( !checkpasswd2(passwd,record) ){
        printf( ":Err: user '%s' password incorrect!!\n", record->userid );
        exit( 0 );
    }
    if (NULL!=fgets(realfrom,MAXLEN, stdin))
	realfrom[strlen(realfrom)-1]=0;
    strcpy(emailad,realfrom);
    /*    sprintf( genbuf, "tmp/email_%s", record->userid );
        if( (fn = fopen( genbuf, "w" )) != NULL ) {
            fprintf( fn, "%s\n", realfrom );
            fclose( fn );
        }
        if( !strstr( realfrom, "bbs@" ) ) {
            record->termtype[ 15 ] = '\0';
            strncpy( record->termtype+16, realfrom, STRLEN-16 );
        }
    */
    /* Below moved elsewhere ... Leeward 98.04.28 */
    /*if( !strstr( homepath, "test" ) && ' ' == EMode ) {*/ /* Leeward 97.12.23 */
    /*record->numposts++;
}*/

    /* Leeward: 97.12.20: ÈÃ 3W ·¢ÎÄÓ°ÏìÉúÃüÁ¦ */
    strcpy(record->lasthost, fromhost);
    record->lastlogin = time(NULL) ;
}

check_userec( record, name )
struct userec   *record;
char    *name;
{
    int         fh;
    char        exitfile[80];
    int userindex;

    if( (fh = open( ".PASSWDS", O_RDWR )) == -1 ) {
        printf( ":Err: unable to open .PASSWDS file.\n" );
        exit( 0 );
    }
    /*while( read( fh, record, sizeof *record ) > 0 ) {
        if( strcasecmp( name, record->userid ) == 0 ) {*/

    if (userindex = searchuser(name)) /* modified by dong , 1998.11.2 */
    {
        lseek(fh, (userindex-1) * sizeof(*record), SEEK_SET);
        read(fh, record, sizeof *record);

        strcpy( name, record->userid );
        check_password( record );
        lseek( fh, -1 * sizeof *record, SEEK_CUR );
        write( fh, record, sizeof *record );
        close( fh );
        /* Leeward 98.10.26 */
        sprintf(exitfile, "home/%c/%s/exit", toupper(name[0]), name);
        if (- 1 == (fh = open(exitfile, O_WRONLY|O_CREAT, 0644))) {
            printf(":Err: failed login exit time for userid %s\n", name);
        } else {
            record->lastlogin ++;
            write(fh, &record->lastlogin, sizeof(record->lastlogin));
            close(fh);
        }
        return;
        /*}*/
    }
    close( fh );
    printf( ":Err: unknown (1) userid %s\n", name );
    /*    exit( 0 );*/return;
}


Xuserec( record, name, delta)
struct userec   *record;
char    *name;
int delta;
{
    int         fh;
    int userindex;
    if( (fh = open( ".PASSWDS", O_RDWR )) == -1 ) {
        printf( ":Err: unable to open .PASSWDS file.\n" );
        exit( 0 );
    }
    /*while( read( fh, record, sizeof *record ) > 0 ) {
        if( strcasecmp( name, record->userid ) == 0 ) {*/

    if (userindex = searchuser(name)) /* modified by dong , 1998.11.2 */
    {
        lseek(fh, (userindex-1) * sizeof(*record), SEEK_SET);
        read(fh, record, sizeof *record);

        strcpy( name, record->userid );
        lseek( fh, -1 * sizeof *record, SEEK_CUR );
        record->numposts += delta;
        write( fh, record, sizeof *record );
        close( fh );
        return;
        /*}*/
    }
    close( fh );
    printf( ":Err: unknown (2) userid %s\n", name );
    exit( 0 );
}

Xuserec2(name) /* Leeward 98.10.01 Fix a bug */
char    *name;
{
    int         fh;
    struct      userec   record;
    int userindex;
    if( (fh = open( ".PASSWDS", O_RDWR )) == -1 ) {
        printf( ":Err: unable to open .PASSWDS file.\n" );
        exit( 0 );
    }

    /*while( read( fh, &record, sizeof record ) > 0 ) {
        if( strcasecmp( name, record.userid ) == 0 ) {*/

    if (userindex = searchuser(name)) /* modified by dong , 1998.11.2 */
    {
        lseek(fh, (userindex-1) * sizeof(record), SEEK_SET);
        read(fh, &record, sizeof(record));

        strcpy( name, record.userid );
        close( fh );
        return;
        /*}*/
    }
    close( fh );
    printf( ":Err: unknown (3) userid %s\n", name );
    exit( 0 );
}

static post_article( usermail )
int usermail;
{
    struct fileheader   header;
    struct boardheader  brdhdr;
    char        userid[ MAXLEN ], subject[ MAXLEN ];
    char        index[ MAXLEN ], name[ MAXLEN ], article[ MAXLEN ];
    char        buf[ MAXLEN ], *ptr;
    FILE        *fidx;
    int         fh;
    time_t      now;
    char        letter[MAXLEN];
    char        WWW_DEL[256] = "X";
    int         Xpost = 0;
    char        Xid[24];
    struct fileheader   del_header;
    char BoardName[32];

    /* Leeward: 97.12.23: for 3W modify article */
    ptr = strstr(homepath, ".MM.");
    if (ptr)
    {
        EMode = 'M';
        *ptr = 0;
        strcpy(name, ptr + 2);
    }
    else
    {
        ptr = strstr(homepath, "XLetterX");
        if (ptr)
            EMode = 'L';
        else
        {
            ptr = strstr(homepath, ".XM.");
            if (ptr)
            {
                EMode = 'X';
                *ptr = 0;
                strcpy(name, ptr + 2);
            }
        }
    }

    if ('X' == EMode)
    {
        Xpost = 1;
        /* ±£´æ±»É¾ÎÄÕÂËùÔÚ°æ added by dong, 1998.9.28 */
        ptr = strrchr( homepath, '/' );
        (ptr == NULL) ? (ptr = homepath) : (ptr++);
        strcpy(BoardName, ptr);

        sprintf(WWW_DEL, "%s/%s", homepath, name );
        ptr = strrchr(homepath, '/');
        strcpy(ptr + 1, "junk");
        EMode = ' ';
    }

    sprintf( index, "%s/.DIR", homepath );

    if ('L' != EMode)
    {
        if( (fidx = fopen( index, "r" )) == NULL ) {
            if( (fidx = fopen( index, "w" )) == NULL ) {
                printf( ":Err: Unable to post in %s.\n", homepath );
                return;
            }
        }
        fclose( fidx );
    }
    if (NULL!=fgets(userid, MAXLEN, stdin))
	userid[strlen(userid)-1]=0;    

    if (NULL!=fgets(subject, MAXLEN, stdin))
	subject[strlen(subject)-1]=0;


    if ('L' == EMode)
    {
        FILE *fpMail;


        check_userec( &alluser, userid );

        if (NULL!=fgets(letter, MAXLEN, stdin ))
		letter[strlen(letter)-1]=0;
        Xuserec2(letter); /* Leeward 98.10.01 Fix a bug */

        if (!HAS_PERM(currentuser,PERM_SYSOP) && !HAS_PERM(currentuser,PERM_LOGINOK))
        { /* Leeward 98.07.26 ¼ÓÈëÈ¨ÏÞÅÐ¶Ï */
            printf(":Err: User %s try to send letter while not registered .\n", userid);
            return;
        }

        if( HAS_PERM(currentuser,PERM_DENYMAIL))
        { /* Bigman: 2000.9.5 ·â½ûMail ²»ÄÜ·¢ÐÅ */
            printf(":Err: Deny_mail User %s try to mail in (%s) .\n", userid, homepath);
            return;
        }

        now = time( NULL );
        sprintf(name, "M.%d.A", now );
        sprintf(homepath, "mail/%c/%s/%s", toupper(letter[0]), letter, name);

        fpMail = fopen(homepath, "wt");
        if (NULL == fpMail)
        {
            printf(":Err: Cannot open file %s!\n", name);
            return;
        }

        fprintf(fpMail, "¼ÄÐÅÈË: %s (%s) [WWW MAIL]\n±ê  Ìâ: %s\n·¢ÐÅÕ¾: BBS " NAME_BBS_CHINESE "Õ¾ (%24.24s)\nÀ´  Ô´: %s\n\n", userid, currentuser->username, subject, ctime(&now), fromhost);
        while(fgets(buf, MAXLEN, stdin) != NULL )
            fputs(buf, fpMail);
        AddSignature(fpMail, userid, 0); /* Leeward: 98.05.17 */

        fclose(fpMail);

        bzero( (void *)&header, sizeof( header ) );
        strcpy( header.filename, name );
        strncpy( header.owner, userid, IDLEN );
        strncpy( header.title, subject, STRLEN );
        sprintf(homepath, "mail/%c/%s/.DIR", toupper(letter[0]), letter);

        fpMail = fopen(homepath, "ab");
        if (NULL == fpMail)
        {
            printf(":Err: Cannot open file %s!\n", name);
            return;
        }

        fwrite((void *)&header, sizeof(header), 1, fpMail);
        fclose(fpMail);

        return;
    }

    if( usermail ) {
        check_userec( &alluser, userid );
        strcpy(Xid, userid);
        ptr = strrchr( homepath, '/' );
        (ptr == NULL) ? (ptr = homepath) : (ptr++);
/*        search_record(BOARDS, &brdhdr, sizeof(brdhdr), cmpbnames, ptr);*/
        if ( haspostperm(currentuser,ptr) ) /* Leeward 98.09.28 Fix bugs */
        {
            if( !HAS_PERM(currentuser,PERM_SYSOP) && (!HAS_PERM(currentuser,PERM_POST)
                                      || !HAS_PERM(currentuser,brdhdr.level)
                                      || HAS_PERM(currentuser,PERM_DENYPOST)) )
            { /* Leeward 98.05.02
                  ¼ÓÈë PERM_DENYPOST µÄÅÐ¶Ï£¬Ïû³ýÏÂÁÐ BUG:
                     ±»·âPOSTºó²»ÓÃtelnetµÇÂ¼Ôò PERM(PERM_POST) ÈÔÎªÕæ£¬ÈÔ¿É WWW-POST)
                  ¼ÓÈë PERM_SYSOP µÄÅÐ¶Ï£¬Ïû³ýÏÂÁÐ BUG:
                     ¾ßÓÐSYSOPÈ¨ÏÞÕß²»ÄÜÔÚÆÕÍ¨ÓÃ»§¡°Ö»¶Á¡±µÄ°æ WWW-POST£¨Èç Announce£©
                  */
                printf(":Err: User %s try to post article in a POSTMASK(%s) board.\n", userid, homepath);
                return;
            }
        }

    }

    now = time( NULL );
    if (' ' == EMode && !Xpost) /* Leeward 97.12.23 */
        sprintf( name, "M.%d.A", now );
    else
    { /* Check if the file "name" belongs to the user "userid"! */
        FILE *chk;
        int  ok = 0;
        char fuserid[24] = "WWW-DEL-ERROR";

        if (Xpost)
            strcpy(article, WWW_DEL);
        else
            sprintf( article, "%s/%s", homepath, name );
        chk = fopen(article, "rt");
        if (NULL == chk)
        {
            printf(":Err: Cannot open file %s!\n", article);
            return;
        }
        while (!feof(chk))
        {
            fgets(article, 128, chk);
            if (feof(chk))
                break;
            if (strstr(article, "·¢ÐÅÈË: ") && strstr(article, "), ÐÅÇø: "))
            {
                sscanf(article, "·¢ÐÅÈË: %24s", fuserid);
                if (!strcmp(fuserid, userid))
                {
                    ok = 1;
                    break;
                }
            }
        }
        fclose(chk);

        if (0 == ok)
        {
            char currBM[BM_LEN];
            FILE *temp_inf ;
            search_record(BOARDS, &brdhdr, sizeof(brdhdr), cmpbnames, BoardName);
            /* just for debug
                    temp_inf = fopen("/home0/bbs/dong/test.bbslog", "wr");
                    fprintf(temp_inf, "BoardName = %s", BoardName);
                    fprintf(temp_inf, "homepath = %s\n", homepath);
                    fprintf(temp_inf, "WWW_DEL = %s\n", WWW_DEL);
                    fprintf(temp_inf, "username = %s\n", record.userid);
                    fclose(temp_inf);
            */
            memcpy( currBM, brdhdr.BM, BM_LEN -1);
            if (CheckBM(currBM,currentuser))/* ÐÞ¸ÄÈ¨ÏÞ¼ì²é,ÔÊÐí°æÖ÷É¾ÎÄ modified by dong, 1998.9.28 */
            {
                ptr = strrchr(homepath, '/');
                strcpy(ptr + 1, "deleted");
                sprintf(index, "%s/.DIR", homepath );
                strcpy(userid, fuserid);
            }
            else
            {
                printf(":Err: You can't modify other's articles!\n");
                return;
            }
        }
    }

    ptr = strrchr( name, 'A' );
    while( 1 ) {
        sprintf( article, "%s/%s", homepath, name );
        if ('M' == EMode)
            unlink(article);
        fh = open( article, O_CREAT | O_EXCL | O_WRONLY, 0644 );
        if( fh != -1 )
            break;
        if( *ptr < 'Z' )  (*ptr)++;
        else  ptr++, *ptr = 'A', ptr[1] = '\0';
    }

    /*    printf( "post to %s\n", article ); */
    if( usermail ) {
        ptr = strrchr( homepath, '/' );
        (ptr == NULL) ? (ptr = homepath) : (ptr++);
        if (Xpost)
        {
            /*sprintf( buf, "%24.24s \033[1m\033[32mWWW-DEL\033[0m\033[0m by %s\n\n", ctime( &now ), Xid); Leeward 98.05.20: »¹ÊÇ²»Ð´ÕâÒ»ÐÐ£¬·ñÔò undelete ÎÄÕÂµÄÊ±ºò²»ºÃ¿´ºÕºÕ */
            /* Add 45 space characters first in below ! */
            strcat(subject, "                                            ");
            sprintf(subject + 45 - strlen(Xid) - 3, " - %s", Xid);
        }
        else
        { /*now+=28800;Haohmaru.99.4.21.²»ÖªµÀÎªÊ²Ã´WWWµÄÊ±ÖÓ±ÈÏµÍ³Ê±ÖÓÂý8Ð¡Ê±*/
            sprintf( buf, "·¢ÐÅÈË: %s (%s), ÐÅÇø: %s\n±ê  Ìâ: %s\n·¢ÐÅÕ¾: BBS "NAME_BBS_CHINESE"Õ¾ (%24.24s) \033[1m\033[32mWWW-POST\033[0m\033[0m\n\n", userid, currentuser->username, ptr, subject, ctime( &now ) );
        }
        write( fh, buf, strlen( buf ) );
    }
    while( fgets( buf, MAXLEN, stdin ) != NULL ) {
        /* Leeward 98.07.30 Ö§³Ö×Ô¶¯»»ÐÐ */
        char *ppt = buf; /* ÕÛÐÐ´¦ */
        char *pp  = ppt; /* ÐÐÊ× */
        unsigned int  LLL = 78; /* ÕÛÐÐÎ»ÖÃ */
        char *ppx, cc;
        int  ich, lll;

        while (strlen(pp) > LLL) {
            lll = 0; ppx = pp; ich = 0;
            do {
                if (ppx = strstr(ppx, "[")) {
                    ich = strchr(ppx, 'm') - ppx;
                    if (ich > 0) ich ++; else ich = 2;
                    lll += ich; ppx += 2; ich = 0;
                }
            } while (ppx);
            ppt += LLL + lll;

            if (*pp & 0x80) { /* ±ÜÃâÔÚºº×ÖÖÐ¼äÕÛÐÐ */
                for (ppx = ppt - 1, ich = 0; ppx >= pp; ppx --)
                        if (!(*ppx & 0x80)) break; else ich ++;
                if (ich % 2) ppt --;
            }
            else if (*ppt) {
                for (ppx = ppt - 1, ich = 0; ppx >= pp; ppx --)
                        if ((*ppx & 0x80) || ' ' == *ppx) break; else ich ++;
                if (ppx > pp && ich < 16) ppt -= ich;
            }

            cc = *ppt; *ppt = 0;
            if (':' == buf[0] && ':' != *pp) write(fh, ": ", 2);
            write(fh, pp, strlen(pp));
            if (cc) write(fh, "\n", 1);
            *ppt = cc; pp = ppt;
        }
        if (':' == buf[0] && ':' != *pp) write(fh, ": ", 2);
        write(fh, pp, strlen(pp)); /*write(fh, "\n", 1);*/
    }
    if(usermail){
        if(!Xpost)  AddSignature(NULL, userid, fh); /* Leeward: 98.05.17 */

        if ('M' == EMode)
        { /* Leeward 97.12.23 */
            sprintf(buf,"\n[1m¡ù ÐÞ¸Ä:¡¤%s ì¶ %15.15s ÐÞ¸Ä±¾ÎÄ¡¤[FROM: %.22s] [m", userid, ctime( &now ) + 4, emailad);
            write( fh, buf, strlen( buf ) );
        }

        sprintf(buf,"\n[1m¡ù À´Ô´:¡¤BBS Ë®Ä¾Çå»ªÕ¾ "NAME_BBS_ENGLISH"¡¤[FROM: %.22s] [m\n", emailad);
        if (!Xpost) write( fh, buf, strlen( buf ) );
    }
    close(fh);

    bzero( (void *)&header, sizeof( header ) );
    strcpy( header.filename, name );
    strncpy( header.owner, userid, IDLEN );
    strncpy( header.title, subject, STRLEN );
    if( ! usermail ) {
        header.filename[ STRLEN - 1 ] = 'M';
    }
    if (' ' == EMode) /* Leeward 97.12.23 */
    { /* Leeward 98.04.27 copied from ../edit.c and modified */
        /*char *ptr; This is already defined before */
        /*time_t now; This is already defined before */
        struct
        {
            char author[IDLEN + 1];
            char board[IDLEN + 6];
            char title[66];
            time_t date;
            int number;
        } postlog, pl;

        append_record( index, &header, sizeof( header ) );

        if (Xpost)
        {
            FILE *DIR, *DEL;
            char DIRDEL[256], fn[256], fN[256];
            char CMD[1024];

            strcpy(fn, WWW_DEL);
            strcpy(fN, strrchr(fn, '/') + 1);
            strcpy(strrchr(fn, '/'), "/.DIR");
            sprintf(DIRDEL, "%s.X", fn);
            DIR = fopen(fn, "r");
            DEL = fopen(DIRDEL, "w");
            if (!DIR || !DEL)
            {
                if (DIR) fclose(DIR);
                if (DEL) fclose(DEL);
            }
            else
            {
                sprintf(CMD, "/bin/mv %s %s", DIRDEL, fn);
                while(!feof(DIR))
                {
                    fread(&del_header, sizeof(struct fileheader), 1, DIR);
                    if (feof(DIR))
                        break;
                    else if (strcmp(del_header.filename, fN))
                        fwrite(&del_header, sizeof(struct fileheader), 1, DEL);
                }
                fclose(DIR);
                fclose(DEL);
                system(CMD);
                unlink(WWW_DEL);
                ptr = strrchr(WWW_DEL, '/') - 5;
                if(strncmp(ptr, "/test", 5))
                    Xuserec(currentuser, userid, -1); /* Decrease article number */
            }
        }

        /*if(junkboard()||normal_board(currboard)!=1||!strcmp(currboard,"blame"))
            return ; WWW post program has checked board name and permissions */
        if (!strcmp(strrchr(homepath, '/'), "/test")
                || !strcmp(strrchr(homepath, '/'), "/junk")
                || !strcmp(strrchr(homepath, '/'), "/deleted"))
            return; /* But ...hmm */
        /*now = time(0) ; This is already executed before */
        strcpy(postlog.author, header.owner/*currentuser->userid*/);
        strcpy(postlog.board, 1 + strrchr(homepath, '/')/*currboard*/);
        ptr = header.title/*save_title*/;
        if (!strncmp(ptr, "Re: ", 4))
            ptr += 4;
        strncpy(postlog.title, ptr, 65);
        postlog.date = now;
        postlog.number = 1;

        { /* added by Leeward 98.04.25 */
            char buf[STRLEN];
            int  log = 1;
            FILE *fp = fopen(".Xpost", "r");

            if (fp)
            {
                while (!feof(fp))
                {
                    fread(&pl, sizeof(pl), 1, fp);
                    if (feof(fp)) break;

                    if (!strcmp(pl.title, postlog.title)
                            && !strcmp(pl.author, postlog.author)
                            && !strcmp(pl.board, postlog.board))
                    {
                        log = 0;
                        break;
                    }
                }
                fclose(fp);
            }

            if (log)
            {
                append_record(".Xpost", &postlog, sizeof(postlog));
                append_record(".post", &postlog, sizeof(postlog));
            }
        }

        append_record(".post.X", &postlog, sizeof(postlog));

        Xuserec( currentuser, userid, 1); /* Increase article number */
    }
}

cancel_article( board, file )
char    *board, *file;
{
    struct fileheader   header;
    struct stat         state;
    char        dirname[ MAXLEN ];
    char        buf[ MAXLEN ];
    long        numents, size, time, now;
    int         fd, lower, ent;

    if( file == NULL || file[0] != 'M' || file[1] != '.' ||
            (time = atoi( file+2 )) <= 0 )
        return;
    size = sizeof( header );
    sprintf( dirname, "%s/boards/%s/.DIR", homepath, board );
    if( (fd = open( dirname, O_RDWR )) == -1 )
        return;
    flock( fd, LOCK_EX );
    fstat( fd, &state );
    ent = ((long)state.st_size) / size;
    lower = 0;
    while( 0/*1*/ ) { /* Leeward: 97.12.23: disable this block */
        ent -= 8;
        if( ent <= 0 || lower >= 2 )
            break;
        lseek( fd, size * ent, SEEK_SET );
        if( read( fd, &header, size ) != size ) {
            ent = 0;
            break;
        }
        now = atoi( header.filename + 2 );
        lower = (now < time) ? lower + 1 : 0;
    }
    if( ent < 0 )  ent = 0;
    while( read( fd, &header, size ) == size ) {
        if( strcmp( file, header.filename ) == 0 ) {
            sprintf( buf, "-%s", header.owner );
            strcpy( header.owner, buf );
            strcpy( header.title, "<< article canceled >>" );
            lseek( fd, -size, SEEK_CUR );
            safewrite( fd, &header, size );
            break;
        }
        now = atoi( header.filename + 2 );
        if( now > time )
            break;
    }
    flock( fd, LOCK_UN );
    close( fd );
}

expire_article( brdname, days_str, maxpost, minpost )
char    *brdname, *days_str;
int	maxpost, minpost;
{
    struct fileheader   head;
    struct stat         state;
    char        lockfile[ MAXLEN ], index[ MAXLEN ];
    char        tmpfile[ MAXLEN ], delfile[ MAXLEN ];
    int         days, total;
    int         fd, fdr, fdw, done, keep;
    int         duetime, ftime;

    days = atoi( days_str );
    if( days < 1 ) {
        printf( ":Err: expire time must more than 1 day.\n" );
        return;
    } else if( maxpost < 100 ) {
        printf( ":Err: maxmum posts number must more than 100.\n" );
        return;
    }
    sprintf( lockfile, "%s/.dellock", homepath, brdname );
    sprintf( index, "%s/boards/%s/.DIR", homepath, brdname );
    sprintf( tmpfile, "%s/boards/.tmpfile", homepath, brdname );
    sprintf( delfile, "%s/boards/.deleted", homepath, brdname );

    if( (fd = open( lockfile, O_RDWR | O_CREAT | O_APPEND, 0644 )) == -1 )
        return;
    flock( fd, LOCK_EX );
    unlink( tmpfile );

    duetime = time( NULL ) - days * 24*60*60;
    done = 0;
    if( (fdr = open( index, O_RDONLY, 0 )) > 0 ) {
        fstat( fdr, &state );
        total = state.st_size / sizeof( head );
        if( (fdw = open( tmpfile, O_WRONLY|O_CREAT|O_EXCL, 0644 )) > 0 ) {
            while( read( fdr, &head, sizeof head ) == sizeof head ) {
                done = 1;
                ftime = atoi( head.filename + 2 );
                if( head.owner[0] == '-' )
                    keep = 0;
                else if( head.accessed[0] & FILE_MARKED || total <= minpost )
                    keep = 1;
                else if( ftime < duetime || total > maxpost )
                    keep = 0;
                else
                    keep = 1;
                if( keep ) {
                    if( safewrite( fdw, &head, sizeof head ) == -1 ) {
                        done = 0;
                        break;
                    }
                } else {
                    printf( "Unlink %s\n", head.filename );
                    if( head.owner[0] == '-' )
                        printf( "Unlink %s.cancel\n", head.filename );
                    total --;
                }
            }
            close( fdw );
        }
        close( fdr );
    }
    if( done ) {
        unlink( delfile );
        if( rename( index, delfile ) != -1 ) {
            rename( tmpfile, index );
        }
    }
    flock( fd, LOCK_UN );
    close( fd );
}

main( argc, argv )
int argc;
char    *argv[];
{
    char        *progmode;
    int         max, min;

    currentuser = & alluser;
    if( argc < 3 )  usage();
    progmode = argv[1];
    homepath = argv[2];

    resolve_ucache();/* modified by dong , 1998.11.2 */
    bzero(currentuser,sizeof(struct userec));

    /* Leeward: 97.12.20: ÈÃ 3W ·¢ÎÄÄÜÓ°ÏìÉúÃüÁ¦ */
    if (argc < 5)/*Haohmaru.99.11.24.Ô­À´Îª4*/
        strcpy(fromhost, "WWW");
    else
        strncpy(fromhost, argv[4], 15);/*Haohmaru.99.11.24.Ô­À´Îª[3]*/
    fromhost[15] = 0;

    sign = atoi(argv[3]);/*Haohmaru.99.11.24.µÚsign¸öÇ©Ãûµµ*/
    if(sign<0 || sign>5)
        sign = 1;
    if( strcasecmp( progmode, "list" ) == 0 ) {
        search_boards( 0 );
    } else if( strcasecmp( progmode, "visit" ) == 0 ) {
        search_boards( 1 );
    } else if( strcasecmp( progmode, "post" ) == 0 ) {
        post_article( 0 );
    } else if( strcasecmp( progmode, "mail" ) == 0 ) {
        post_article( 1 );
    } else if( strcasecmp( progmode, "cancel" ) == 0 ) {
        if( argc < 5 )  usage();
        cancel_article( argv[3], argv[4] );
    } else if( strcasecmp( progmode, "expire" ) == 0 ) {
        if( argc < 5 )  usage();
        max = atoi( argc > 5 ? argv[5] : "9999" );
        min = atoi( argc > 6 ? argv[6] : "10" );
        expire_article( argv[3], argv[4], max, min );
    }
}

