/*-
 *              mail2bbs.c              -- mail -> bbs gateway for Firebird BBS 3.0
 *
 * $Id$
 */

#include "bbs.h"

#define BLOCKFILE ".blockmail"

void my_ansi_filter(char *source)
{
        char result[500];
        int  i, flag = 0, loc=0;
		int len;

		len = strlen(source);
		len = len >= sizeof(result) ? sizeof(result)-1 : len;
        for ( i = 0 ; i < len ; i++ )
		{
			if ( source[i] == '\x1B' ) {
					flag = 1;
					continue;
			} else if ( flag == 1 && isalpha(source[i]) ) {
					flag = 0;
					continue;
			} else if ( flag == 1 ) {
					continue;
			} else {
					result[loc++]=source[i];
			}
        }
        result[loc]='\0';
        strncpy(source, result, loc+1);
}

void strmov(char *source, int offset)
{
	int i;
	int len;

	if (source == NULL)
		return;
	if (offset <= 0)
		return;
	len = strlen(source);
	if (len <= offset)
		source[0] = '\0';
	else
	{
		for (i = offset; i < len; i++)
			source[i-offset] = source[i];
		source[i-offset] = '\0';
	}
}

void eat_forward(char *source)
{
	char *ptr;
	char *strfwd[] =
	{
		"[转寄]",
		"(转寄)",
		NULL
	};
	int i;

	for (i = 0; strfwd[i] != NULL; i++)
	{
		ptr = strstr(source, strfwd[i]);
		if (ptr == source)
		{
			if (source[6] == ' ')
			{
				strmov(source, 7);
				return;
			}
			else
			{
				strmov(source, 6);
				return;
			}
		}
		else if (ptr != NULL)
		{
			*ptr = '\0';
			return;
		}
	}
}

int 
append_mail(fin, sender1, sender, userid, title, received)
	FILE           *fin;
	char           *userid, *sender1, *sender, *title, *received;
{
	struct fileheader newmessage;
	char            fname[512], buf[256];
	char            maildir[256];
	struct stat     st;
	FILE           *fout;
	char            conv_buf[256];
	char			*ptr;
	struct userec   *user;

	/* check if the userid is in our bbs now */
	if (getuser(userid, &user) == 0)
		return -1;

	/* check for the mail dir for the userid */
	setmailpath(maildir, user->userid);

	if (stat(maildir, &st) == -1) {
		if (mkdir(maildir, 0755) == -1)
			return -1;
	} else {
		if (!(st.st_mode & S_IFDIR))
			return -1;
	}

	str_decode(conv_buf, title);
	my_ansi_filter(conv_buf);
	if (conv_buf[0] == '\0')
		strcpy(conv_buf, "无标题");

	/* allocate a record for the new mail */
	bzero(&newmessage, sizeof(newmessage));
	GET_MAILFILENAME(fname, maildir);
	strcpy(newmessage.filename, fname);
	setmailfile(buf, user->userid, fname);
	strcpy(fname, buf);
	strncpy(newmessage.title, conv_buf, sizeof(newmessage.title)-1);
	newmessage.title[sizeof(newmessage.title)-1] = '\0';
    ptr = strchr(sender, '@');
	if (ptr == NULL || ptr == sender)
	    return -1;
	strncpy(newmessage.owner, sender, sizeof(newmessage.owner)-1);
	newmessage.owner[sizeof(newmessage.owner)-1] = '\0';

	/* copy the stdin to the specified file */
	if ((fout = fopen(fname, "w")) == NULL)
	{
		printf("Cannot open %s \n", fname);
		return -1;
	}
	else
	{
		time_t          tmp_time;
		time(&tmp_time);
		fprintf(fout, "寄信人: %-.70s \n", sender);
		fprintf(fout, "标  题: %-.70s\n", conv_buf);
		fprintf(fout, "发信站: %s 信差\n", BBS_FULL_NAME);
		if (received != NULL && received[0] != '\0')
			fprintf(fout, "来  源: %-.70s\n", received);
		fprintf(fout, "日  期: %s\n", ctime(&tmp_time));
		while (fgets(buf, 255, fin) != NULL)
			fputs(buf, fout);
		fclose(fout);
	}

	if (chkusermail(user) == 0)
	{
		setmailfile(buf, user->userid, DOT_DIR);
		if (append_record(buf, &newmessage, sizeof(newmessage)) == 0)
		{
			struct stat fs;

			stat(fname, &fs);
			update_user_usedspace(fs.st_size, user);
			return 0;
		}
	}
	unlink(fname);
	return -1;
}

int
block_mail(addr)
	char           *addr;
{
	FILE           *fp;
	char            temp[STRLEN];

	if ((fp = fopen(BLOCKFILE, "r")) != NULL) {
		while (fgets(temp, STRLEN, fp) != NULL) {
			strtok(temp, "\n");
			if (strstr(addr, temp)) {
				fclose(fp);
				return 1;
			}
		}
		fclose(fp);
	}
	return 0;
}


int 
main(argc, argv)
	int             argc;
	char           *argv[];
{

	char            sender[256];
	char            username[256];
	char            receiver[256];
	char            nettyp[256];
	int             xxxx;

	/* argv[ 1 ] is original sender */
	/* argv[ 2 ] is userid in bbs   */
	/* argv[ 3 ] is the mail title  */
	/* argv[ 4 ] is the message-id  */
	if (argc != 3)
	{
		char           *p = (char *) rindex(argv[0], '/');

		printf("Usage: %s sender receiver_in_bbs\n",
		       p ? p + 1 : argv[0]);
		return 1;
	}

	setregid(BBSGID, BBSGID);
	setreuid(BBSUID, BBSUID);
	chdir(BBSHOME);
	resolve_ucache();

	if (strchr(argv[1], '@')) {
		strcpy(sender, argv[1]);
		/* added by netty  */
		xxxx = 0;
		while (sender[xxxx] != '@') {
			nettyp[xxxx] = sender[xxxx];
			xxxx = xxxx + 1;
		}
		nettyp[xxxx] = '\0';	/* added by netty  */
	} else {
		char           *p, *l, *r;
		char            buf[256];
		strcpy(buf, argv[1]);
		p = strtok(buf, " \t\n\r");
		l = strchr(argv[1], '(');
		r = strchr(argv[1], ')');
		if (l < r && l && r)
			strncpy(username, l, r - l + 1);
		sprintf(sender, "%s@%s %s", p, MAIL_BBSDOMAIN, username);
		strcpy(nettyp, p);
	}

	if (block_mail(sender))
		return -2;

	strcpy(receiver, argv[2]);
	return append_mail(stdin, nettyp, sender, receiver, getenv("TITLE"), getenv("MSGID"));
}
