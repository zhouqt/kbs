/*-
 *              mail2bbs.c              -- mail -> bbs gateway for Firebird BBS 3.0
 *
 * $Id$
 */

#include "bbs.h"
#include "mime_dec.c"

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

int my_after_post(struct fileheader *fh, char *boardname)
{
	char buf[256];
	int fd, err = 0, nowid = 0;
	char *p;

	if (!strncmp(fh->title, "Re:", 3)) {
		strncpy(fh->title, fh->title + 4, STRLEN);
	}
	setbfile(buf, boardname, DOT_DIR);

	if ((fd = open(buf, O_WRONLY | O_CREAT, 0664)) == -1) {
		err = 1;
	}

	if (!err) {
		flock(fd, LOCK_EX);
		nowid = get_nextid(boardname);
		fh->id = nowid;
		fh->groupid = fh->id;
		fh->reid = fh->id;
		set_posttime(fh);
		lseek(fd, 0, SEEK_END);
		if (safewrite(fd, fh, sizeof(fileheader)) == -1) {
			err = 1;
		}
		flock(fd, LOCK_UN);
		close(fd);
	}
	if (err) {
		setbfile(buf, boardname, fh->filename);
		unlink(buf);
		return 1;
	}
	updatelastpost(boardname);

	if (fh->id == fh->groupid)
		setboardorigin(boardname, 1);
	setboardtitle(boardname, 1);
	return 0;
}

