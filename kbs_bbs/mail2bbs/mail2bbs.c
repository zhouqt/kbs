#include <bbs.h>
#define MAILDIR  BBSHOME"/mail"
#define MYPASSFILE BBSHOME"/.PASSWDS"
#define DOMAIN_NAME "bbs.zixia.net"
/*
report()
{
}
*/
struct userec checkuser ;

cmpuids(uid,up)
char *uid ;
struct userec *up ;
{
    if (!strncasecmp(uid, up->userid, sizeof(up->userid))) {
        strncpy( uid, up->userid, sizeof( up->userid ));
        return 1;
    } else {
        return 0;
    } 
}
int
dashd( fname )
char    *fname;
{
    struct stat st;

    return ( stat( fname, &st ) == 0 && S_ISDIR( st.st_mode ) );
}

int
dosearchuser(userid)
char *userid ;
{
    return search_record(MYPASSFILE, &checkuser, sizeof(currentuser),
        cmpuids, userid) ;
}

append_mail( fin, sender1, sender, userid, title)
FILE *fin;
char *userid, *sender1, *sender, *title ;
{
        struct fileheader newmessage ;

        char fname[ 512 ], buf[ 256 ], genbuf[ 256 ] ,fff[80],fff2[80],mailaddr[80];
        char *ip ,maildir[256];
        struct stat st ;
        int fp ;
        FILE *fout,*dp,*rmail;
        int yyyy, zzzz,passcheck=0;
        
/* check if the userid is in our bbs now */
        if (!dosearchuser(userid)) return -1 ;

/* check for the mail dir for the userid */
        sprintf(genbuf, "%s/%c/%s", MAILDIR, toupper(userid[0]),userid) ;

        if(stat(genbuf,&st) == -1) {
                if(mkdir(genbuf,0755) == -1) 
                  return -1 ;
        } else {        
                if(!(st.st_mode & S_IFDIR))
                  return -1 ;
        }

        printf("Ok, dir is %s\n", genbuf );
        
/* added by netty */        
/*if ((userid[0] == 'n') && (userid[1] == 'e') && (userid[2] == 't') &&
     (userid[3] == 't') && (userid[4] == 'y') 
      && (userid[5] == '\0') ) {*/
if(!strcmp(userid,"SYSOP")&&strstr(title,"mail check"))
{
      passcheck=1;
      if ((!strstr(sender, ".bbs@"))/* && (!strstr(sender, "@bbs.")) */&&
          (strchr( title, '@' ))) { 

          yyyy = 0;
          zzzz = 0;
          while ((title[ yyyy ] != '@') && (yyyy < NAMELEN))
               yyyy = yyyy + 1;
          yyyy = yyyy + 1;     
          while ((title[ yyyy ] != '@') && (yyyy < NAMELEN)) {
               sender1[ zzzz ] = title[ yyyy ];
               yyyy = yyyy + 1;
               zzzz = zzzz + 1;
          } 
          sender1[ zzzz ] = '\0';
          strcpy(userid,sender1);
          sprintf(fff,"%s/home/%c/%s/mailcheck",BBSHOME,toupper(sender1[0]),sender1);
      if((dp=fopen(fff,"r"))!=NULL)
      {
             printf("open mailcheck\n");
             fgets(fff2,sizeof(fff2),dp);
             fclose(dp);
             sprintf(fff2,"%9.9s",fff2);
      if (dosearchuser(sender1)&&strstr(title,fff2)/*&&strstr(sender,checkuser.email)*/) {
             printf("pass1\n");

                unlink(fff);
                passcheck=5;
                /*Modify for SmallPig*/
                sprintf( genbuf, "%s", sender/*checkuser.email*/);
                sprintf( buf, "%s/tmp/email_%s", BBSHOME,sender1 );
                     if( (fout = fopen( buf, "w" )) != NULL ) {
                          fprintf( fout, "%s\n", genbuf );
                          fclose( fout );
/*                          return 0; */
                }
          }
        }
      }
}  

/* allocate a record for the new mail */
        bzero( &newmessage, sizeof(newmessage)) ;
        sprintf(fname, "M.%d.A", time(0)) ;
        sprintf(genbuf, "%s/%c/%s/%s", MAILDIR, toupper(userid[0]),userid, fname) ;
        sprintf(maildir, "%s/%c/%s", MAILDIR, toupper(userid[0]),userid) ;
        if(!dashd(maildir))
        {
                mkdir( maildir, 0755 );
                chmod( maildir, 0755 );
        }
       
        ip = (char *) rindex(fname,'A') ;
        while((fp = open(genbuf,O_CREAT|O_EXCL|O_WRONLY,0644)) == -1) {
                if(*ip == 'Z')
                  ip++,*ip = 'A', *(ip + 1) = '\0' ;
                else
                  (*ip)++ ;
                sprintf(genbuf,"%s/%c/%s/%s",MAILDIR,toupper(userid[0]),userid,fname) ;
        }
        close(fp) ;
        strcpy(newmessage.filename, fname) ;
        strncpy(newmessage.title, title, STRLEN) ;
        strncpy(newmessage.owner, sender, STRLEN) ;

        printf("Ok, the file is %s\n", genbuf );

/* copy the stdin to the specified file */
        sprintf(genbuf,"%s/%c/%s/%s", MAILDIR,toupper(userid[0]), userid, fname) ;
        if ( (fout = fopen( genbuf, "w" )) == NULL) {
            printf("Cannot open %s \n", genbuf );
            return -1;
        } else {
            time_t tmp_time;
            time( &tmp_time );
/*          fprintf( fout, "To:        @%s@firebird \n", userid ); */
            fprintf( fout, "寄信人: %s \n", sender );
            fprintf( fout, "標  題: %s\n", title );
            fprintf( fout, "發信站: 中正大學四百年來第一站 BBS 信差\n");
            fprintf( fout, "日  期: %s\n", ctime( &tmp_time ) );
            if(passcheck>=1)
            {
                fprintf(fout,"親愛的%s:\n",sender1);
                sprintf(maildir,"%s/etc/%s",BBSHOME,(passcheck==5)?"smail":"fmail");
                if((rmail=fopen(maildir,"r"))!=NULL)
                {
                     while (fgets( genbuf, 255, rmail ) != NULL) 
                     fputs( genbuf, fout );
                     fclose(rmail);
                }
            }else{
        
            while (fgets( genbuf, 255, fin ) != NULL) {
                fputs( genbuf, fout );
            }}
            fclose( fout );
        
        }

/* append the record to the MAIL control file */
        sprintf(genbuf, "%s/%c/%s/%s", MAILDIR,toupper(userid[0]), userid,  DOT_DIR) ;
        if( append_record( genbuf, &newmessage, sizeof(newmessage)) == -1)
             return 1 ;
        else return 0;
}


