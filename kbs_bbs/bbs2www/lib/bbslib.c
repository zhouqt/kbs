#include "types.h"
#include "bbslib.h"

int seek_in_file(filename,seekstr)
char filename[STRLEN],seekstr[STRLEN];
{
    FILE *fp;
    char buf[STRLEN];
    char *namep;

    if ((fp = fopen(filename, "r")) == NULL)
        return 0;
    while (fgets(buf, STRLEN, fp) != NULL)
	{
        namep = (char *)strtok( buf, ": \n\r\t" );
        if (namep != NULL && strcasecmp(namep, seekstr) == 0 ) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

int junkboard(char *board)
{
    return seek_in_file("etc/junkboards", board);
}

int file_has_word(char *file, char *word) {
	FILE *fp;
	char buf[256], buf2[256];
	fp=fopen(file, "r");
	if(fp==0) return 0;
	while(1) {
		bzero(buf, 256);
		if(fgets(buf, 255, fp)==0) break;
		sscanf(buf, "%s", buf2);
		if(!strcasecmp(buf2, word)) {
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}

int f_append(char *file, char *buf) {
	FILE *fp;
	fp=fopen(file, "a");
	if(fp==0) return;
	fprintf(fp, "%s", buf);
	fclose(fp);
}

struct stat *f_stat(char *file) {
	static struct stat buf;
	bzero(&buf, sizeof(buf));
	if(stat(file, &buf)==-1) bzero(&buf, sizeof(buf));
	return &buf;
}

char *Ctime(time_t t) {
	static char s[80];
	sprintf(s, "%24.24s", ctime(&t));
	return s;
}

char *noansi(char *s) {
	static char buf[1024];
	int i=0, mode=0;
	while(s[0] && i<1023) {
		if(mode==0) { 
			if(s[0]==27) {
				mode=1;
			} else {
				buf[i]=s[0];
				i++;
			}
		} else {
			if(!strchr(";[0123456789", s[0])) mode=0;
		}
		s++;
	}
	buf[i]=0;
	return buf;
}

char *nohtml(char *s) {
	char *buf=calloc(strlen(s)+1, 1);
	int i=0, mode=0;
	while(s[0] && i<1023) {
		if(mode==0) {
			if(s[0]=='<') {
				mode=1;
			} else {
				buf[i]=s[0];
				i++;
			}
		} else {
			if(s[0]=='>') mode=0;
		}
		s++;
	}
	buf[i]=0;
	return buf;
}

char *strright(char *s, int len) {
	int l=strlen(s);
	if(len<=0) return "";
	if(len>=l) return s;
	return s+(l-len);
}

char *strcasestr(char *s1, char *s2) {
	int l;
	l=strlen(s2);
	while(s1[0]) {
		if(!strncasecmp(s1, s2, l)) return s1;
		s1++;
	}
	return 0;
}

int strsncpy(char *s1, char *s2, int n) {
	int l=strlen(s2);
	if(n<0) return;
	if(n>l+1) n=l+1;
	strncpy(s1, s2, n-1);
	s1[n-1]=0;
}

char *ltrim(char *s) {
	char *s2=s;
	if(s[0]==0) return s;
	while(s2[0] && strchr(" \t\r\n", s2[0])) s2++;
	return s2;
}

char *rtrim(char *s) {
	static char t[1024], *t2;
	if(s[0]==0) return s;
	strsncpy(t, s, 1024);
	t2=t+strlen(s)-1;
	while(strchr(" \t\r\n", t2[0]) && t2>t) t2--;
	t2[1]=0;
	return t;
}

char *get_new_shm(int key, int size) {
        int id;
        id=shmget(key, size, IPC_CREAT | IPC_EXCL | 0640);
        if(id<0) return 0;
        return shmat(id, NULL, 0);
}

char *get_old_shm(int key, int size) {
        int id;
        id=shmget(key, size, 0);
        if(id<0) return 0;
        return shmat(id, NULL, 0);
}

char *get_shm(int key, int size) {
        int id;
        id=shmget(key, size, IPC_CREAT | 0640);
        if(id<0) return 0;
        return shmat(id, NULL, 0);
}

char *getsenv(char *s) {
        char *t=getenv(s);
        if(t) return t;
        return "";
}

int http_quit() {
	printf("\n</html>\n");
	exit(0);
}

int http_fatal(char *fmt, ...) {
        char buf[1024];
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(buf, 1023, fmt, ap);
        va_end(ap);
	buf[1023]=0;
 	printf("´íÎó! %s! <br><br>\n", buf);
	printf("<a href=javascript:history.go(-1)>¿ìËÙ·µ»Ø</a>");
	http_quit();
}

int strnncpy(char *s, int *l, char *s2) {
	strncpy(s+(*l), s2, strlen(s2));
	(*l)+=strlen(s2);
}

int hsprintf(char *s, char *fmt, ...) {
	char buf[1024], ansibuf[80], buf2[80];
	char *tmp;
	int c, bold, m, i, l, len;
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, 1023, fmt, ap);
	va_end(ap);
	s[0]=0;
	l=strlen(buf);
	len=0;
	bold=0;
	for(i=0; i<l; i++) {
		c=buf[i];
		if(c=='&') {
			strnncpy(s, &len, "&amp;");
		} else if(c=='<') {
			strnncpy(s, &len, "&lt;");
		} else if(c=='>') {
			strnncpy(s, &len, "&gt;");
		} else if(c==27) {
			if(buf[i+1]!='[') continue;
			for(m=i+2; m<l && m<i+24; m++)
				if(strchr("0123456789;", buf[m])==0) break;
			strsncpy(ansibuf, &buf[i+2], m-(i+2)+1);
			i=m;
			if(buf[i]!='m') continue;
			if(strlen(ansibuf)==0) {
				bold=0;
				strnncpy(s, &len, "<font class=c37>");
			}
			tmp=strtok(ansibuf, ";");
			while(tmp) {
				c=atoi(tmp);
				tmp=strtok(0, ";");
				if(c==0) {
					strnncpy(s, &len, "<font class=c37>");
					bold=0;
				}
				if(c>=30 && c<=37) {
					if(bold==1) sprintf(buf2, "<font class=d%d>", c);
					if(bold==0) sprintf(buf2, "<font class=c%d>", c);
					strnncpy(s, &len, buf2);
				}
			}
		} else {
			s[len]=c;
			len++;
		}
	}
	s[len]=0;
}


int hprintf(char *fmt, ...) {
	char buf[8096], buf2[1024];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf2, 1023, fmt, ap);
	va_end(ap);
	hsprintf(buf, "%s", buf2);
	printf("%s", buf);
}

int hhprintf(char *fmt, ...) {
	char buf0[1024], buf[1024], *s, *getparm();
	int len=0;
	int my_link_mode;
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, 1023, fmt, ap);
	va_end(ap);
	buf[1023]=0;
	s=buf;
	my_link_mode=atoi(getparm("my_link_mode"));
	if(my_link_mode==1)  return hprintf("%s", buf);
	if(!strcasestr(s, "http://") && !strcasestr(s, "ftp://") && !strcasestr(s, "mailto:"))
		return hprintf("%s", buf);
	while(s[0]) {
		if(!strncasecmp(s, "http://", 7) || !strncasecmp(s, "mailto:", 7) || !strncasecmp(s, "ftp://", 6)) {
			char *tmp;
			if(len>0) {
				buf0[len]=0;
				hprintf("%s", buf0);
				len=0;
			}
			tmp=strtok(s, "\'\" \r\t)(,;\n");
			if(tmp==0) break;
			if(1) {
				if(strstr(tmp, ".gif") || strstr(tmp, ".jpg") || strstr(tmp, ".bmp")) {
					printf("<IMG SRC='%s'>", nohtml(tmp));
					tmp=strtok(0, "");
					if(tmp==0) return;
					return hhprintf(tmp);
				}
			}
			printf("<a target=_blank href='%s'>%s</a>", nohtml(tmp), nohtml(tmp));
			tmp=strtok(0, "");
			if(tmp==0) return printf("\n");
			return hhprintf(tmp);
		} else {
			buf0[len]=s[0];
			if(len<1000) len++;
			s++;
		}
	}
}

int parm_add(char *name, char *val) {
	int len=strlen(val);
	if(parm_num>=255) http_fatal("too many parms.");
	parm_val[parm_num]=calloc(len+1, 1);
	if(parm_val[parm_num]==0) http_fatal("memory overflow2 %d %d", len, parm_num);
	strsncpy(parm_name[parm_num], name, 78);
	strsncpy(parm_val[parm_num], val, len+1);
	parm_num++;
}

int http_init() {
	char *buf, buf2[1024], *t2, *t3;
	int n;
#ifndef SILENCE
	printf("Content-type: text/html; charset=%s\n\n\n", CHARSET);
	printf("<html>\n");
	printf("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=%s\">\n", CHARSET);
#ifndef MY_CSS
	printf("<link rel=\"stylesheet\" type=\"text/css\" href=\"%s\">\n", CSS_FILE);
#else
	printf("<link rel=\"stylesheet\" type=\"text/css\" href=\"%s\">\n", MY_CSS);
#endif
#endif
	n=atoi(getsenv("CONTENT_LENGTH"));
	if(n>5000000) n=5000000;
	buf=calloc(n+1, 1);
	if(buf==0) http_fatal("memory overflow");
	fread(buf, 1, n, stdin);
	buf[n]=0;
	t2=strtok(buf, "&");
	while(t2) {
		t3=strchr(t2, '=');
		if(t3!=0) {
			t3[0]=0;
			t3++;
			__unhcode(t3);
			parm_add(trim(t2), t3);
		}
		t2=strtok(0, "&");
	}
	strsncpy(buf2, getsenv("QUERY_STRING"), 1024);
	t2=strtok(buf2, "&");
	while(t2) {
		t3=strchr(t2, '=');
		if(t3!=0) {
			t3[0]=0;
			t3++;
			__unhcode(t3);
			parm_add(trim(t2), t3);
		}
		t2=strtok(0, "&");
	}
	strsncpy(buf2, getsenv("HTTP_COOKIE"), 1024);
	//printf("HTTP_COOKIE = %s\n", buf2);
	t2=strtok(buf2, ";");
	while(t2) {
		t3=strchr(t2, '=');
		if(t3!=0) {
			t3[0]=0;
			t3++;
			parm_add(trim(t2), t3);
		}
		t2=strtok(0, ";");
	}
	strsncpy(fromhost, getsenv("REMOTE_ADDR"), 32);
}

int __to16(char c) {
	if(c>='a'&&c<='f') return c-'a'+10;
	if(c>='A'&&c<='F') return c-'A'+10;
	if(c>='0'&&c<='9') return c-'0';
	return 0;
}

int __unhcode(char *s) {
	int m, n;
	for(m=0, n=0; s[m]!=0; m++, n++) {
		if(s[m]=='+') {
			s[n]=' ';
			continue;
		}
		if(s[m]=='%') {
			s[n]=__to16(s[m+1])*16+__to16(s[m+2]);
			m+=2;
			continue;
		}
		s[n]=s[m];
	}
	s[n]=0;
}

char *getparm(char *var) {
	int n;
	for(n=0; n<parm_num; n++) 
		if(!strcasecmp(parm_name[n], var)) return parm_val[n];
	return "";
}

int get_shmkey(char *s) {
        int n=0;
        while(shmkeys[n].key!=0) {
                if(!strcasecmp(shmkeys[n].key, s)) return shmkeys[n].value;
                n++;
        }
        return 0;
}

/* smh_init() ÐèÒªÐÞ¸Ä */
int shm_init()
{
	resolve_ucache();
	resolve_utmp();
	resolve_boards(); 
}

int user_init(struct userec *x, struct user_info **y)
{
	struct userec *x2 = NULL;
	char id[20], num[20];
	int i, uid, key;
	struct UTMPFILE *utmpshm_ptr;

	strsncpy(id, getparm("utmpuserid"), 13);
	strsncpy(num, getparm("utmpnum"), 12);
	//printf("utmpuserid = %s\n", id);
	//printf("utmpnum = %s\n", num);
	key=atoi(getparm("utmpkey"));
	//printf("utmpkey = %d\n", key);
	i=atoi(num);
	if(i<1 || i>=MAXACTIVE) return 0;
	//utmpshm_ptr = get_utmpshm_addr();
	/* ÕâÀïÓÐÎÊÌâ, (*y)ÔÚºóÃæ½«Ò»Ö±Ö¸Ïò&(utmpshm_ptr->uinfo[i]),
	 * ²»¹ÜºóÃæµÄÄÇÐ©ÅÐ¶ÏÊÇ·ñ³É¹¦ */
	(*y) = get_user_info(i);
	if(strncmp((*y)->from, fromhost, 24))
	{
		//printf("from is ->%s<-, len = %d\n", (*y)->from, strlen((*y)->from));
		//printf("fromhost is ->%s<-, len = %d\n", fromhost, strlen(fromhost));
		//printf("fromhost error!\n");
		return 0;
	}
	if((*y)->utmpkey != key)
	{
		//printf("utmpkey error!\n");
		return 0;
	}
	
	if((*y)->active == 0)
	{
		//printf("user not active!\n");
		return 0;
	}
	if((*y)->userid[0] == 0)
	{
		//printf("userid error!\n");
		return 0;
	}
	if((*y)->mode!=10001)
	{
		//printf("mode error!\n");
		return 0; /* faint, what does 10001 mean? */
	}
	if(!strcasecmp((*y)->userid, "new") || !strcasecmp((*y)->userid, "guest"))
		return 0;
	getuser((*y)->userid, &x2);
	if(x2==0)
	{
		//printf("getuser error!\n");
		return 0;
	}
	if(strcmp(x2->userid, id))
	{
		//printf("userid not equal!\n");
		return 0;
	}
	memcpy(x, x2, sizeof(*x));
	return 1;
}

int post_mail(char *userid, char *title, char *file, char *id, char *nickname, char *ip, int sig) {
	FILE *fp, *fp2;
	char buf3[256], dir[256];
	struct fileheader header;
	int t, i;
	if(strstr(userid, "@")) 
		return post_imail(userid, title, file, id, nickname, ip, sig);
	bzero(&header, sizeof(header));
	strcpy(header.owner, id);
	for(i=0; i<100; i++) {
		t=time(0)+i;
		sprintf(buf3, "mail/%c/%s/M.%d.A", toupper(userid[0]), userid, i+time(0));
		if(!file_exist(buf3)) break;
	}
	if(i>=99) return -1;
	sprintf(header.filename, "M.%d.A", t);
	strsncpy(header.title, title, 60);
	fp=fopen(buf3, "w");
	if(fp==0) return -2;
	fp2=fopen(file, "r");
	fprintf(fp, "¼ÄÐÅÈË: %s (%s)\n", id, nickname);
	fprintf(fp, "±ê  Ìâ: %s\n", title);
	fprintf(fp, "·¢ÐÅÕ¾: %s (%s)\n", BBSNAME, Ctime(time(0)));
	fprintf(fp, "À´  Ô´: %s\n\n", ip);
	if(fp2) {
		while(1) {
			if(fgets(buf3, 256, fp2)<=0) break;
			fprintf2(fp, buf3);
		}
		fclose(fp2);
	}
	fprintf(fp, "\n--\n");
	sig_append(fp, id, sig);
	fprintf(fp, "\n\n[1;%dm¡ù À´Ô´:£®%s %s£®[FROM: %.20s][m\n", 31+rand()%7, BBSNAME, NAME_BBS_ENGLISH, ip);
	fclose(fp);
        sprintf(dir, "mail/%c/%s/.DIR", toupper(userid[0]), userid);
        fp=fopen(dir, "a");
	if(fp==0) return -1;
        fwrite(&header, sizeof(header), 1, fp);
        fclose(fp);
}

int post_imail(char *userid, char *title, char *file, char *id, char *nickname, char *ip, int sig) {
        FILE *fp1, *fp2;
        char buf[256];
	if(strstr(userid, ";") || strstr(userid, "`")) http_fatal("´íÎóµÄÊÕÐÅÈËµØÖ·");
	sprintf(buf, "sendmail -f %s.bbs@%s '%s'", id, BBSHOST, userid);
        fp2=popen(buf, "w");
        fp1=fopen(file, "r");
        if(fp1==0 || fp2==0) return -1;
        fprintf(fp2, "From: %s.bbs@%s\n", id, BBSHOST);
        fprintf(fp2, "To: %s\n", userid);
        fprintf(fp2, "Subject: %s\n\n", title);
        while(1) {
		if(fgets(buf, 255, fp1)==0) break;
		if(buf[0]=='.' && buf[1]=='\n') continue;
                fprintf(fp2, "%s", buf);
        }
	fprintf(fp2, "\n--\n");
 	sig_append(fp2, id, sig);
 	fprintf(fp2, "\n\n[1;%dm¡ù À´Ô´:£®%s %s£®[FROM: %.20s][m\n", 31+rand()%7, BBSNAME, NAME_BBS_ENGLISH, ip);
        fprintf(fp2, ".\n");
        fclose(fp1);
        pclose(fp2);
}

int check_readonly(checked) /* Leeward 98.03.28 */
char *checked;          /* ¸Ä¶¯±¾º¯Êý±ØÐëÍ¬²½ bbs.c ºÍ bbssnd.c (4 WWW) */
{
    struct stat st;
    char        buf[STRLEN];

    sprintf(buf, "boards/%s", checked);
    stat(buf, &st);
    if (365 == (st.st_mode & 0X1FF)) /* Checking if DIR access mode is "555" */
        return YEA;
    else
        return NA;
}

int deny_me(char *board)   /* ÅÐ¶Ïµ±Ç°ÓÃ»§ ÊÇ·ñ±»½ûÖ¹·¢±íÎÄÕÂ */
{
    char buf[STRLEN];

    setbfile(buf, board, "deny_users");
    return seek_in_file(buf, getcurruserid());
}

int outgo_post(struct fileheader *fh, char *board, 
				struct userec *user, char *title)
{
    FILE *foo;

    if (foo = fopen("innd/out.bntp", "a"))
    {
        fprintf(foo, "%s\t%s\t%s\t%s\t%s\n", board,
                fh->filename, user->userid, user->username, title);
        fclose(foo);
    }
}

void add_loginfo2(char *filepath, char *board, struct userec *user, int anony)
{
	FILE *fp;
    FILE *fp2;
    int color;
    char fname[STRLEN];

    //noidboard=(seek_in_file("etc/anonymous",board)&&anony);
    color=(user->numlogins%7)+31; /* ÑÕÉ«Ëæ»ú±ä»¯ */
    setuserfile( fname, "signatures" );
    fp=fopen(filepath,"a");
    if ((fp2=fopen(fname, "r"))== NULL|| /* ÅÐ¶ÏÊÇ·ñÒÑ¾­ ´æÔÚ Ç©Ãûµµ */
            user->signature==0 || anony==1)
    {
		fputs("\n--\n", fp);
    }
	else
	{ /*Bigman 2000.8.10ÐÞ¸Ä,¼õÉÙ´úÂë */
        fprintf(fp,"\n");
	}
    /* ÓÉBigmanÔö¼Ó:2000.8.10 Announce°æÄäÃû·¢ÎÄÎÊÌâ */
    if (!strcmp(board,"Announce"))
        fprintf(fp, "[m[%2dm¡ù À´Ô´:¡¤%s %s¡¤[FROM: %s][m\n",
				color, "BBS "NAME_BBS_CHINESE"Õ¾", NAME_BBS_ENGLISH,
                NAME_BBS_CHINESE" BBSÕ¾");
    else
        fprintf(fp, "\n[m[%2dm¡ù À´Ô´:¡¤%s %s¡¤[FROM: %s][m\n",
				color, "BBS "NAME_BBS_CHINESE"Õ¾", NAME_BBS_ENGLISH,
				(anony)?NAME_ANONYMOUS_FROM:user->lasthost);

    if (fp2)
		fclose(fp2);
    fclose(fp);
    return;
}

void addsignature2(FILE *fp, struct userec *user, int sig)
{
    FILE *sigfile;
    int  i,valid_ln=0;
    char tmpsig[MAXSIGLINES][256];
    char inbuf[256];
    char fname[STRLEN];
    char tmp[STRLEN];

	if (sig == 0)
		return;
    setuserfile( fname, "signatures" );
    if ((sigfile = fopen(fname, "r"))== NULL)
    	return;
    fputs("--\n", fp);
    for (i=1; i<=(sig-1)*MAXSIGLINES && sig!=1; i++)
    {
        if (!fgets(inbuf, sizeof(inbuf), sigfile))
		{
            fclose(sigfile);
            return;
		}
    }
    for (i=1; i<=MAXSIGLINES; i++)
	{
        if (fgets(inbuf, sizeof(inbuf), sigfile))
        {
            if(inbuf[0]!='\n')
                valid_ln = i;
            strcpy(tmpsig[i-1],inbuf);
        }
        else
			break;
    }
    fclose(sigfile);
    for(i=1;i<=valid_ln;i++)
        fputs(tmpsig[i-1], fp);
	user->signature = sig;
	save_user_data(user);
}

void write_header2(FILE *fp, char *board, char *title, 
					struct userec *user, int anony)
{
	if (!strcmp(board,"Announce"))
		fprintf(fp,"·¢ÐÅÈË: %s (%s), ÐÅÇø: %s       \n",
				"SYSOP", NAME_SYSOP, board) ;
	else
		fprintf(fp,"·¢ÐÅÈË: %s (%s), ÐÅÇø: %s       \n",
				anony ? board : user->userid,
				anony ? NAME_ANONYMOUS : user->username,
				board) ;
	fprintf(fp, 
			"±ê  Ìâ: %s\n·¢ÐÅÕ¾: %s (%24.24s)\n\n",
			title, "BBS "NAME_BBS_CHINESE"Õ¾", Ctime(time(0)));
}

// fp 		for destfile
// fp2		for srcfile
int write_file2(FILE *fp, FILE *fp2)
{
	char buf3[1024];

	while(1)
	{
		if(fgets(buf3, 1000, fp2) == NULL)
			break;
		fprintf2(fp, buf3);
	}
}

// return value:
// >0		success
// -1		write .DIR failed
int
post_article(char *board, char *title, char *file, struct userec *user,
		char *ip, int sig, int local_save, int anony)
{
    struct fileheader post_file ;
    char        filepath[STRLEN], fname[STRLEN];
    char        buf[256];
    int         fd, anonyboard;
    time_t      now;
	FILE *fp, *fp2;

    memset(&post_file,0,sizeof(post_file)) ;
    anonyboard = seek_in_file("etc/anonymous",board); /* ÊÇ·ñÎªÄäÃû°æ */

    /* ×Ô¶¯Éú³É POST ÎÄ¼þÃû */
    now = time(NULL);
    sprintf(fname,"M.%d.A", now) ;
    setbfile( filepath, board, fname );
    while((fd = open(filepath,O_CREAT|O_EXCL|O_WRONLY,0644)) == -1)
	{
        now++;
        sprintf(fname,"M.%d.A",now) ;
        setbfile(filepath, board, fname);
    }
    close(fd) ;
    strcpy(post_file.filename,fname) ;

	anony = anonyboard && anony;
    strncpy(post_file.owner, anony ? board:getcurruserid(), STRLEN);

    if ((!strcmp(board,"Announce"))&&(!strcmp(post_file.owner,board)))
    	strcpy(post_file.owner,"SYSOP");

	fp = fopen(filepath, "w");
	fp2 = fopen(file, "r");
	write_header2(fp, board, title, user, anony);
	write_file2(fp, fp2);
	fclose(fp2);
	if (!anony)
		addsignature2(fp, user, sig);
	fclose(fp);
    add_loginfo2(filepath, board, user, anony); /*Ìí¼Ó×îºóÒ»ÐÐ*/

    strncpy( post_file.title, title, STRLEN );
    if ( local_save == 1 ) /* local save */
    {
        post_file.filename[ STRLEN - 1 ] = 'L';
        post_file.filename[ STRLEN - 2 ] = 'L';
    }
	else
    {
        post_file.filename[ STRLEN - 1 ] = 'S';
        post_file.filename[ STRLEN - 2 ] = 'S';
        outgo_post(&post_file, board, user, title);
    }

    setbfile( buf, board, DOT_DIR);

    /* ÔÚboards°æ°æÖ÷·¢ÎÄ×Ô¶¯Ìí¼ÓÎÄÕÂ±ê¼Ç Bigman:2000.8.12*/
    if (!strcmp(board, "Board")  && !has_perm(PERM_OBOARDS)
		&& has_perm(PERM_BOARDS) )
    {
        post_file.accessed[0] |= FILE_SIGN;
    }

    if (append_record( buf, &post_file, sizeof(post_file)) == -1)
	{ /* Ìí¼ÓPOSTÐÅÏ¢ µ½ µ±Ç°°æ.DIR */
        sprintf(buf, "posting '%s' on '%s': append_record failed!",
                post_file.title, board);
        report(buf);
		// ±ØÐë°Ñ¸Õ¸Õ´´½¨µÄÎÄ¼þÉ¾³ý²ÅÐÐ
		unlink(filepath);
        return -1 ;
    }

    sprintf(buf,"posted '%s' on '%s'", post_file.title, board) ;
    report(buf) ; /* log */
    /*      postreport(post_file.title, 1);*/ /*added by alex, 96.9.12*/
    //if ( !junkboard(board) )
    //{
    //    currentuser.numposts++;
    //}
    return now;
}

int sig_append(FILE *fp, char *id, int sig) {
	FILE *fp2;
	char path[256];
	char buf[100][256];
	int i, total;
	struct userec *x = NULL;
	if(sig<0 || sig>10) return;
#ifndef SMTH
	x=getuser(id);
#else /* SMTH */
	getuser(id, &x);
#endif /* SMTH */
	if(x==0) return;
	sprintf(path, "home/%c/%s/signatures", toupper(id[0]), id);
	fp2=fopen(path, "r");
	if(fp2==0) return;
	for(total=0; total<255; total++)
		if(fgets(buf[total], 255, fp2)==0) break;
	fclose(fp2);
	for(i=sig*6; i<sig*6+6; i++) {
		if(i>=total) break;
		fprintf(fp, "%s", buf[i]);
	}
}

char* anno_path_of(char *board) {
	FILE *fp;
	static char buf[256], buf1[80], buf2[80];
	fp=fopen("0Announce/.Search", "r");
	if(fp==0) return "";
	while(1) {
		if(fscanf(fp, "%s %s", buf1, buf2)<=0) break;
		buf1[strlen(buf1)-1]=0;
		if(!strcasecmp(buf1, board)) {
			sprintf(buf, "/%s", buf2);
			return buf;
		}
	}
	fclose(fp);
	return "";
}

int has_BM_perm(struct userec *user, char *board) {
	bcache_t *x;
	char buf[256], *bm;

	x=getbcache(board);
	if(x==0) return 0;
	if(user_perm(user, PERM_BLEVELS)) return 1;
	if(!user_perm(user, PERM_BOARDS)) return 0;
	strcpy(buf, x->BM);
	bm=strtok(buf, ",: ;&()\n");
	while(bm) {
		if(!strcasecmp(bm, user->userid)) return 1;
		bm=strtok(0, ",: ;&()\n");
	}
	return 0;
}

int has_read_perm(struct userec *user, char *board) {  
	bcache_t *x;	/* °æÃæ²»´æÔÚ·µ»Ø0, pºÍz°æÃæ·µ»Ø1, ÓÐÈ¨ÏÞ°æÃæ·µ»Ø1. */

	if(board[0]<=32) return 0;
	x=getbcache(board);
	if(x==0) return 0; 
	if(x->level==0) return 1;
	if(x->level & (PERM_POSTMASK | PERM_NOZAP)) return 1;
	if(!user_perm(user, PERM_BASIC)) return 0;
	/*if(user_perm(user, PERM_SPECIAL8)) return 0;*/
	if(user_perm(user, x->level)) return 1;
	return 0;
}

int has_post_perm(struct userec *user, char *board)
{
	return haspostperm(board);
}

int count_mails(char *id, int *total, int *unread) {
        struct fileheader x1;
	char buf[256];
        int n;
        FILE *fp;
	*total=0;
	*unread=0;
		{
			struct userec *x;
			if (getuser(id, &x) == 0)
				return 0;
		}
        sprintf(buf, "%s/mail/%c/%s/.DIR", BBSHOME, toupper(id[0]), id);
        fp=fopen(buf, "r");
        if(fp==0) return;
        while(fread(&x1, sizeof(x1), 1, fp)>0) {
                (*total)++;
                if(!(x1.accessed[0] & FILE_READ)) (*unread)++;
        }
        fclose(fp);
}

int findnextutmp(char *id, int from) {
	int i;
	if(from<0) from=0;
	for(i=from; i<MAXACTIVE; i++) 
		if(shm_utmp->uinfo[i].active)
			if(!strcasecmp(shm_utmp->uinfo[i].userid, id)) return i;
	return -1;
}

int setmsgfile(char *buf, char *id)
{
	if (buf == NULL || id == NULL)
		return -1;
	sethomefile(buf, id, SYS_MSGFILE);
	return 0;
}

int setmsgfilelog(char *buf, char *id)
{
	if (buf == NULL || id == NULL)
		return -1;
	sethomefile(buf, id, SYS_MSGFILELOG);
	return 0;
}

struct _tag_t_search {
	struct user_info* result;
	int pid;
};

int _t_search(struct user_info* uentp,struct _tag_t_search* data,int pos)
{
	if (data->pid==0) {
		data->result=uentp;
		return QUIT;
	}
	data->result=uentp;
	if (uentp->pid==data->pid)
		return QUIT;
	UNUSED_ARG(pos);
	return 0;
}

struct user_info *
            t_search(sid,pid)
            char *sid;
int  pid;
{
    int         i;
    struct _tag_t_search data;

    data.pid=pid;
    data.result=NULL;

    apply_utmp(_t_search,20,sid,&data);
    
    return data.result;
}

int
cmpfnames(userid, uv)
char    *userid;
struct friends *uv;
{
    return !strcasecmp(userid, uv->id);
}

int
can_override( userid, whoasks )
char *userid;
char *whoasks;
{
    struct friends fh;
	char buf[STRLEN];

    sethomefile( buf, userid, "friends" );
    return (search_record( buf, &fh, sizeof(fh), cmpfnames, whoasks )>0)?YEA:NA;
}

int
canmsg(uin)
struct user_info *uin;
{
    if ((uin->pager&ALLMSG_PAGER) || has_perm(PERM_SYSOP))
		return YEA;
    if ((uin->pager&FRIENDMSG_PAGER))
    {
        if(can_override(uin->userid, getcurruserid()))
            return YEA;
    }
    return NA;
}

int cmpinames(const char *userid,const  char *uv)       /* added by Luzi 1997.11.28 */
{
    return !strcasecmp(userid, uv);
}

int canIsend2(userid) /* Leeward 98.04.10 */
char *userid;
{
    char buf[IDLEN+1];
    char path[256];

    if (has_perm(PERM_SYSOP)) return YEA;

    sethomefile( path, userid , "ignores");
    if (search_record(path, buf, IDLEN+1, cmpinames, getcurruserid()))
        return NA;
    sethomefile( path, userid , "bads");
    if (search_record(path, buf, IDLEN+1, cmpinames, getcurruserid()))
        return NA;
    else
        return YEA;
}

int do_sendmsg2(uentp,msgstr)
struct user_info *uentp;
char msgstr[256];
{
    char uident[STRLEN];
    FILE *fp;
    time_t now;
    struct user_info *uin ;
    char buf[80],msgbuf[256] ,*timestr,msgbak[256];
    int msg_count=0;
    int Gmode = 0;
	struct userec *u;
	uinfo_t *ui;

	ui = getcurruinfo();
    *msgbak = 0;	/* period 2000-11-20 may be used without init */
	if (uentp == NULL)
		return -1;
	uin = uentp;
    if(LOCKSCREEN == uin->mode) /* Leeward 98.02.28 */
        return -1 ;	/* dest user is in lock screen mode */

    if (NA==canIsend2(uin->userid))/*Haohmaru.06.06.99.¼ì²é×Ô¼ºÊÇ·ñ±»ignore*/
        return -1;	/* ¶Ô·½¾Ü¾øÄãµÄÑ¶Ï¢ */

	sethomefile(buf,uident,"msgcount");
	fp=fopen(buf, "rb");
	if (fp!=NULL)
	{
		fread(&msg_count,sizeof(int),1,fp);
		fclose(fp);

		if(msg_count>MAXMESSAGE)
			return -1 ;	/* ¶Ô·½ÉÐÓÐÒ»Ð©Ñ¶Ï¢Î´´¦Àí */
	}
    if(msgstr==NULL)
		return -1;

	strcpy(uident, uin->userid);
	u = getcurrusr();
    now=time(0);
    timestr=ctime(&now)+11;
    *(timestr+8)='\0';
	sprintf(msgbuf,
			"[44m[36m%-12.12s[33m(%-5.5s):[37m%-59.59s[m[%dm\033[%dm\n", 
			u->userid, timestr, 
			msgstr,ui->pid+100,uin->pid+100);
	sprintf(msgbak,
			"[44m[0;1;32m=>[37m%-10.10s[33m(%-5.5s):[36m%-59.59s[m[%dm\033[%dm\n",
			uident, timestr, 
			msgstr,ui->pid+100,uin->pid+100);
	uin = t_search(uin->userid, uin->pid);

    if ((uin == NULL) || (uin->active == 0)
		|| (uin->pid == 0) || (kill(uin->pid, 0) !=0))
        return -2; /* ¶Ô·½ÒÑ¾­ÀëÏß */

    sethomefile(buf,uident,"msgfile");
    if((fp=fopen(buf,"a"))==NULL)
        return -1;
    fputs(msgbuf,fp);
    fclose(fp);

    /*Haohmaru.99.6.03.»ØµÄmsgÒ²¼ÇÂ¼*/
    if(strcmp(u->userid,uident))
	{
        sethomefile(buf,u->userid,"msgfile");
        if((fp=fopen(buf,"a"))==NULL)
            return -1;
        fputs(msgbak,fp);
        fclose(fp);
    }
    if(kill(uin->pid,SIGUSR2)==-1)
        return -1;
    sethomefile(buf,uident,"msgcount");
    fp=fopen(buf, "wb");
    if (fp!=NULL)
    {
        msg_count++;
        fwrite(&msg_count,sizeof(int),1,fp);
        fclose(fp);
    }
    return 0 ;
}

int send_msg(char *myuserid, int mypid, char *touserid, int topid, char msg[256])
{
	int i;
	uinfo_t *uin;

	/* ÂËµôÌØÊâ×Ö·û£¬Ó¦¸ÃÐ´³ÉÒ»¸öº¯Êý */
	for(i=0; i<strlen(msg); i++)
		if((0<msg[i] && msg[i]<=27 )|| msg[i]==-1) msg[i]=32;

	uin = t_search(touserid, topid);
	return do_sendmsg2(uin, msg);
}

int count_life_value(struct userec *urec) {
	int i;
	i=(time(0) - urec->lastlogin)/60;
	if(urec->userlevel & PERM_SPECIAL8)
		return (360*24*60-i)/1440;
	if((urec->userlevel & PERM_XEMPT)||!strcasecmp(urec->userid, "guest"))
        	return 999;
	if(urec->numlogins<=3)
		return (45*1440-i)/1440;
	if(!(urec->userlevel & PERM_LOGINOK))
		return (90*1440-i)/1440;
	return (180*1440-i)/1440;
}

int modify_mode(struct user_info *x, int newmode) {
	if(x==0) return;
	x->mode=newmode;
}

int save_user_data(struct userec *x)
{
	int n;

	n = getusernum(x->userid);
	update_user(x, n, 0);
	return 1;
}

int is_bansite(char *ip) {
	FILE *fp;
	char buf3[256];
	fp=fopen(".bansite", "r");
	if(fp==0) return 0;
	while(fscanf(fp, "%s", buf3)>0)
		if(!strcasecmp(buf3, ip)) return 1;
	fclose(fp);
	return 0;
}

int user_perm(struct userec *x, int level) {
	return (x->userlevel & level);
}

int getusernum(char *id)
{
	return searchuser(id);
}

int checkuser(char *id, char *pw) {
	int i;
	struct userec *x = NULL;
	getuser(id, &x);
	if(x == NULL) return 0;
	return checkpasswd2(pw, x);
}

int count_id_num(char *id) {
	int i, total=0;
	for(i=0; i<MAXACTIVE; i++)
		if(shm_utmp->uinfo[i].active && !strcasecmp(shm_utmp->uinfo[i].userid, id)) total++;
	return total;
}

int count_online2() {
	int i, total=0;
	for(i=0; i<MAXACTIVE; i++)
		if(shm_utmp->uinfo[i].active && shm_utmp->uinfo[i].invisible==0) total++;
	return total;
}

int loadfriend(char *id) {
        FILE *fp;
        char file[256];
        if(!loginok) return;
        sprintf(file, "home/%c/%s/friends", toupper(id[0]), id);
        fp=fopen(file, "r");
        if(fp) {
                friendnum=fread(fff, sizeof(fff[0]), 200, fp);
                fclose(fp);
        }
}

int isfriend(char *id)
{
	return myfriend(searchuser(id), NULL);
}

int loadbad(char *id) {
        FILE *fp;
        char file[256];
        if(!loginok) return;
        sprintf(file, "home/%c/%s/rejects", toupper(id[0]), id);
        fp=fopen(file, "r");
        if(fp) {
                badnum=fread(fff, sizeof(fff[0]), MAXREJECTS, fp);
                fclose(fp);
        }
}

int isbad(char *id) {
        static inited=0;
        int n;
        if(!inited) {
                loadbad(currentuser.userid);
                inited=1;
        }
        for(n=0; n<badnum; n++)
                if(!strcasecmp(id, bbb[n].id)) return 1;
        return 0;
}

int init_all() {
	srand(time(0)*2+getpid());
	chdir(BBSHOME);
	http_init();
	/*seteuid(BBSUID);*/
	/*if(geteuid()!=BBSUID) http_fatal("uid error.");*/
	shm_init();
	loginok=user_init(&currentuser, &u_info);
}

int init_no_http() {
	srand(time(0)+getpid());
	chdir(BBSHOME);
	shm_init();
}

char *void1(unsigned char *s) {
        int i;
        int flag=0;
        for(i=0; s[i]; i++) {
                if(flag==0) {
                        if(s[i]>=128) flag=1;
                        continue;
                }
                flag=0;
                if(s[i]<32) s[i-1]=32;
        }
        if(flag) s[strlen(s)-1]=0;
        return s;
}

char *sec(char c) {
	int i;
	for(i=0; i<SECNUM; i++) {
		if(strchr(seccode[i], c)) return secname[i][0];
	}
	return "(unknown.)";
}

char *flag_str(int access) {
	static char buf[80];
	char *flag2="";
	strcpy(buf, "  ");
	if(access & FILE_DIGEST) flag2="G";
	if(access & FILE_MARKED) flag2="M";
	if((access & FILE_MARKED) && (access & FILE_DIGEST)) flag2="B";
	sprintf(buf, "%s", flag2);
	return buf;
}

char *flag_str2(int access, int has_read) {
        static char buf[80];
	strcpy(buf, "   ");
        if(loginok) strcpy(buf, "N  ");
        if(access & FILE_DIGEST) buf[0]='G';
        if(access & FILE_MARKED) buf[0]='M';
        if((access & FILE_MARKED) && (access & FILE_DIGEST)) buf[0]='B';
	if(has_read) buf[0]=tolower(buf[0]);
	if(buf[0]=='n') buf[0]=' ';
        return buf;
}

char *userid_str(char *s) {
	static char buf[512];
	char buf2[256], tmp[256], *ptr, *ptr2;
	strsncpy(tmp, s, 255);
	buf[0]=0;
	ptr=strtok(tmp, " ,();\r\n\t");
	while(ptr && strlen(buf)<400) {
		if(ptr2=strchr(ptr, '.')) {
			ptr2[1]=0;
			strcat(buf, ptr);
			strcat(buf, " ");
		} else {
			ptr=nohtml(ptr);
			sprintf(buf2, "<a href=bbsqry?userid=%s>%s</a> ", ptr, ptr);
			strcat(buf, buf2);
		}
		ptr=strtok(0, " ,();\r\n\t");
	}
	return buf;
}

int fprintf2(FILE *fp, char *s) {
	int i, tail=0, sum=0;
	if(s[0]==':' && s[1]==' ' && strlen(s)>79) {
		sprintf(s+76, "..\n");
		fprintf(fp, "%s", s);
		return;
	}
	for(i=0; s[i]; i++) {
		fprintf(fp, "%c", s[i]);
		sum++;
		if(tail) {
			tail=0;
		} else if(s[i]<0) {
			tail=s[i];
		}
		if(sum>=78 && tail==0) {
			fprintf(fp, "\n");
			sum=0;
		}
	}
}

#if 0
int fprintf2(FILE *fp, char *s)
{
	unsigned char *ppt = s; /* ÕÛÐÐ´¦ */
	unsigned char *pp  = ppt; /* ÐÐÊ× */
	unsigned int  LLL = 78; /* ÕÛÐÐÎ»ÖÃ */
	unsigned char *ppx, cc;
	int  ich, lll;

	while (strlen(pp) > LLL)
	{
		lll = 0; ppx = pp; ich = 0;
		do
		{
			if (ppx = strstr(ppx, "["))
			{
				ich = (int)strchr(ppx, 'm') - (int)ppx;
				if (ich > 0)
					ich ++;
				else
					ich = 2;
				lll += ich;
				ppx += 2;
				ich = 0;
			}
		} while (ppx);
		ppt += LLL + lll;

		if (*ppt > 127)
		{ /* ±ÜÃâÔÚºº×ÖÖÐ¼äÕÛÐÐ */
			for (ppx = ppt - 1, ich = 0; ppx >= pp; ppx --)
			{
				if (*ppx < 128)
					break; 
				else 
					ich ++;
			}
			if (ich % 2) 
				ppt --;
		}
		else if (*ppt)
		{
			for (ppx = ppt - 1, ich = 0; ppx >= pp; ppx --)
			{
				if (*ppx > 127 || ' ' == *ppx)
					break;
				else
					ich ++;
			}
			if (ppx > pp && ich < 16)
				ppt -= ich;
		}

		cc = *ppt;
		*ppt = 0;
		if (':' == s[0] && ':' != *pp)
			fprintf(fp, ": ");
		fprintf(fp,"%s",pp) ;
		if (cc)
			fprintf(fp, "\n");
		*ppt = cc;
		pp = ppt;
	}
	if (':' == s[0] && ':' != *pp)
		fprintf(fp, ": ");
	fprintf(fp,"%s\n",pp) ;

	return 0;
}
#endif

struct fileheader *get_file_ent(char *board, char *file) {
	FILE *fp;
	char dir[80];
	static struct fileheader x;
	int num=0;
	sprintf(dir, "boards/%s/.DIR", board);
	fp=fopen(dir, "r");
	while(1) {
		if(fread(&x, sizeof(x), 1, fp)<=0) break;
		if(!strcmp(x.filename, file)) {
			fclose(fp);
			return &x;
		}
		num++;
	}
	fclose(fp);
	return 0;
}

char *getbfroma(char *path) {
        FILE *fp;
	static char buf1[180], buf2[180];
	int len;
	if(path[0]==0) return "";
	path++;
        fp=fopen("0Announce/.Search", "r");
        if(fp==0) return "";
        while(1) {
		bzero(buf1, 80);
		bzero(buf2, 80);
                if(fscanf(fp, "%s %s", buf1, buf2)<=0) break;
                if(buf1[0]) buf1[strlen(buf1)-1]=0;
		if(buf1[0]=='*') continue;
                if(!strncmp(buf2, path, strlen(buf2))) return buf1;
        }
        fclose(fp);
        return "";
}

int set_my_cookie() {
	FILE *fp;
	char path[256], buf[256], buf1[256], buf2[256];
	int my_t_lines=20, my_link_mode=0, my_def_mode=0;
	sprintf(path, "home/%c/%s/.mywww", toupper(currentuser.userid[0]), currentuser.userid);
 	fp=fopen(path, "r");
 	if(fp) {
 		while(1) {
 			if(fgets(buf, 80, fp)==0) break;
 			if(sscanf(buf, "%80s %80s", buf1, buf2)!=2) continue;
 			if(!strcmp(buf1, "t_lines")) my_t_lines=atoi(buf2);
 			if(!strcmp(buf1, "link_mode")) my_link_mode=atoi(buf2);
			if(!strcmp(buf1, "def_mode")) my_def_mode=atoi(buf2);
 		}
 		fclose(fp);
 	}
	sprintf(buf, "%d", my_t_lines);
	setcookie("my_t_lines", buf);
	sprintf(buf, "%d", my_link_mode);
	setcookie("my_link_mode", buf);
	sprintf(buf, "%d", my_def_mode);
	setcookie("my_def_mode", buf);
}

int has_fill_form() {
        FILE *fp;
        int r;
        char userid[256], tmp[256], buf[256], *ptr;
        fp=fopen("new_register", "r");
        if(fp==0) return 0;
        while(1) {
                if(fgets(buf, 100, fp)==0) break;
                r=sscanf(buf, "%s %s", tmp, userid);
                if(r==2) {
                        if(!strcasecmp(tmp, "userid:") && !strcasecmp(userid, currentuser.userid)) {
                                fclose(fp);
                                return 1;
                        }
                }
        }
        fclose(fp);
        return 0;
}


time_t get_exit_time(id,exittime) /* »ñÈ¡ÀëÏßÊ±¼ä£¬id:ÓÃ»§ID,
                                   exittime:±£´æ·µ»ØµÄÊ±¼ä£¬½áÊø·ûÎª\n
                                            ½¨Òé¶¨ÒåÎª char exittime[40]
                                   Luzi 1998/10/23 */
/* Leeward 98.10.26 add return value: time_t */
char *id;
char *exittime;
{
    char path[80];
    FILE *fp;
    time_t now = 1; /* if fopen failed return 1 -- Leeward */
    sethomefile( path, id , "exit");
    fp=fopen(path, "rb");
    if (fp!=NULL)
    {
        fread(&now,sizeof(time_t),1,fp);
        fclose(fp);
        strcpy(exittime, ctime(&now));
    }
    else exittime[0]='\n';

    return now;
}

/*
 * Return value:
 *      0       Success
 *     -1       so called "bad arg1"
 *     -2       so called "bad arg2"
 */
int valid_filename(char *file)
{
	if (file == NULL)
		return -1;
	if(strncmp(file, "M.", 2) && strncmp(file, "G.", 2))
		/*http_fatal("´íÎóµÄ²ÎÊý1");*/
		return -1;
	if(strstr(file, "..") || strstr(file, "/"))
		/*http_fatal("´íÎóµÄ²ÎÊý2");*/
		return -2;
	return 0;
}

/* merge boardrc.c into bbslib.c */
char *brc_getrecord(char *ptr, char *name, int *pnum, int *list) {
        int     num;
        char   *tmp;
        strncpy(name, ptr, BRC_STRLEN);
        ptr += BRC_STRLEN;
        num = (*ptr++) & 0xff;
        tmp = ptr + num * sizeof(int);
        if (num > BRC_MAXNUM) num = BRC_MAXNUM;
        *pnum = num;
        memcpy(list, ptr, num * sizeof(int));
        return tmp;
}

char *brc_putrecord(char *ptr, char *name, int num, int *list) {
        if (num > 0 ) {
                if (num > BRC_MAXNUM) {
                        num = BRC_MAXNUM;
                }
                strncpy(ptr, name, BRC_STRLEN);
                ptr += BRC_STRLEN;
                *ptr++ = num;
                memcpy(ptr, list, num * sizeof(int));
                ptr += num * sizeof(int);
        }
        return ptr;
}

void brc_update(char *userid, char *board) {
        char    dirfile[STRLEN], *ptr;
        char    tmp_buf[BRC_MAXSIZE - BRC_ITEMSIZE], *tmp;
        char    tmp_name[BRC_STRLEN];
        int     tmp_list[BRC_MAXNUM], tmp_num;
        int     fd, tmp_size;
        ptr = brc_buf;
        if (brc_num > 0) {
                ptr = brc_putrecord(ptr, brc_name, brc_num, brc_list);
        }
        if (1) {
                sethomefile(dirfile, userid, ".boardrc");
                if ((fd = open(dirfile, O_RDONLY)) != -1) {
                        tmp_size = read(fd, tmp_buf, sizeof(tmp_buf));
                        close(fd);
                } else {
                        tmp_size = 0;
                }
        }
        tmp = tmp_buf;
        while (tmp < &tmp_buf[tmp_size] && (*tmp >= ' ' && *tmp <= 'z')) {
                tmp = brc_getrecord(tmp, tmp_name, &tmp_num, tmp_list);
                if (strncmp(tmp_name, board, BRC_STRLEN) != 0) {
                        ptr = brc_putrecord(ptr, tmp_name, tmp_num, tmp_list);
                }
        }
        brc_size = (int) (ptr - brc_buf);
        if ((fd = open(dirfile, O_WRONLY | O_CREAT, 0644)) != -1) {
                ftruncate(fd, 0);
                write(fd, brc_buf, brc_size);
                close(fd);
        }
}

int brc_init(char *userid, char *boardname) {
	FILE *fp;
        char dirfile[STRLEN], *ptr;
	int tmpno=0;
        sethomefile(dirfile, userid, ".boardrc");
        if(fp=fopen(dirfile, "r")) {
                brc_size=fread(brc_buf, 1, BRC_MAXSIZE, fp);
                fclose(fp);
        } else {
                brc_size = 0;
        }
        ptr=brc_buf;
        while(ptr< &brc_buf[brc_size] && (*ptr >= ' ' && *ptr <= 'z')) {
                ptr=brc_getrecord(ptr, brc_name, &brc_num, brc_list);
                if (!strncasecmp(brc_name, boardname, BRC_STRLEN)) return brc_num;
        }
        strncpy(brc_name, boardname, BRC_STRLEN);
        brc_list[0] = 1;
        brc_num=1;
        return 0;
}

int brc_add_read(char *filename) {
        int     ftime, n, i;
        if(!loginok) return;
        ftime=atoi(&filename[2]);
        if(filename[0]!='M' && filename[0]!='G')  return;
        if(brc_num<=0) {
                brc_list[brc_num++] = ftime;
                return;
        }
        for (n = 0; n < brc_num; n++) {
                if (ftime == brc_list[n]) {
                        return;
                } else if (ftime > brc_list[n]) {
                        if (brc_num < BRC_MAXNUM)
                                brc_num++;
                        for (i = brc_num - 1; i > n; i--) {
                                brc_list[i] = brc_list[i - 1];
                        }
                        brc_list[n] = ftime;
                        return;
                }
        }
        if(brc_num<BRC_MAXNUM) brc_list[brc_num++] = ftime;
}

int brc_un_read(char *file) {
        int ftime, n;
        if(file[0]!='M' && file[0]!='G') return 0;
	ftime=atoi(file+2);
        if (brc_num <= 0)
                return 1;
        for (n=0; n<brc_num; n++) {
                if(ftime>brc_list[n]) return 1;
                if(ftime==brc_list[n]) return 0;
        }
        return 0;
}

int brc_clear() {
	int i;
	char filename[20];
	if(!loginok) return;
	for(i=0; i<60; i++) {
		sprintf(filename, "M.%d.A", time(0)-i);
		brc_add_read(filename);
	}
}

int brc_has_read(char *file) {
	return !brc_un_read(file);
}
/* End */

/* added by flyriver, 2001.12.17
 * using getcurrusr() instead of using currentuser directly
 */
struct userec *getcurrusr()
{
	return &currentuser;
}

struct userec *setcurrusr(struct userec *user)
{
	if (user == NULL)
		return NULL;
	currentuser = *user;
	return &currentuser;
}

char *getcurruserid()
{
	return currentuser.userid;
}

unsigned int getcurrulevel()
{
	return currentuser.userlevel;
}

int has_perm(unsigned int x)
{
	return x ? currentuser.userlevel&x : 1;
}

int define(unsigned int x)
{
	return x ? currentuser.userdefine&x : 1;
}

time_t get_idle_time(struct user_info *uentp)
{
	return uentp->freshtime;
}

time_t set_idle_time(struct user_info *uentp, time_t t)
{
	uentp->freshtime = t;

	return t;
}

bcache_t *getbcacheaddr()
{
	return bcache;
}

uinfo_t *getcurruinfo()
{
	return u_info;
}

uinfo_t *setcurruinfo(uinfo_t *ui)
{
	u_info = ui;

	return u_info;
}

/* should do some locking before calling this function */
int eat_file_content(int fd, off_t start, off_t len)
{
    struct stat fs;
    char *addr;
    int i, r;

    if (start < 0 || len <= 0)
        return 1;
    if (fstat(fd, &fs) < 0)
        return -1;
    if (start >= fs.st_size)
        return 1;
    addr = mmap(0, fs.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED)
        return -1;
    if (start + len > fs.st_size)
        len = fs.st_size - start;
    r = fs.st_size - start - len;
    for (i = start; i < r; i++)
        addr[start + i] = addr[start + len + i];
    munmap(addr, fs.st_size);
    ftruncate(fd, fs.st_size - len);

	return 0;
}

int isowner(struct userec *user, struct fileheader *fileinfo)
{
    char buf[25];
    time_t posttime;
    if (strcmp(fileinfo->owner,user->userid))
        return 0;
    posttime = atoi(fileinfo->filename+2);
    if (posttime<user->firstlogin)
        return 0;
    return 1;
}

int chk_currBM(char *BMstr)   /* ¸ù¾ÝÊäÈëµÄ°æÖ÷Ãûµ¥ ÅÐ¶Ïµ±Ç°userÊÇ·ñÊÇ°æÖ÷ */
{
    char *ptr;
    char BMstrbuf[STRLEN-1];

    if(has_perm(PERM_OBOARDS)||has_perm(PERM_SYSOP))
        return YEA;

    if(!has_perm(PERM_BOARDS))
        return NA;
    strcpy(BMstrbuf,BMstr);
    ptr=strtok(BMstrbuf,",: ;|&()\0\n");
    while(1)
    {
        if(ptr==NULL)
            return NA;
        if(!strcmp(ptr,getcurruserid()/*,strlen(currentuser->userid)*/))
            return YEA;
        ptr=strtok(NULL,",: ;|&()\0\n");
    }
}


