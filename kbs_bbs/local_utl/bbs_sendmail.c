/*
 *  Source Directory: ~bbs_src/local_util/bbs_sendmail.c
 *  Binary Directory: ~bbs/bin/bbs_sendmail
 *
 *  It is written for wrapping the NAME environment. 
 *
 *  To make the program take effect, the MAILER environment should
 *  be set for tin.sh.  ==> setenv MAILER /bin/bbs_sendmail
 *
 *  					-- jj  (Apr 3, 1994)
 */

#include <stdio.h>
#include <stdlib.h>
#define DOMAIN "csie.nctu.edu.tw"

FILE *fp;
char *user, *o_user;
char buf[ 256 ], name_buf[ 256 ];
char name_env[ 256 ];

main(argc, argv)
int argc;
char *argv[];
{
    int i, from_flag = 0;
 
    user = getenv("USER");
    if ( user == NULL ) user = "Unknown";

    strcpy( name_buf, user );
    o_user = (char *)strtok( name_buf, " .\n\r");

    sprintf( name_env, "NAME=%s@%s", user, DOMAIN ); 
    putenv( name_env );

    sprintf( buf, "/usr/lib/sendmail -f %s.bbs@%s ", o_user, DOMAIN);

    for (i = 1 ; i < argc; i++) {
        strcat( buf, argv[ i ] );
        strcat( buf, " ");
    }

    if ( (fp = popen( buf, "w" )) == NULL ) {
        exit ( 1 );
    }

    fprintf( fp, "From: %s.bbs@%s\n", o_user, DOMAIN); 
    fprintf( fp, "Reply-To: %s.bbs@%s\n", o_user, DOMAIN); 

    while ( fgets( buf, 255, stdin ) != NULL ) {
#if 0
        if ( !strncmp("From: ",  buf, 6 ) && (from_flag == 0)) {
            from_flag++;
            sprintf( buf, "From: %s.bbs@%s\n", o_user, DOMAIN); 
            sprintf( buf, "Reply-To: %s.bbs@%s\n", o_user, DOMAIN); 
        } 
#endif 0
        fputs( buf, fp );    
    }

    pclose( fp );
    return ( 0 );
}

