/* add userdefine1 to userec */

#include "bbs.h"
#include "stdio.h"

struct userec1 {                 /* Structure used to hold information in */
    char userid[IDLEN + 2];     /* PASSFILE */
    char flags; /*¨°?D?¡À¨º??¡ê???¨ª?¡ê?¡ã?????D¨°??¨¤¨¤¦Ì?*/
    unsigned char title; /*¨®??¡ì??¡Àe*/
    int firstlogin;
    char lasthost[16];
    unsigned int numlogins;
    unsigned int numposts;
#ifdef CONV_PASS
    char passwd[OLDPASSLEN];
	char unused_padding[2];
#endif
    char username[NAMELEN];
    unsigned int club_read_rights[MAXCLUB>>5];
    unsigned int club_write_rights[MAXCLUB>>5];
    unsigned char md5passwd[MD5PASSLEN];
    unsigned userlevel;
    int lastlogin;
    int stay;
    int signature;
    unsigned int userdefine[2];
    int notedate;
    int noteline;
    int notemode;
    int exittime;
	/* ¨¦¨²¨¨?¨ºy?Y¡Áa¨°?¦Ì? userdata ?¨¢11?D */
    unsigned int usedspace;     /* used space of user's mailbox, in bytes */
#ifdef HAVE_USERMONEY
    int money;
    int score;
    char unused[20];
#endif
};

struct userec2 {                 /* Structure used to hold information in */
    char userid[IDLEN + 2];     /* PASSFILE */
    char flags; /*¨°?D?¡À¨º??¡ê???¨ª?¡ê?¡ã?????D¨°??¨¤¨¤¦Ì?*/
    unsigned char title; /*¨®??¡ì??¡Àe*/
    time_t firstlogin;
    char lasthost[16];
    unsigned int numlogins;
    unsigned int numposts;
#ifdef CONV_PASS
    char passwd[OLDPASSLEN];
	char unused_padding[2];
#endif
    char username[NAMELEN];
    unsigned int club_read_rights[MAXCLUB>>5];
    unsigned int club_write_rights[MAXCLUB>>5];
    unsigned char md5passwd[MD5PASSLEN];
    time_t lastlogin;
    time_t stay;
    unsigned userlevel;
    int signature;
    unsigned int userdefine[2];
    time_t notedate;
    int noteline;
    int notemode;
    time_t exittime;
	/* ¨¦¨²¨¨?¨ºy?Y¡Áa¨°?¦Ì? userdata ?¨¢11?D */
    unsigned int usedspace;     /* used space of user's mailbox, in bytes */
#ifdef HAVE_USERMONEY
    int money;
    int score;
    char unused[20];
#endif
};

int main(int argc , char* argv[])
{
        FILE * fp,*fp2;
        struct userec1 bh;
        struct userec2 bhnew;

        int i;

	if(argc !=3 )
	{
		printf("usage: convert Old_PASSWDS_FILE New_PASSWDS_FILE\n");
		exit(0);
	}
	if((fp=fopen(argv[2],"r"))!=NULL){
		printf("NEW FILE exist!");
		fclose(fp);
		exit(0);
	}
        if((fp = fopen(argv[1],"r")) == NULL)
        {
                printf("open .BOARDS file failed!");
                exit(0);
        }

        if((fp2 = fopen(argv[2],"w")) == NULL)
        {
                printf("cant create newboards file!");
                exit(0);
        }

        while( fread(&bh,sizeof(struct userec1),1,fp) ){
        	memset(&bhnew,0,sizeof(struct userec2));
            memcpy(bhnew.userid, bh.userid, (IDLEN + 2));
            bhnew.flags = bh.flags;
            bhnew.title = bh.title;
            bhnew.firstlogin = bh.firstlogin;
            memcpy(bhnew.lasthost, bh.lasthost, 16);
            bhnew.numlogins = bh.numlogins;
            bhnew.numposts = bh.numposts;
#ifdef CONV_PASS
            memcpy(bhnew.passwd, bh.passwd, OLDPASSLEN);
            memcpy(bhnew.unused_padding, bh.unused_padding, 2);
#endif
            memcpy(bhnew.username, bh.username, NAMELEN);
            memcpy(bhnew.club_read_rights, bh.club_read_rights, (MAXCLUB>>5)*sizeof(unsigned int));
            memcpy(bhnew.club_write_rights, bh.club_write_rights, (MAXCLUB>>5)*sizeof(unsigned int));
            memcpy(bhnew.md5passwd, bh.md5passwd, MD5PASSLEN);
            bhnew.userlevel = bh.userlevel;
            bhnew.lastlogin = bh.lastlogin;
            bhnew.stay = bh.stay;
            bhnew.signature = bh.signature;
            bhnew.userdefine[0] = bh.userdefine[0];
            bhnew.userdefine[1] = bh.userdefine[1];
            bhnew.notedate = bh.notedate;
            bhnew.noteline = bh.noteline;
            bhnew.notemode = bh.notemode;
            bhnew.exittime = bh.exittime;
            bhnew.usedspace = bh.usedspace;
#ifdef HAVE_USERMONEY
			bhnew.money = bh.money;
			bhnew.score = bh.score;
			memcpy(bhnew.unused, bh.unused, 20);
#endif
        	fwrite(&bhnew,sizeof(struct userec2),1,fp2);
		}

        fclose(fp2);
        fclose(fp);
        return 0;
}


