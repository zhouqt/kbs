#include "bbs.h"
struct userec2 {                /* Structure used to hold information in */
    char userid[IDLEN + 2];     /* PASSFILE */
    char fill[30];
    time_t firstlogin;
    char lasthost[16];
    unsigned int numlogins;
    unsigned int numposts;
    char flags[2];
    char passwd[OLDPASSLEN];
    char username[NAMELEN];
    char ident[NAMELEN];
    unsigned char md5passwd[MD5PASSLEN];
    char realemail[STRLEN - 16];
    unsigned userlevel;
    time_t lastlogin;
    time_t stay;
    char realname[NAMELEN];
    char address[STRLEN];
    char email[STRLEN];
    int signature;
    unsigned int userdefine;
    time_t notedate;
    int noteline;
    int notemode;
    time_t exittime;
    int unuse2;                 /* no use */
};

main()
{
    FILE *rec, *rec2;
    int i = 0;
    struct userec2 user;
    struct userec newuser;

    rec = fopen("./.PASSWDS", "rb");
    rec2 = fopen("./.PASSWDS.tmp", "wb");

    printf("Records transfer...");
    while (1) {
        i++;
        if (fread(&user, sizeof(user), 1, rec) <= 0)
            break;
	memcpy(newuser.userid,user.userid,IDLEN+2);
	newuser.firstlogin=user.firstlogin;
	memcpy(newuser.lasthost,user.lasthost,16);
	newuser.numlogins=user.numlogins;
	newuser.numposts=user.numposts;
	memcpy(newuser.flags,user.flags,2);
	memcpy(newuser.username,user.username,NAMELEN);
	memcpy(newuser.ident,user.ident,NAMELEN);
	memcpy(newuser.md5passwd,user.md5passwd,MD5PASSLEN);
	memcpy(newuser.realemail,user.realemail,STRLEN-16);
	newuser.userlevel=user.userlevel;
	newuser.lastlogin=user.lastlogin;
	newuser.stay=user.stay;
	memcpy(newuser.realname,user.realname,NAMELEN);
	memcpy(newuser.address,user.address,STRLEN);
	memcpy(newuser.email,user.email,STRLEN);
	newuser.signature=user.signature;
	newuser.userdefine=user.userdefine;
	newuser.notedate=user.notedate;
	newuser.noteline=user.noteline;
	newuser.notemode=user.notemode;
	newuser.exittime=user.exittime;
	newuser.unuse2=user.unuse2;

        fwrite(&newuser, sizeof(newuser), 1, rec2);
    }
    printf("\n%d records changed...\n", i);
    fclose(rec);
    fclose(rec2);
}
