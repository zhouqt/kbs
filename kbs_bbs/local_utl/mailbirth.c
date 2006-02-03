#include "bbs.h"

#define MAXBIRTHFILE 12
#ifndef BIRTHFILEPATH
#define BIRTHFILEPATH "0Announce/groups/system.faq/SYSOP/birthfile/"
#endif

char filename[MAXBIRTHFILE][256];
int birthfilenum=0;
struct userdata ud;

unsigned char birthmonth;
unsigned char birthday;

int mailuser(struct userec* user,void* arg)
{
	int i;

	if( read_userdata(user->userid, &ud) != 0)
		return 0;

#ifdef HAVE_BIRTHDAY
	if(ud.birthmonth != birthmonth || ud.birthday != birthday)
#endif
		return 0;

	i=rand()%birthfilenum;

    mail_file("SYSOP", filename[i], user->userid, "水木清华站恭祝您生日快乐", BBSPOST_LINK, NULL);
    return 0;

};

int load_birthfile(){

    DIR *dirp;
    struct dirent *de;
    char buf[256];
	struct stat st;

    if (!(dirp = opendir(BIRTHFILEPATH)))
        return -1;

    while ((de = readdir(dirp))!=NULL) {
        if (de->d_name && de->d_name[0]!='.'){
			sprintf(filename[birthfilenum], "%s%s", BIRTHFILEPATH, de->d_name);

            if (stat(filename[birthfilenum], &st) != 0)
				continue;

			birthfilenum++;

			if(birthfilenum >= MAXBIRTHFILE)
				break;
        }
    }
    closedir(dirp);

	return birthfilenum;

}

main()
{
	time_t t;
	struct tm tt;

    chdir(BBSHOME);

	if (load_birthfile()<=0){
		printf("error read birthfile\n");
		exit(0);
	}

	t=time(0);
	if (localtime_r(&t, &tt)==NULL) exit(0);

	birthmonth = tt.tm_mon + 1;
	birthday = tt.tm_mday;

    resolve_ucache();
    resolve_boards();
    resolve_utmp();

	srand(time(0));
    apply_users(mailuser,NULL);
}

