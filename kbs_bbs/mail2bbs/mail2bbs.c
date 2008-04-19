#include "bbs.h"
/*
#define MAILDIR     BBSHOME"/mail"
*/
#define BLOCKFILE ".blockmail"
#define BUFLEN 256

#define UBMAPNUM 0

#if UBMAPNUM > 0
static char ubmap[UBMAPNUM][2][20] = {
	{"ArmMailing", "MailingTest"},
    {"",""}
};
#endif /* UBMAPNUM > 0 */

int str_decode(register unsigned char *dst, register unsigned char *src);

char* strsncpy(char *c,const char *d,size_t l){
	strncpy(c,d,l);
	c[l-1]=0;
    return c;
}

void
mailog(char *msg)
{
	FILE   *fp;
	char    xx[256];
	sprintf(xx, "%s/mail.logg", BBSHOME);
	fp = fopen(xx, "a+");
	if (fp == NULL)
		return;
	fprintf(fp, "%ld: %s\n", time(0), msg);
	fclose(fp);
}

void
chop(char *s)
{
	int i;
	i = strlen(s);
	if (s[i - 1] == '\n')
		s[i - 1] = 0;
	return;
}

void
decode_mail(FILE * fin, FILE * fout)
{
	char filename[BUFLEN];
	char encoding[BUFLEN];
	char buf[BUFLEN];
	char dbuf[BUFLEN + 20];
	char ch;
	int wc;
	long sizep=0;
	uint32_t sz;
	while (fgets(filename, sizeof (filename), fin)) {
		if (!fgets(encoding, sizeof (encoding), fin))
			return;
		chop(filename);
		chop(encoding);
		ch = 0;
		sizep = 0;
		sz=0;
		if (filename[0]) {
			str_decode((unsigned char*)buf,(unsigned char*)filename);
			strsncpy(filename, buf, sizeof (filename));
			printf("f:%s\n", filename);
			fprintf(fout, "\n信件有附件: %s\n", filename);
			fwrite(ATTACHMENT_PAD, ATTACHMENT_SIZE, 1, fout);
			fwrite(filename, strlen(filename) + 1, 1, fout);
			fwrite(&sz, sizeof (sz), 1, fout);
			//fwrite(&ch, 1, 1, fout);
			sizep = ftell(fout);
		}
		if (!strcmp(encoding, "quoted-printable")
		    || !strcmp(encoding, "base64")) {
			ch = encoding[0];
			while (fgets(buf, sizeof (buf), fin)) {
				if (!buf[0] && buf[1] == '\n')
					break;

				buf[sizeof(buf)-1]='\0';
				chop(buf);
				//wc = str_decode(dbuf, sbuf);
				wc = from64tobits(dbuf, buf);
				if (wc >= 0) {
					fwrite(dbuf, wc, 1, fout);
				} else{
					fputs(buf, fout);
				}
			}
		} else {
			while (fgets(buf, sizeof (buf), fin)) {
				if (!buf[0] && buf[1] == '\n')
					break;
				fputs(buf, fout);
			}
		}
		if (sizep) {
			uint32_t asize;
			sizep = ftell(fout) - sizep ;
			asize=htonl(sizep);
			fseek(fout, -sizep -4, SEEK_CUR);
			fwrite(&asize, 4, 1, fout);
			fseek(fout, sizep, SEEK_CUR);
		}
	}
	if (sizep)
		fputs("\n\n--\n", fout);
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
    else {
        for (i = offset; i < len; i++)
            source[i - offset] = source[i];
        source[i - offset] = '\0';
    }
}

void eat_forward(char *source)
{
    char *ptr;
    char *strfwd[] = {
        "[转寄]",
        "(转寄)",
        NULL
    };
    int i;

    for (i = 0; strfwd[i] != NULL; i++) {
        ptr = strstr(source, strfwd[i]);
        if (ptr == source) {
            if (source[6] == ' ') {
                strmov(source, 7);
                return;
            } else {
                strmov(source, 6);
                return;
            }
        } else if (ptr != NULL) {
            *ptr = '\0';
            return;
        }
    }
}

