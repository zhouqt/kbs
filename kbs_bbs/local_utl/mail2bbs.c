#include "bbs.h"
#define MAILDIR  "/mail/"
#define MYPASSFILE "/.PASSWDS"
#define DOMAIN_NAME "cs.ccu.edu.tw"
#define BBSHOME		"/home/bbs"
#define BBSGID		99
#define BBSUID		9999

report()
{
}

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
dosearchuser(userid)
char *userid ;
{
    struct userec auser;
    return search_record(MYPASSFILE, &auser, sizeof(currentuser),
        cmpuids, userid) ;
}

append_mail( fin, sender, userid, title)
FILE *fin;
char *userid, *sender, *title ;
{
	struct fileheader newmessage ;

	char fname[ 512 ], buf[ 256 ], genbuf[ 256 ] ;
	char *ip ;
	struct stat st ;
	int fp ;
	FILE *fout;

/* check if the userid is in our bbs now */
	if (!dosearchuser(userid)) return -1 ;

/* check for the mail dir for the userid */
	sprintf(genbuf, "%s/%s", MAILDIR, userid) ;

	if(stat(genbuf,&st) == -1) {
		if(mkdir(genbuf,0755) == -1) 
		  return -1 ;
	} else {	
		if(!(st.st_mode & S_IFDIR))
		  return -1 ;
	}

        printf("Ok, dir is %s\n", genbuf );

/* allocate a record for the new mail */
	bzero( &newmessage, sizeof(newmessage)) ;
	sprintf(fname, "M.%d.A", time(0)) ;
	sprintf(genbuf, "%s/%s/%s", MAILDIR, userid, fname) ;
	ip = (char *) rindex(fname,'A') ;
	while((fp = open(genbuf,O_CREAT|O_EXCL|O_WRONLY,0644)) == -1) {
		if(*ip == 'Z')
		  ip++,*ip = 'A', *(ip + 1) = '\0' ;
		else
		  (*ip)++ ;
		sprintf(genbuf,"mail/%s/%s",userid,fname) ;
	}
	close(fp) ;
	strcpy(newmessage.filename, fname) ;
	strncpy(newmessage.title, title, STRLEN) ;
	strncpy(newmessage.owner, sender, STRLEN) ;

        printf("Ok, the file is %s\n", genbuf );

/* copy the stdin to the specified file */
	sprintf(genbuf,"%s/%s/%s", MAILDIR, userid, fname) ;
        if ( (fout = fopen( genbuf, "w" )) == NULL) {
            printf("Cannot open %s \n", genbuf );
            return -1;
        } else {
            time_t tmp_time;
            time( &tmp_time );
            fprintf( fout, "From:      %s via BBS mail gateway\n", sender );
            fprintf( fout, "Title:     %s\n", title );
            fprintf( fout, "Date:      %s\n", ctime( &tmp_time ) );

            while (fgets( genbuf, 255, fin ) != NULL) {
                fputs( genbuf, fout );
            }
            fclose( fout );
        }

/* append the record to the MAIL control file */
	sprintf(genbuf, "%s/%s/%s", MAILDIR, userid,  DOT_DIR) ;
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
    } else {
        char *p, *l, *r;
        char buf[ 256 ];
        strcpy( buf, argv[ 1 ] ); 
        p = strtok( buf , " \t\n\r" );
        l = strchr( argv[ 1 ], '(');
        r = strchr( argv[ 1 ], ')');
        if (l < r && l && r ) strncpy( username, l, r - l + 1 ); 
        sprintf(sender, "%s@%s %s", p, DOMAIN_NAME, username );
    }

    strcpy( receiver, argv[ 2 ] );
    return append_mail( stdin, sender, receiver, argv[ 3 ]);
}


