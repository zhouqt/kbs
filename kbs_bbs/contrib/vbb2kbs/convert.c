/*
 *   vBulletin to SMTHBBS Converter
 *   by FreeWizard@happynet.org
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mysql.h>
#include "bbs.h"
#include "convert.h"

MYSQL* mysql;
MYSQL_RES *results, *results2;
MYSQL_ROW record, record2;

#define CHECK {if(mysql_errno(mysql)){fprintf(stderr, "MySQL error: %s\n", mysql_error(mysql)); exit(0);}}
#define _MAX 999999
//FIXME: so dirty
//char bbsid[_MAX][128];
int threads[_MAX];
static const char TAG_SPECL[][2][8] = {
	{"nbsp", " "},
	{"amp", "&"},
	{"quot", "\""},
	{"gt", ">"},
	{"lt", "<"},
	{"", ""}
};

char* getattachpath(int userid, int attachid) {
	static char buf[128];
	//vbb attach mode 1: path/userid/attachid.attach
	//sprintf(buf, "%s/%d/%d.attach", ATTACHPATH, userid, attachid);
	//vbb attach mode 2: path/u/s/e/r/i/d/attachid.attach
	char t[16], n[32];
	int i;
	sprintf(t, "%d", userid);
	memset(n, 0, sizeof(n));
	for (i=0;i<strlen(t);i++) {
		n[i*2] = t[i];
		n[i*2+1] = '/';
	}
	sprintf(buf, "%s/%s%d.attach", ATTACHPATH, n, attachid);
	return buf;
}

int main() {
	int cnt, a, b;
	char *p, *q;
	time_t now;
	char stmt[999];
	int fd;
	FILE *fout, *af;
	struct fileheader post;
	char fname[512], buf[1024], boardpath[256], conv_buf[256];
	
	mysql = mysql_init(NULL);
	CHECK;
	mysql_real_connect(mysql, HOST, USER, PASS, DB, PORT, NULL, 0);
	CHECK;
	/*
	//FIXME: converting users
	mysql_query(mysql, "SELECT userid, username FROM user");
	CHECK;
	results = mysql_store_result(mysql);
	CHECK;
	memset(bbsid, 0, sizeof(bbsid));
	cnt = 0;
	while((record = mysql_fetch_row(results))) {
		a = atoi(record[0]);
		strcpy(bbsid[a], vbbuser2bbsid(record[1], a));
	//	printf("%d\t%s\n", a, bbsid[a]);
		cnt++;
	}
	printf("%d IDs collected.\n", cnt);
	mysql_free_result(results);
	CHECK;
	*/
	chdir(BBSHOME);
	a = 0;
	while (forumid[a]) {
		printf("Convert %d -> %s...", forumid[a], boardname[a]);
		
		memset(threads, 0, sizeof(threads));
		sprintf(stmt, "SELECT threadid, firstpostid FROM thread WHERE forumid=%d ORDER BY threadid", forumid[a]);
		mysql_query(mysql, stmt);
		CHECK;
		results = mysql_store_result(mysql);
		CHECK;
		cnt = 0;
		while((record = mysql_fetch_row(results))) threads[atoi(record[0])] = atoi(record[1]);
		mysql_free_result(results);
		CHECK;
		
		setbfile(buf, boardname[a], DOT_DIR);
		setbpath(boardpath, (char *)boardname[a]);
		if ((fd = open(buf, O_WRONLY | O_CREAT | O_TRUNC, 0664)) == -1) {
			printf("cannot open %s.\n", buf);
			break;
		}
		flock(fd, LOCK_EX);
		
		sprintf(stmt, "SELECT post.postid, post.threadid, post.parentid, post.username, post.userid,  thread.title, post.title, post.dateline, post.pagetext, post.ipaddress, post.attach FROM `post`, thread WHERE post.threadid=thread.threadid and thread.forumid=%d ORDER BY post.dateline", forumid[a]);
		mysql_query(mysql, stmt);
		CHECK;
		results = mysql_store_result(mysql);
		CHECK;
		cnt=0;
		while((record = mysql_fetch_row(results))) {
			fflush(stdout);
			//if (cnt==100) break;
			cnt++;
			memset(&post, 0, sizeof(fileheader));
			now = atoi(record[7]);
			bbssettime(now);
			//lseek(fd, 0, SEEK_END);
			post.id = atoi(record[0]);
			post.groupid = threads[atoi(record[1])] ? threads[atoi(record[1])] : atoi(record[0]);
			post.reid = atoi(record[2]) ? atoi(record[2]) : atoi(record[0]);
			strcpy(post.owner, vbbuser2bbsid(record[3], atoi(record[4])));
			//for (b=0; b<11; b++) printf("%d\t%s\n", b, record[b]);
			if ((record[6][0]!='\0')&&(!((record[6][6]=='\0')&&(record[6][0]=='[')&&(record[6][5]==']')))) {
				strncpy(conv_buf, record[6], sizeof(conv_buf) - 1);
			} else {
				strcpy(conv_buf, "Re: ");
				strcat(conv_buf, record[5]); //FIXME: potential overflow
			}
			p = conv_buf;
			while (*p) {
				if (*p=='&') {
					q = ++p;
					while ((*p)&&(*p!=';')) p++;
					*p = '\0';
					b = 0;
					while(TAG_SPECL[b][0][0]) {
						if (!strcmp(TAG_SPECL[b][0], q)) {
							strcpy(q, ++p);
							p = --q;
							*p = TAG_SPECL[b][1][0];
							break;
						}
						b++;
					}
				}
				p++;
			}
			strncpy(post.title, conv_buf, sizeof(post.title) - 1);
			post.innflag[0] = 'L';
			post.innflag[1] = 'L';
			post.posttime = now;
			post.eff_size = strlen(record[8]);
			GET_POSTFILENAME(fname, boardpath);
			strcpy(post.filename, fname);
			setbfile(buf, boardname[a], fname);
			strcpy(fname, buf);
			if ((fout = fopen(fname, "w")) == NULL) { printf("cannot open %s\n", fname); exit(1); }
			fprintf(fout, "发信人: %s (%s), 信区: %s\n", post.owner, record[3], boardname[a]);
			fprintf(fout, "标  题: %s\n", conv_buf);
			fprintf(fout, "发信站: %s (%24.24s), VBBPost\n", BBS_FULL_NAME, ctime(&now));
			fprintf(fout, "\n");
			//fprintf(fout, "%s\n\n--\n\n", record[8]);
			p = record[8];
			while (*p) {
				q = strcasestr(p, "[QUOTE");
				if (!q) { //no quote left
					fputs(p, fout);
					break;
				} else {
					*q = '\0';
					fputs(p, fout);
					q+= strlen("[QUOTE");
					p = (*q=='=') ? ++q : NULL;
					while ((*q)&&(*q!=']')) q++;
					if (p) { //quote from someone
						*q = '\0';
						fprintf(fout, "【 在 %s 的大作中提到: 】\n", p);
					}
					p = ++q;
					q = strcasestr(p, "[/QUOTE]");
					if (!q) { //unmatched pair
						fputs(p, fout);
						break;
					} else {
						*q = '\0';
						fputs(": ", fout);
						while (*p) {
							fputc(*p, fout);
							if (*p=='\n') fputs(": ", fout);
							p++;
						}
						p+= strlen("[/QUOTE]");
					}
				}
			}
			fprintf(fout, "\n\n--\n\n");
			fprintf(fout, "\033[m\033[1;%2dm※ 来源:·%s %s·[FROM: %s]\033[m\n", (int) (now % 7) + 31, BBS_FULL_NAME, NAME_BBS_ENGLISH, 
#ifdef HIDEIP
					"*.*.*.*"
#else
					record[9]
#endif
					);
			if (atoi(record[10])) {
				post.attachment = ftell(fout);
				sprintf(stmt, "SELECT userid, attachmentid, filename FROM attachment WHERE postid=%d", post.id);
				mysql_query(mysql, stmt);
				CHECK;
				results2 = mysql_store_result(mysql);
				CHECK;
				while((record2 = mysql_fetch_row(results2))) {
					p = getattachpath(atoi(record2[0]), atoi(record2[1]));
					fwrite(ATTACHMENT_PAD, 1, ATTACHMENT_SIZE, fout);
					fwrite(record2[2], 1, strlen(record2[2])+1, fout);
					if ((af=fopen(p, "r")) == NULL) {
						b = 0;	
						fwrite(&b, 1, 4, fout);
						printf("%s not found ", p);
					} else {
						//printf("info: %d %s found ", post.id, p);
						fseek(af, 0, SEEK_END);
						b = htonl(ftell(af));
						fwrite(&b, 1, 4, fout);
						fseek(af, 0, SEEK_SET);
						while ((b=fread(buf, 1, sizeof(buf), af))>0) fwrite(buf, 1, b, fout);
						fclose(af);
					}
				}
				mysql_free_result(results2);
				CHECK;
			}
			fclose(fout);
			
			if (safewrite(fd, &post, sizeof(fileheader)) == -1) { printf("cannot update .DIR\n"); exit(1); }
			//printf("%s\t%s\n", post.filename, record[2]);
		}
		printf("%d posts processed.\n", cnt);
		mysql_free_result(results);
		CHECK;
		flock(fd, LOCK_UN);
		close(fd);
		//updatelastpost(boardname[a]);
		//setboardtitle(boardname[a], 1);
		a++;
	}
	mysql_close(mysql);
	CHECK;
	return 0;
}