int my_after_post(struct fileheader *fh, char *boardname)
{
    char buf[256];
    int fd, err = 0, nowid = 0;
    /*
    if (!strncmp(fh->title, "Re:", 3)) {
        strncpy(fh->title, fh->title + 4, ARTICLE_TITLE_LEN);
    }
	*/
    setbfile(buf, boardname, DOT_DIR);

    if ((fd = open(buf, O_WRONLY | O_CREAT, 0664)) == -1) {
        err = 1;
    }

    if (!err) {
        writew_lock(fd, 0, SEEK_SET, 0);
        nowid = get_nextid(boardname);
        fh->id = nowid;
        fh->groupid = fh->id;
        fh->reid = fh->id;
#ifdef HAVE_REPLY_COUNT
        fh->replycount = 1;
#endif /* HAVE_REPLY_COUNT */
        set_posttime(fh);
        lseek(fd, 0, SEEK_END);
        if (safewrite(fd, fh, sizeof(fileheader)) == -1) {
            err = 1;
        }
        un_lock(fd, 0, SEEK_SET, 0);
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

int append_board(fin, sender1, sender, bname, title, received)
FILE *fin;
char *bname, *sender1, *sender, *title, *received;
{
    time_t now;
    FILE *fout;
    struct fileheader newmessage;
    char fname[512], buf[256], boardpath[256], conv_buf[256];
    const struct boardheader *brd;
    char *user, *userptr;
    char *lesssym, *nameptrleft, *nameptrright;
    char author[STRLEN];

/* check if the board is in our bbs now */
    if ((brd = getbcache(bname)) == NULL)
        return -21;
    strcpy(bname, brd->filename);

/* check for the dir for the board */
    setbpath(boardpath, bname);
    printf("OK, board dir is %s\n", boardpath);

    str_decode((unsigned char*)conv_buf,(unsigned char*)title);
/* copy from flyriver qmailpost.c */
    process_control_chars(conv_buf,NULL);
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
        return -22;
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

    strncpy(newmessage.owner, author, sizeof(newmessage.owner) - 1);
    newmessage.owner[sizeof(newmessage.owner) - 1] = '\0';
    newmessage.innflag[0] = 'L';
    newmessage.innflag[1] = 'L';
    printf("OK, the file is %s\n", fname);

/* copy the stdin to the specified file */
    if ((fout = fopen(fname, "w")) == NULL) {
        printf("Cannot open %s\n", fname);
        return -23;
    }
    now = time(NULL);
    fprintf(fout, "发信人: %s (%s), 信区: %s\n", newmessage.owner, sender, bname);
    fprintf(fout, "标  题: %s\n", conv_buf);
    fprintf(fout, "发信站: %s (%24.24s), 站内\n", BBS_FULL_NAME, ctime(&now));
    strcat(received, "\n");
    fprintf(fout, "来  源: %s\n", received);
    while (fgets(buf, 255, fin) != NULL) {
        fputs(buf, fout);
    }
    fclose(fout);

    return my_after_post(&newmessage, bname);
}

int
append_mail(fin, sender1, sender, userid, title, received)
FILE *fin;
char *userid, *sender1, *sender, *title, *received;
{
	struct fileheader newmessage;
	char fname[512],buf[BUFLEN], genbuf[BUFLEN];
	char maildir[BUFLEN];
	struct stat st;
	FILE *fout,*rmail;
	int passcheck = 0;
	char conv_buf[BUFLEN];
    struct userec *user;

/* check if the userid is in our bbs now */
    if (getuser(userid, &user) == 0){
        return -1;
	}

	if (!strcasecmp(userid, "guest"))
		return -11;

/* check for the mail dir for the userid */
    setmailpath(maildir, user->userid);

	if (stat(maildir, &st) == -1) {
		if (mkdir(maildir, 0755) == -1)
			return -2;
	} else {
		if (!(st.st_mode & S_IFDIR))
			return -3;
	}

	printf("Ok, dir is %s\n", genbuf);

	str_decode((unsigned char*)conv_buf,(unsigned char*)sender);
	strsncpy(sender, conv_buf, BUFLEN);
	str_decode((unsigned char*)conv_buf,(unsigned char*)title);
    process_control_chars(conv_buf,NULL);
    if (conv_buf[0] == '\0')
        strcpy(conv_buf, "没主题");

/* allocate a record for the new mail */
	bzero(&newmessage, sizeof (newmessage));
    GET_MAILFILENAME(fname, maildir);
    strcpy(newmessage.filename, fname);

	strsncpy(newmessage.title, conv_buf, sizeof (newmessage.title));
    strsncpy(newmessage.owner, sender, sizeof(newmessage.owner) );

/* copy the stdin to the specified file */
    setmailfile(genbuf, user->userid, fname);
	strcpy(fname,genbuf);

	printf("Ok, the file is %s\n", maildir);

	if ((fout = fopen(fname, "w")) == NULL) {
		printf("Cannot open %s \n", maildir);
		return -4;
	} else {
		time_t tmp_time;
		struct stat st;

		time(&tmp_time);
		fprintf(fout, "寄信人: %-.70s \n", sender);
		fprintf(fout, "标  题: %-.70s\n", conv_buf);
		fprintf(fout, "发信站: %s BBS\n", BBS_FULL_NAME);
		if (received[0] != '\0')
			fprintf(fout, "来  源: %-.70s\n", received);
		fprintf(fout, "日  期: %s\n", ctime(&tmp_time));
		if (passcheck >= 1) {
			fprintf(fout, "亲爱的 %s:\n", sender1);
			sprintf(maildir, "etc/%s",
				(passcheck == 5) ? "smail" : "fmail");
			if ((rmail = fopen(maildir, "r")) != NULL) {
				while (fgets(genbuf, sizeof (genbuf), rmail) !=
				       NULL)
					fputs(genbuf, fout);
				fclose(rmail);
			}
		} else
			decode_mail(fin, fout);
		if (fstat(fileno(fout), &st) != -1)
        	newmessage.eff_size = st.st_size;
		fclose(fout);

	}

    if (chkusermail(user) == 0) {
        setmailfile(buf, user->userid, DOT_DIR);
        if (append_record(buf, &newmessage, sizeof(newmessage)) == 0) {
           	update_user_usedspace(newmessage.eff_size, user);
			setmailcheck(user->userid);
            return 0;
        }
    }
    unlink(fname);
    return -5;

}

int
block_mail(addr)
char *addr;
{
	FILE *fp;
	char temp[STRLEN];

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
int argc;
char *argv[];
{

	char myarg[4][BUFLEN];
	char nettyp[BUFLEN];
	char *p;
	int i;

	for (i = 0; i < 4; i++)
		if (!fgets(myarg[i], sizeof (myarg[i]), stdin))
			return i + 1;
	for (i = 0; i < 4; i++)
		chop(myarg[i]);

	chdir(BBSHOME);
	setreuid(BBSUID, BBSUID);
	setregid(BBSGID, BBSGID);
    resolve_ucache();
    resolve_boards();
    resolve_utmp();

	strsncpy(nettyp, myarg[0], sizeof (nettyp));
	p = strchr(nettyp, '@');
	if (NULL != p)
		*p = 0;

	if (block_mail(myarg[0]) == true)
		return -2;

#if UBMAPNUM > 0
    do{
        char uboard[32];
        for(i=0;i<UBMAPNUM;i++){
            if(!strcasecmp(ubmap[i][0], myarg[1])){
                strcpy(uboard, ubmap[i][1]);
                append_board(stdin, nettyp, myarg[0], uboard, myarg[2], myarg[3]);
                break;
            }
        }
    }
    while(0);
#endif /* UBMAPNUM > 0 */

	return append_mail(stdin, nettyp, myarg[0], myarg[1], myarg[2],
			   myarg[3]);
}

