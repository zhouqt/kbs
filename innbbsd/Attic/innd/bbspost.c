/*
 * File: bbspost.c
 *
 * A part of innbbsd, SEEDNetBBS v1 series
 * $Id$
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

#undef BBSHOST				/* prevent redefined */   
#include "bbs.h"
#include "../innbbsconf.h"
#define INNDHOME BBSHOME"/innd"

#define MAXLEN          1024

char            *crypt();
char            *homepath;
int             visitflag;
char        realfrom[ MAXLEN ];

usage()
{
    printf( ProgramUsage );
    exit( 0 );
}

report()
{
        /* Function called from record.o */
        /* Please leave this function empty */
}

int
ci_strcmp( s1, s2 )
register char   *s1, *s2;
{
    char        c1, c2;

    while( 1 ) {
        c1 = *s1++;
        c2 = *s2++;
        if( c1 >= 'a' && c1 <= 'z' )
            c1 += 'A' - 'a';
        if( c2 >= 'a' && c2 <= 'z' )
            c2 += 'A' - 'a';
        if( c1 != c2 )
            return (c1 - c2);
        if( c1 == 0 )
            return 0;
    }
}

void search_article( brdname )
char    *brdname;
{
    struct fileheader   head;
    struct stat         state;
    char        index[ MAXLEN ], article[ MAXLEN ];
    int         fd, num, offset, type;
    char        send;

    offset = (int) &(head.innflag[ 1 ]) - (int) &head;
    sprintf( index, "%s/boards/%s/.DIR", homepath, brdname );
    if( (fd = open( index, O_RDWR )) < 0 ) {
        return;
    }
    fstat( fd, &state );
    num = (state.st_size / sizeof( head )) - 1;
    while( num >= 0 ) {
        lseek( fd, (off_t) (num * sizeof( head )) + offset, 0 );
        if( read( fd, &send, 1 ) > 0 && send == '%' )  break;
        num -= 4;
    }
    num++;
    if( num < 0 )  num = 0;
    lseek( fd, (off_t) (num * sizeof( head )), 0 );
    for( send = '%'; read( fd, &head, sizeof( head )) > 0; num++ ) {
        type = head.innflag[ 1 ];
        if( type != send && visitflag ) {
            lseek( fd, (off_t) (num * sizeof( head )) + offset, 0 );
            safewrite( fd, &send, 1 );
            lseek( fd, (off_t) ((num + 1) * sizeof( head )), 0 );
        }
        if( type == '\0' ) {
            printf( "%s\t%s\t%s\t%s\n", brdname,
                        head.filename, head.owner, head.title );
        }
    }
    close( fd );
}

void search_boards( visit )
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
    char        genbuf[ MAXLEN ];

    fgets(passwd, MAXLEN, stdin);
    passwd[MAXLEN-1] = 0;
    passwd[strlen(passwd)-1] = 0;
    
    pw = crypt( passwd, record->md5passwd );
    if( strcmp( pw, record->passwd ) != 0 ) {
        printf( ":Err: user '%s' password incorrect!!\n", record->userid );
        exit( 0 );
    }
    
    fgets(realfrom, MAXLEN, stdin);
    realfrom[MAXLEN-1] = 0;
    realfrom[strlen(realfrom)-1] = 0;

    if( !strstr( realfrom, "bbs@" ) ) {
        record->termtype[ 15 ] = '\0';
        strncpy( record->termtype+16, realfrom, STRLEN-16 );
    }
    if( !strstr( homepath, "test" ) ) {
        record->numposts++;
    }
}

check_userec( record, name )
struct userec   *record;
char    *name;
{
    int         fh;

    if( (fh = open( ".PASSWDS", O_RDONLY )) == -1 ) {
        printf( ":Err: unable to open .PASSWDS file.\n" );
        exit( 0 );
    }
    while( read( fh, record, sizeof *record ) > 0 ) {
        if( ci_strcmp( name, record->userid ) == 0 ) {
            strcpy( name, record->userid );
            check_password( record );
#if 0	/* edwardc.990623 is it necessary ?? for what ? */            
            lseek( fh, (off_t) (-1 * sizeof *record), SEEK_CUR );
            safewrite( fh, record, sizeof *record );
#endif            
            close( fh );
            return;
        }
    }
    close( fh );
    printf( ":Err: unknown userid %s\n", name );
    exit( 0 );
}

