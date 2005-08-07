/* add userdefine1 to userec */

#include "bbs.h"
#include "stdio.h"

struct userec1 {                 /* Structure used to hold information in */
    char userid[IDLEN + 2];     /* PASSFILE */
    char flags; /*一些标志，戒网，版面排序之类的*/
    unsigned char title; /*用户级别*/
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
    unsigned userlevel;
    time_t lastlogin;
    time_t stay;
    int signature;
    unsigned int userdefine;
    time_t notedate;
    int noteline;
    int notemode;
    time_t exittime;
	/* 生日数据转移到 userdata 结构中 */
    unsigned int usedspace;     /* used space of user's mailbox, in bytes */
#ifdef HAVE_USERMONEY
    int money;
    int score;
    char unused[20];
#endif
};

struct userec2 {                 /* Structure used to hold information in */
    char userid[IDLEN + 2];     /* PASSFILE */
    char flags; /*一些标志，戒网，版面排序之类的*/
    unsigned char title; /*用户级别*/
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
    unsigned userlevel;
    time_t lastlogin;
    time_t stay;
    int signature;
    unsigned int userdefine[2];
    time_t notedate;
    int noteline;
    int notemode;
    time_t exittime;
	/* 生日数据转移到 userdata 结构中 */
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
			memcpy(&bhnew, &bh, sizeof(struct userec1));
			bhnew.userdefine[1]=-1;
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

