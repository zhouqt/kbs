/***********
stiger: 重新计算所有用户或者某一用户的签名档数目
************/

#include "bbs.h"

int recalc_sig(struct userec *user, char *arg)
{
	struct userdata ud;

	if( user==NULL || user->userid[0]=='\0' )
		return 0;

	if( read_userdata(user->userid, &ud) < 0 ){
		printf("%s: no userdata load\n", user->userid);
		return 1;
	}
	ud.signum = calc_numofsig(user->userid);
	if( write_userdata(user->userid, &ud) < 0 ){
		printf("%s: write userdata error\n",user->userid);
	}
    return 1;
}

static void 
usage()
{
	fprintf(stderr, "Usage: recalc_signum <-a|-u userid>\n\n");
	fprintf(stderr, "    If -a parameter is provided, this program will recalc all userids' signum,\n");
	fprintf(stderr, "    else only recalc the specified userid's signum.\n");
}

int 
main(int argc, char ** argv)
{
	struct userec *user = NULL;

    chdir(BBSHOME);
    resolve_ucache();
    resolve_boards();
	if (argc == 2 && !strcmp(argv[1], "-a"))
    	apply_users(recalc_sig, NULL);
	else if (argc == 3 && !strcmp(argv[1], "-u"))
	{
		getuser(argv[2], &user);
		if (user == NULL)
		{
			fprintf(stderr, "User %s not found.\n", argv[1]);
			return -1;
		}
		recalc_sig(user, NULL);
	}
	else
		usage();

	return 0;
}
