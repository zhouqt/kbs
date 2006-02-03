/*
 * 这个程序有问题，本来的原理是，设置了 0xffff 之后，在用户检查邮箱的时候就会自动重新扫描
 * 所有的信件 reset usedspace。但是，有可能在自动扫描之前 usedspace 就发生了变化比方收到了
 * 新信件。如果你真的要真的重新设置 usedspace，把 0xffff 改成 get_mailusedspace(user,1)，
 * 不过这样一来这个程序的运行时间就会大大加长。另外什么情况下 usedspace 会不正确？嘿嘿，
 * 不要怀疑是水木的程序有问题，肯定是因为服务器上有人操作失误误删了文件！- atppp...如果不对请指正。
 */
#include "bbs.h"

int query_BM(struct userec *user, void *arg)
{
    if (!user->userid[0]) return 0;
    user->usedspace = 0xffff;
    return 1;
}

static void 
usage()
{
	fprintf(stderr, "Usage: resetMailTotal <-a|-u userid|-s userid>\n\n");
	fprintf(stderr, "    If -a parameter is provided, this program resets all userids' mail space,\n");
	fprintf(stderr, "    else only resets (-u) or shows (-s) the specified userid's mail space.\n");
}

int 
main(int argc, char ** argv)
{
	struct userec *user = NULL;

    chdir(BBSHOME);
    resolve_ucache();
    resolve_boards();
	if (argc == 2 && !strcmp(argv[1], "-a"))
    	apply_users(query_BM, NULL);
	else if (argc == 3 && !strcmp(argv[1], "-u"))
	{
		getuser(argv[2], &user);
		if (user == NULL)
		{
			fprintf(stderr, "User %s not found.\n", argv[1]);
			return -1;
		}
		user->usedspace = 0xFFFF;
	}
	else if (argc == 3 && !strcmp(argv[1], "-s"))
	{
		getuser(argv[2], &user);
		if (user == NULL)
		{
			fprintf(stderr, "User %s not found.\n", argv[1]);
			return -1;
		}
		printf("User %s usedspace: %u bytes.\n", user->userid, user->usedspace);
	}	else
		usage();

	return 0;
}