post_article( usermail )
{
    struct userec       record;
    struct fileheader   header;
    char        userid[ MAXLEN ], subject[ MAXLEN ];
    char        index[ MAXLEN ], name[ MAXLEN ], article[ MAXLEN ];
    char        buf[ MAXLEN ], *ptr;
    FILE        *fidx;
    int         fh,islocalpost;
    time_t      now;

    sprintf( index, "%s/.DIR", homepath );
    if( (fidx = fopen( index, "r" )) == NULL ) {
        if( (fidx = fopen( index, "w" )) == NULL ) {
            printf( ":Err: Unable to post in %s.\n", homepath );
            return;
        }
    }
    fclose( fidx );

    fgets(userid, MAXLEN, stdin);
    
    userid[MAXLEN-1] = 0;
    userid[strlen(userid)-1] = 0;
    
    fgets(subject, MAXLEN, stdin);
    
    subject[MAXLEN-1] = 0;
    subject[strlen(subject)-1] = 0;
    
    if( usermail ) {
        check_userec( &record, userid );
    }

    now = time( NULL );
    sprintf( name, "M.%d.A", now );
    ptr = strrchr( name, 'A' );
    while( 1 ) {
        sprintf( article, "%s/%s", homepath, name );
        fh = open( article, O_CREAT | O_EXCL | O_WRONLY, 0644 );
        if( fh != -1 )  break;
        if( *ptr < 'Z' )  (*ptr)++;
        else  ptr++, *ptr = 'A', ptr[1] = '\0';
    }

    printf( "post to %s\n", article );
    if( usermail ) {
        char local[MAXLEN];

        fgets(local, MAXLEN, stdin);
        local[MAXLEN-1] = 0;
        local[strlen(local)-1] = 0;
        
        islocalpost=strcmp(local,"localpost");
        ptr = strrchr( homepath, '/' );
        (ptr == NULL) ? (ptr = homepath) : (ptr++);
        sprintf( buf, "\
·¢ÐÅÈË: %s (%s), ÐÅÇø: %s\n\
±ê  Ìâ: %s\n\
·¢ÐÅÕ¾: %s (%24.24s), %s\n\n",
            userid, record.username, ptr, subject, BBSNAME, ctime( &now )  ,islocalpost?"×ªÐÅ":"Õ¾ÄÚÐÅ¼þ");
        write( fh, buf, strlen( buf ) );
    }
    while( fgets( buf, MAXLEN, stdin ) != NULL ) {
        write( fh, buf, strlen( buf ) );
    }
    if( usermail ) 
    {
        FILE *foo;
        char outname[80];

        /*Ð´Èë×ªÐÅµµ*/
        if(islocalpost)
        {
          sprintf(outname,"%s/out.bntp",INNDHOME);
          if (foo = fopen(outname, "a"))
          {
            fprintf(foo, "%s\t%s\t%s\t%s\t%s\n", ptr,
              name, userid, record.username, subject);
            fclose(foo);
          }
        }
        realfrom[20]='\0';
        sprintf(buf,"\n--\n[1;36m¡î À´Ô´:¡õ%s %s¡õ[FROM: %s]\n",BBSNAME, BBSHOST, realfrom);
        write( fh, buf, strlen( buf ) );
    }
    close( fh );

    bzero( (void *)&header, sizeof( header ) );
    strcpy( header.filename, name );
    strncpy( header.owner, userid, IDLEN );
    strncpy( header.title, subject, STRLEN );
    if( ! usermail ) {
        header.innflag[ 1 ] = 'M';
    }
    after_post(NULL, &header, ptr, NULL);
}

cancel_article( board, file, message )
char    *board, *file, *message;
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
    while( 1 ) {
        ent -= 8;
        if( ent <= 0 || lower >= 2 )
            break;
        lseek( fd, (off_t) (size * ent), SEEK_SET );
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
            if (message && *message)
              strcpy( header.title, "<< E-mail post ÎÄÕÂÉóºËÖÐ... >>" );
            else
              strcpy( header.title, "<< article canceled >>" );
            lseek( fd, (off_t) -size, SEEK_CUR );
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
char    *argv[];
{
    char        *progmode;
    int         max, min;

    if( argc < 3 )  usage();
    progmode = argv[1];
    homepath = argv[2];
    if( ci_strcmp( progmode, "list" ) == 0 ) {
        search_boards( 0 );
    } else if( ci_strcmp( progmode, "visit" ) == 0 ) {
        search_boards( 1 );
    } else if( ci_strcmp( progmode, "post" ) == 0 ) {
        post_article( 0 );
    } else if( ci_strcmp( progmode, "mail" ) == 0 ) {
        post_article( 1 );
    } else if( ci_strcmp( progmode, "cancel" ) == 0 ) {
        if( argc < 5 )  usage();
        if ( argc >= 6) 
          cancel_article( argv[3], argv[4], argv[5] );
        else
          cancel_article( argv[3], argv[4] );
    } else if( ci_strcmp( progmode, "expire" ) == 0 ) {
        if( argc < 5 )  usage();
        max = atoi( argc > 5 ? argv[5] : "9999" );
        min = atoi( argc > 6 ? argv[6] : "10" );
        expire_article( argv[3], argv[4], max, min );
    }
}