main(argc, argv)
int argc;
char *argv[];
{
    
    char sender[ 256 ];
    char username[ 256 ];
    char receiver[ 256 ];
    char nettyp[ 256 ];
    int  xxxx;
     
    /* argv[ 1 ] is original sender */
    /* argv[ 2 ] is userid in bbs   */
    /* argv[ 3 ] is the mail title  */
    if (argc != 4) {
        char *p = (char *) rindex( argv[ 0 ], '/' );

        printf("Usage: %s sender receiver_in_bbs mail_title\n",
             p ? p+1 : argv[ 0 ]);
        return 1;
    }
 if (chroot(BBSHOME) == 0) {
        chdir("/");
#ifdef DEBUG
        printf("Chroot ok!\n");
#endif
    } else {
        /* assume it is in chrooted in bbs */
        /* if it is not the case, append_main() will handle it */
        chdir("/");
        printf("Already chroot\n");
    }

    setreuid( BBSUID, BBSUID );
    setregid( BBSGID, BBSGID );

    if (strchr( argv[ 1 ], '@' )) {
        strcpy(sender, argv[ 1 ]);
        /*  added by netty  */
        xxxx = 0;
        while (sender[ xxxx ] != '@') {
           nettyp[ xxxx ] = sender[ xxxx ];
           xxxx = xxxx + 1;
       } 
       nettyp[ xxxx ] = '\0';   /* added by netty  */
    } else {
        char  *p, *l, *r;
        char buf[ 256 ];
        strcpy( buf, argv[ 1 ] ); 
        p = strtok( buf , " \t\n\r" );
        l = strchr( argv[ 1 ], '(');
        r = strchr( argv[ 1 ], ')');
        if (l < r && l && r ) strncpy( username, l, r - l + 1 ); 
        sprintf(sender, "%s@%s %s", p, DOMAIN_NAME, username );
        strcpy( nettyp, p ); 
    }

    strcpy( receiver, argv[ 2 ] );
    return append_mail( stdin, nettyp, sender, receiver, argv[ 3 ]);
}