int append_board(fin, sender1, sender, bname, title, received, encoding, boundary)
	FILE *fin;
	char *bname, *sender1, *sender, *title, *received, *encoding, *boundary;
{
	time_t now;
	FILE *fout;
	struct fileheader newmessage;
	char fname[512], buf[256], boardpath[256], conv_buf[256];
	struct boardheader *brd;
	char *user, *userptr;
	char *lesssym, *nameptrleft, *nameptrright;
	char author[STRLEN];

	/* check if the board is in our bbs now */
	if ((brd = getbcache(bname)) == NULL)
		return -1;
	strcpy(bname, brd->filename);

	/* check for the dir for the board */
	setbpath(boardpath, bname);
	printf("OK, board dir is %s\n", boardpath);

	str_decode(conv_buf, title);
	/* copy from flyriver qmailpost.c */
	my_ansi_filter(conv_buf);
	if (conv_buf[0] == '\0')
		strcpy(conv_buf, "没主题");
	eat_forward(conv_buf);

	/* allocate a record for the new mail */
	bzero(&newmessage, sizeof(newmessage));
	GET_POSTFILENAME(fname, boardpath);
	strcpy(newmessage.filename, fname);
	setbfile(buf, bname, fname);
	strcpy(fname, buf);
	if (!dashd(boardpath))
		return -1;
	strncpy(newmessage.title, conv_buf, sizeof(newmessage.title) - 1);
	newmessage.title[sizeof(newmessage.title) - 1] = '\0';

	/* parse the owner address */
	if (strchr(sender, '<') && (sender[strlen(sender) - 1] == '>'))
		user = (char *) strrchr(sender, '@');
	else
		user = (char *) strchr(sender, '@');
	lesssym = (char *) strchr(sender, '<');
	nameptrleft = NULL, nameptrright = NULL;
	if (lesssym == NULL || lesssym >= user) {
		lesssym = sender;
		nameptrleft = strchr(sender, '(');
		if (nameptrleft != NULL)
			nameptrleft++;
		nameptrright = strrchr(sender, ')');
	} else {
		nameptrleft = sender;
		nameptrright = strrchr(sender, '<');
		lesssym++;
	}
	if (user != NULL) {
		*user = '\0';
		userptr = (char *) strchr(sender, '.');
		if (userptr != NULL) {
			*userptr = '\0';
			strncpy(author, lesssym, sizeof(author));
			*userptr = '.';
		} else
			strncpy(author, lesssym, sizeof(author));
		*user = '@';
	} else
		strncpy(author, lesssym, sizeof(author));
	if (!isalnum(author[0]))
		strcpy(author, "Unknown");
	strcat(author, ".");

	strncpy(newmessage.owner, author, sizeof(newmessage.owner) - 1);
	newmessage.owner[sizeof(newmessage.owner) - 1] = '\0';
	newmessage.innflag[0] = 'L';
	newmessage.innflag[1] = 'L';
	printf("OK, the file is %s\n", fname);

	/* copy the stdin to the specified file */
	if ((fout = fopen(fname, "w")) == NULL) {
		printf("Cannot open %s\n", fname);
		return -1;
	}
	now = time(NULL);
	fprintf(fout, "发信人: %s (%s), 信区: %s\n", newmessage.owner, sender, bname);
	fprintf(fout, "标  题: %s\n", conv_buf);
	fprintf(fout, "发信站: %s (%24.24s), 邮件转寄\n", BBS_FULL_NAME, ctime(&now));
	strcat(received, "\n");
	fprintf(fout, "来  源: %s\n", received);
	if ((!boundary)||(!boundary[0])) {
		int t;
		char data[256];
		if (strstr(encoding, "8bit")) t = 1;
		else if (strstr(encoding, "quoted-printable")) t = 2;
		else if (strstr(encoding, "base64")) t = 3;
		else t = 0;
		while (fgets(buf, 255, fin) != NULL)
			switch (t) {
				case 1:
					fputs(buf, fout);
					break;
				case 2:
					qpdec(buf, data);
					fputs(data, fout);
					break;
				case 3:
					data[b64dec(buf, data)]=0;
					fputs(data, fout);
					break;
				default:
					fputs(buf, fout);
			}
	}
	else {
#define READ  if (!fgets(buf, 255, fin)) { err=1; break;}
#define WRITE(data, size)  FileSize += fwrite(data, 1, size, fout); totalsize += size;
#define CHECK  if ((totalsize>MAXMAILSIZE)||(number-firstText>MAXATTACHMENTCOUNT)) err=2; if (err) break;
		int err;
		int ContentType,  ContentEncoding;
		int number;
		int block;
		char FileName[256];
		char Boundary[256];
		int FileSize;
		char data[256];
		int totalsize;
		int firstText;
		err = 0;
		buf[0] = 0;
		totalsize = 0;
		firstText = 0;
		do {
			READ
		} while (!strstr(buf, boundary));
		number = 0;
		do {
			READ
		} while (0);
		do {
			ContentType = 0;
			ContentEncoding = 0;
			number++;
			FileName[0]=0;
			do {
				char* tag;
				if (tag=strstr(buf, ": ")) {
					tag[0]=0; tag+=2;
					if (!strcasecmp(buf, "Content-Type")) {
						char* t;
						if (t=strchr(tag, ';')) *t=0;
						if (!strcasecmp(tag, "text/plain")) ContentType = 1;
						else if (!strcasecmp(tag, "text/html")) ContentType = 2;
						else if (!strcasecmp(tag, "multipart/related")) ContentType = 3;
					} else if (!strcasecmp(buf, "Content-Transfer-Encoding")) {
						if (strstr(tag, "8bit")) ContentEncoding = 1;
						else if (strstr(tag, "quoted-printable")) ContentEncoding = 2;
						else if (strstr(tag, "base64")) ContentEncoding = 3;
					}
				} else if (tag=strstr(buf, "filename=\"")) {
					char* t;
					tag+=10;
					if (t=strchr(tag, '"')) *t=0;
					strcpy(FileName, tag);
				} else if ((ContentType==3) && (tag=strstr(buf, "boundary=\""))) {
					char* t;
					tag+=10;
					if (t=strchr(tag, '\"')) *t=0;
					while (tag[strlen(tag)-1]<27) tag[strlen(tag)-1]=0;
					strcpy(Boundary, tag);
				}
				READ
			} while (strlen(buf)>2); /*(strcspn(buf, " \r\n")<strlen(buf));*/
			CHECK
				do {
					READ
				} while (strlen(buf)<2); 
			CHECK
				if (!FileName[0])
					switch (ContentType) {
						case 1: 
							strcpy(FileName, "noname.txt");
							break;
						case 2:
							strcpy(FileName, "noname.htm");
							break;
							/*						case 3:
							 *						strcpy(FileName, "noname.mht");
							 *						break;
							 *						*/						default:
							strcpy(FileName, "noname");
					}			
			block = 0;
			FileSize = 0;
			do {
				if ((ContentType==1)&&(number==1)) {
					firstText = 1;
					switch (ContentEncoding) { 
						case 1:
							fputs(buf, fout);
							break;
						case 2:
							qpdec(buf, data);
							fputs(data, fout);
							break;
						case 3:
							data[b64dec(buf, data)]=0;
							fputs(data, fout);
							break;
						default:
							fputs(buf, fout);
					}

				} else {
					if (!newmessage.attachment) newmessage.attachment=ftell(fout);
					if (!FileSize) {
						fwrite(ATTACHMENT_PAD, 1, ATTACHMENT_SIZE, fout);
						fwrite(FileName, 1, strlen(FileName)+1, fout);
						fwrite(&FileSize, 1, 4, fout);
						totalsize += 12+strlen(FileName)+1;
					}
					switch (ContentEncoding) {
						case 1:
							WRITE(buf, strlen(buf));
							break;
						case 2:
							qpdec(buf, data);
							WRITE(data, strlen(data));
							break;
						case 3:
							WRITE(data, b64dec(buf, data));
							break;
						default:
							/*								if ((!FileSize)&&(ContentType==3)) {
							 *								WRITE("Content-Type: multipart/related;\n", 33);
							 *								WRITE("      boundary=\"", 16);
							 *								WRITE(Boundary, strlen(Boundary));
							 *								WRITE("\";\n",3);
							 *								WRITE("      type=\"multipart/alternative\"\n\n", 36);
							 *								}
							 *								*/								WRITE(buf, strlen(buf));
					}
				}
				CHECK
					READ
			} while (!strstr(buf, boundary));
			if ((ContentType!=1)||(number!=1)) {
				fseek(fout, -FileSize-4, SEEK_CUR);
				FileSize = htonl(FileSize);
				fwrite(&FileSize, 1, 4, fout);
				fseek(fout, 0, SEEK_END);
			}
			CHECK
				READ
		} while (!feof(fin));
		/*if (err==2) fputs("Mail too long or too many attachments!", fout);*/
#undef READ
#undef WRITE
#undef CHECK
	}
	fclose(fout);

	return my_after_post(&newmessage, bname);
}


			
int 
append_mail(fin, sender1, sender, userid, title, received, encoding, boundary)
	FILE           *fin;
	char           *userid, *sender1, *sender, *title, *received, *encoding, *boundary;
{
	struct fileheader newmessage;
	char            fname[512], buf[256];
	char            maildir[256];
	struct stat     st;
	FILE		*fout;
	char            conv_buf[256];
	char		*ptr, *ptr2;
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
	
	strncpy(buf, sender, 255);
	buf[255]='\0';
	if (ptr=strrchr(buf, '<'))
		if(ptr2=strrchr(ptr, '>'))
			if (ptr<ptr2-1) {
				memmove(buf, ptr+1, ptr2-ptr-1);
				buf[ptr2-ptr-1]='\0';
			}
	strncpy(newmessage.owner, buf, OWNER_LEN-1);
	newmessage.owner[OWNER_LEN-1] = '\0';
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
		fprintf(fout, "发信站: %s (%24.24s), 邮件转寄\n", BBS_FULL_NAME, ctime(&tmp_time));
		if (received != NULL && received[0] != '\0')
			fprintf(fout, "来  源: %-.70s\n", received);
		if ((!boundary)||(!boundary[0])) {
			int t;
			char data[256];
			if (strstr(encoding, "8bit")) t = 1;
			else if (strstr(encoding, "quoted-printable")) t = 2;
			else if (strstr(encoding, "base64")) t = 3;
			else t = 0;
			while (fgets(buf, 255, fin) != NULL)
				switch (t) {
						case 1:
								fputs(buf, fout);
								break;
						case 2:
								qpdec(buf, data);
								fputs(data, fout);
								break;
						case 3:
								data[b64dec(buf, data)]=0;
								fputs(data, fout);
								break;
						default:
								fputs(buf, fout);
				}
		}
		else {
			#define READ  if (!fgets(buf, 255, fin)) { err=1; break;}
			#define WRITE(data, size)  FileSize += fwrite(data, 1, size, fout); totalsize += size;
			#define CHECK  if ((totalsize>MAXMAILSIZE)||(number-firstText>MAXATTACHMENTCOUNT)) err=2; if (err) break;
			int err;
			int ContentType,  ContentEncoding;
			int number;
			int block;
			char FileName[256];
			char Boundary[256];
			int FileSize;
			char data[256];
			int totalsize;
			int firstText;
			err = 0;
			buf[0] = 0;
			totalsize = 0;
			firstText = 0;
			do {
				READ
			} while (!strstr(buf, boundary));
			number = 0;
			do {
				READ
			} while (0);
			do {
				ContentType = 0;
				ContentEncoding = 0;
				number++;
				FileName[0]=0;
				do {
					char* tag;
					if (tag=strstr(buf, ": ")) {
						tag[0]=0; tag+=2;
						if (!strcasecmp(buf, "Content-Type")) {
							char* t;
							if (t=strchr(tag, ';')) *t=0;
							if (!strcasecmp(tag, "text/plain")) ContentType = 1;
							else if (!strcasecmp(tag, "text/html")) ContentType = 2;
							else if (!strcasecmp(tag, "multipart/related")) ContentType = 3;
						} else if (!strcasecmp(buf, "Content-Transfer-Encoding")) {
							if (strstr(tag, "8bit")) ContentEncoding = 1;
							else if (strstr(tag, "quoted-printable")) ContentEncoding = 2;
							else if (strstr(tag, "base64")) ContentEncoding = 3;
						}
					} else if (tag=strstr(buf, "filename=\"")) {
						char* t;
						tag+=10;
						if (t=strchr(tag, '"')) *t=0;
						strcpy(FileName, tag);
					} else if ((ContentType==3) && (tag=strstr(buf, "boundary=\""))) {
						char* t;
						tag+=10;
						if (t=strchr(tag, '\"')) *t=0;
						while (tag[strlen(tag)-1]<27) tag[strlen(tag)-1]=0;
						strcpy(Boundary, tag);
					}
					READ
				} while (strlen(buf)>2); /*(strcspn(buf, " \r\n")<strlen(buf));*/
				CHECK
				do {
					READ
				} while (strlen(buf)<2); 
				CHECK
				if (!FileName[0])
					switch (ContentType) {
						case 1: 
							strcpy(FileName, "noname.txt");
							break;
						case 2:
							strcpy(FileName, "noname.htm");
							break;
/*						case 3:
							strcpy(FileName, "noname.mht");
							break;
*/						default:
							strcpy(FileName, "noname");
					}			
				block = 0;
				FileSize = 0;
				do {
					if ((ContentType==1)&&(number==1)) {
						firstText = 1;
						switch (ContentEncoding) { 
							case 1:
								fputs(buf, fout);
								break;
							case 2:
								qpdec(buf, data);
								fputs(data, fout);
								break;
							case 3:
								data[b64dec(buf, data)]=0;
								fputs(data, fout);
								break;
							default:
								fputs(buf, fout);
						}
									
					} else {
						if (!newmessage.attachment) newmessage.attachment=ftell(fout);
						if (!FileSize) {
							fwrite(ATTACHMENT_PAD, 1, ATTACHMENT_SIZE, fout);
							fwrite(FileName, 1, strlen(FileName)+1, fout);
							fwrite(&FileSize, 1, 4, fout);
							totalsize += 12+strlen(FileName)+1;
						}
						switch (ContentEncoding) {
							case 1:
								WRITE(buf, strlen(buf));
								break;
							case 2:
								qpdec(buf, data);
								WRITE(data, strlen(data));
								break;
							case 3:
								WRITE(data, b64dec(buf, data));
								break;
							default:
/*								if ((!FileSize)&&(ContentType==3)) {
									WRITE("Content-Type: multipart/related;\n", 33);
									WRITE("      boundary=\"", 16);
									WRITE(Boundary, strlen(Boundary));
									WRITE("\";\n",3);
									WRITE("      type=\"multipart/alternative\"\n\n", 36);
								}
*/								WRITE(buf, strlen(buf));
						}
					}
					CHECK
					READ
				} while (!strstr(buf, boundary));
				if ((ContentType!=1)||(number!=1)) {
						fseek(fout, -FileSize-4, SEEK_CUR);
						FileSize = htonl(FileSize);
						fwrite(&FileSize, 1, 4, fout);
						fseek(fout, 0, SEEK_END);
				}
				CHECK
				READ
			} while (!feof(fin));
			/*if (err==2) fputs("Mail too long or too many attachments!", fout);*/
			#undef READ
			#undef WRITE
			#undef CHECK
		}
		fclose(fout);
	}

	if (chkusermail(user) == 0)
	{
		struct stat fs;

		stat(fname, &fs);
		setmailfile(buf, user->userid, DOT_DIR);
		newmessage.eff_size = fs.st_size;
		if (append_record(buf, &newmessage, sizeof(newmessage)) == 0)
		{
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
	int				mail2board = 0;
	char *			ptr;	
	/* argv[ 1 ] is original sender */
	/* argv[ 2 ] is userid in bbs   */
	/* argv[ 3 ] is the mail title  */
	/* argv[ 4 ] is the message-id  */
	if (argc < 3)
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
	resolve_boards();
	
	if (argv[1] == NULL || strlen(argv[1]) == 0)
	{
		fprintf(stderr, "Error: Unknown sender\n");
		return -2;
	}
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
	
	ptr = strchr(receiver, '.');
	if (ptr == NULL) {
		mail2board = 0;
	} else if (!strncasecmp(ptr + 1, "board", 5)) {
		*ptr = '\0';
		mail2board = 1;
	} else
		return 1;

	if (mail2board == 1) {
		return append_board(stdin, nettyp, sender, receiver, getenv("TITLE"), getenv("MSGID"), argv[3], argv[4]);
	} else {
		return append_mail(stdin, nettyp, sender, receiver, getenv("TITLE"), getenv("MSGID"), argv[3], argv[4]);
	}
}


