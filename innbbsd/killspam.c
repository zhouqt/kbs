/*
 * killspam.c		-- an enhanced part innbbsd of Firebird BBS 3.0
 *
 * A part of SEEDNetBBS generation 1
 *
 * Copyright (c) 1998, 1999, Edward Ping-Da Chuang <edwardc@firebird.dhs.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * CVS: $Id$
 */

#ifdef ANTISPAM
#include "innbbsconf.h"
#include "config.h"

#define MYLOG	BBSHOME"/log/bbsnnrp.debug"
#define GOBAL
#define SUB_DIR	"innd"		/* save black list file in ~bbs/innd */

#undef _ANTISPAM_DEBUG_		/* debug only */
#undef _TRY_OUT_ONLY_		/* 请匀尝试 :> 正常情况请 comment out */

#ifndef MAX_LIST
#define MAX_LIST 256
#endif

struct spam {
	char    list[80];
};
/* for antispam */
char    hostc[256];
char    host[256];
char    only[256];
char    ch[1];
int     isspam = 0;
int     only_mode = 0;
int     fd_for_debug;
int     have_list = 0;

struct spam c_list[MAX_LIST];	/* for content use buf fix1 */
struct spam h_list[MAX_LIST];	/* for host    use buf fix2 */
struct spam d_list[MAX_LIST];	/* for domain  use buf fix3 */
struct spam o_list[MAX_LIST];	/* for "only"  use buf fix4 */

int     fix[] = {0, 0, 0, 0};

int     reslov_host = 0;	/* 预设开啟检查来源功能 0 = 开啟 1 = 关闭 */
loadlist(char *sign)
{				/* by edwardc for antispam enahnced */
	FILE   *fp;
	char    mybuf[256];
	char    genbuf[MAXPATHLEN], buf[10], buf2[80];
	char    gobal[MAXPATHLEN];
	int     i = 0, j = 0, k = 0, l = 0;
	if (sign != NULL)
		sprintf(genbuf, "%s/%s/black.list.%s", _PATH_BBSHOME, SUB_DIR, sign);

#ifdef GOBAL
	sprintf(gobal, "%s/%s/black.list.general", _PATH_BBSHOME, SUB_DIR);
#endif

	fp = fopen(genbuf, "r");

#ifdef GOBAL
	if (fp == NULL) {
		fp = fopen(gobal, "r");
	}
#endif

	if (fp != NULL) {

		while (!feof(fp)) {

			fscanf(fp, "%s %s", buf, buf2);

			if (!feof(fp)) {
				switch (buf[0]) {

				case 'c':
				case 'C':

#ifdef _ANTISPAM_DEBUG_
					sprintf(mybuf, "load content [%d]: %s\n", i, buf2);
					antispamdebug(mybuf, 1);
#endif
					strcpy(c_list[i].list, buf2);
					i++;
					break;


				case 'H':
				case 'h':

#ifdef _ANTISPAM_DEBUG_
					sprintf(mybuf, "load host [%d]: %s\n", j, buf2);
					antispamdebug(mybuf, 1);
#endif
					strcpy(h_list[j].list, buf2);
					j++;
					break;

				case 'd':
				case 'D':

#ifdef _ANTISPAM_DEBUG_
					sprintf(mybuf, "load domain [%d]: %s\n", k, buf2);
					antispamdebug(mybuf, 1);
#endif
					strcpy(d_list[k].list, buf2);
					k++;
					break;

				case 'o':
				case 'O':

#ifdef _ANTISPAM_DEBUG_
					sprintf(mybuf, "load only [%d]: %s\n", l, buf2);
					antispamdebug(mybuf, 1);
#endif
					strcpy(o_list[l].list, buf2);
					l++;
					break;

				case 'q':
				case 'Q':

					if (strcasecmp(buf2, "NO") == 0) {
						sprintf(mybuf, "reslove hostname is DISABLE\n");
						antispamdebug(mybuf, 1);
						reslov_host = 1;
					} else {
						reslov_host = 0;
					}

					break;

				case '#':
				case ';':
					/* ignore these lines */
					break;

				default:

				}
			}
		}

		fclose(fp);

	}
	fix[0] = i;
	fix[1] = j;
	fix[2] = k;
	fix[3] = l;
}
dumplist(char *sign)
{
	int     i, j, k, l, log;
	time_t  now;
	char    buf[256];
	if (sign != NULL)
		fprintf(stdout, "设定档: %s/black.list.%s \n", SUB_DIR, sign);
#ifdef GOBAL
	else
		fprintf(stdout, "设定档: %s/black.list.general \n", SUB_DIR);
#endif

	loadlist(sign);

	fprintf(stdout, "是否检查来源: %s \n\n", (reslov_host == 1) ? "不检查" : "检查");

	fprintf(stdout, "编号  类别  设定值\n");

	if (fix[0] == 0) {

		fprintf(stdout, "沒有任何关於 \"content\" 的设定\n");

	} else {

		for (i = 0; i < fix[0]; i++) {
			fprintf(stdout, "%.4d  cont  %s\n", i + 1, c_list[i].list);
		}
	}

	if (fix[1] == 0) {

		fprintf(stdout, "沒有任何关於 \"host\" 的设定\n");

	} else {

		for (j = 0; j < fix[1]; j++) {
			fprintf(stdout, "%.4d  host  %s\n", j + 1, h_list[j].list);
		}
	}

	if (fix[2] == 0) {

		fprintf(stdout, "沒有任何关於 \"domain\" 的设定\n");

	} else {

		for (k = 0; k < fix[2]; k++) {
			fprintf(stdout, "%.4d  domn  %s\n", k + 1, d_list[k].list);
		}
	}

	if (fix[3] == 0) {

		fprintf(stdout, "沒有任何关於 \"only\" 的设定\n");

	} else {

		for (l = 0; l < fix[3]; l++) {
			fprintf(stdout, "%.4d  only  %s\n", l + 1, o_list[l].list);
		}
	}

	fprintf(stdout, "------------------------------------------------\n");
	fprintf(stdout, "共 [%d] 笔资料 \n", fix[0] + fix[1] + fix[3] + fix[4]);

	if (log != -1) {
		now = time(0);
		sprintf(buf, "dump %d record%s \n",
			fix[0] + fix[1] + fix[3] + fix[4], ((fix[0] + fix[1] + fix[3] + fix[4]) > 1) ? "(s)" : "");
		antispamdebug(buf, 1);
		strcpy(buf, "");
		sprintf(buf, "bbsnnrp 1.30b3 terminal at %12.12s \n", ctime(&now) + 4);
		antispamdebug(buf, 1);
	}
	exit(0);
}

int
killspam(char *mid, int artcount, int artno)
{
	char    buf[256];

	int     jam, i, j, k;
	if (have_list == 0) {
		loadlist(NULL);
		have_list = 1;
	}
	strcpy(host, "");
	strcpy(buf, "");
	isspam = 0;
	/*
	 * mid 是这样子的 <MESSAGE-ID@NNTP-Post-Host>
	 */


	if (strrchr(mid, '@') != NULL) {
		sprintf(hostc, "%s", strrchr(mid, '@'));
	} else {
		if (artcount != -1 && artno != -1) {
			printf("** %d ** %d ignore it %s\n", artcount, artno, mid);
			sprintf(buf, "** %d ** %d ignore it %s\n", artcount, artno, mid);
		} else {
			sprintf(buf, "innbbsd:CMDihave:ignore mid: %s\n", mid);
		}
		antispamdebug(buf, 1);
		isspam = 1;
	}


	for (i = 1; i < (strlen(hostc) - 1); i++) {
		sprintf(ch, "%c", hostc[i]);
		strcat(host, ch);
	}

	/* May 9 加入不检查 host 功能 */

	if (isspam == 0) {
		if (!gethostbyname(host) && !gethostbyaddr(host, strlen(host), AF_INET) \
			&&reslov_host == 0) {
			if (artcount != -1 && artno != -1) {
				printf("** %d ** %d ignore it %s \n", artcount, artno, mid);
				sprintf(buf, "** %d ** %d ignore it %s \n", artcount, artno, mid);
			} else {
				sprintf(buf, "innbbsd:CMDihave:ignore mid: %s\n", mid);
			}
			antispamdebug(buf, 1);
			isspam = 2;
		}
	}
	if (isspam == 0) {
		if (!strcasecmp(host, "127.0.0.1") || !strcasecmp(host, "0.0.0.0") || strchr(host, ' ')) {
			if (artcount != -1 && artno != -1) {
				printf("** %d ** %d ignore it %s\n", artcount, artno, mid);
				sprintf(buf, "** %d ** %d ignore it %s\n", artcount, artno, mid);
			} else {
				sprintf(buf, "innbbsd:CMDihave:ignore mid: %s\n", mid);
			}
			antispamdebug(buf, 1);
			isspam = 2;
		}
	}
	if (fix[3] != 0 && isspam == 0) {

		for (i = 0; i < fix[3]; i++) {

			jam = (strlen(host) - strlen(o_list[i].list));
			strcpy(only, "");

			for (j = 0; j < strlen(host) - jam; j++) {
				sprintf(ch, "%c", host[jam + j]);
				strcat(only, ch);
			}
			/* 991107.edwardc 白烂, 就是要保留跟 only 符合的 record
			   所以 strcasecmp 符合的要留下来 .. */
			/* skyo modify 000307 */
			if (strcasecmp(o_list[i].list, only) == 0) {
				if(artcount = -1 && artno != -1) {
					sprintf(buf, "innbbsd:CMDihave:ignore mid: %s\n", mid);
					return isspam=0;
				}
			}
		}
			isspam=3;
                        sprintf(buf, "** %d ** %d ignore it (only) %s\n", artcount, artno, mid);
			antispamdebug(buf, 1);
	}

	/* 检查顺序: only -> host -> doamin -> content */

	if (isspam == 0) {

		for (i = 0; i < fix[1]; i++) {
			if (strcasecmp(h_list[i].list, host) == 0) {
				if (artcount != -1 && artno != -1) {
					printf("** %d ** %d ignore it (host) %s\n", artcount, artno, mid);
					sprintf(buf, "** %d ** %d ignore it (host) %s\n", artcount, artno, mid);
				} else {
					sprintf(buf, "innbbsd:CMDihave:ignore mid: %s\n", mid);
				}
				antispamdebug(buf, 1);
				isspam = 4;
			}
		}

	}
	if (isspam == 0) {
		for (i = 0; i < fix[2]; i++) {

			jam = (strlen(host) - strlen(d_list[i].list));

			strcpy(only, "");

			for (j = 0; j < strlen(host) - jam; j++) {
				sprintf(ch, "%c", host[jam + j]);
				strcat(only, ch);
			}

			if (strcasecmp(d_list[i].list, only) == 0) {
				if (artcount != -1 && artno != -1) {
					printf("** %d ** %d ignore it (domain) %s\n", artcount, artno, mid);
					sprintf(buf, "** %d ** %d ignore it (domain) %s\n", artcount, artno, mid);
				} else {
					sprintf(buf, "innbbsd:CMDihave:ignore mid: %s\n", mid);
				}
				antispamdebug(buf, 1);
				isspam = 5;
			}
		}
	}
	if (isspam == 0) {
		for (i = 0; i < fix[0]; i++) {
			if (strstr(host, c_list[i].list) != NULL) {
				if (artcount != -1 && artno != -1) {
					printf("** %d ** %d ignore it (content) %s\n", artcount, artno, mid);
					sprintf(buf, "** %d ** %d ignore it (content) %s\n", artcount, artno, mid);
				} else {
					sprintf(buf, "innbbsd:CMDihave:ignore mid: %s\n", mid);
				}
				antispamdebug(buf, 1);
				isspam = 6;
			}
		}
	}
	return isspam;
}

int
antispamdebug(char *mesg, int type)
{				/* 专属 log 用 function */
	char    buf[50];
#ifndef _ANTISPAM_DEBUG_
	/* discard it :) */
	return;
#endif

	if (type == 0) {

		fd_for_debug = open(MYLOG, O_RDWR | O_CREAT | O_APPEND, 0644);

		if (fd_for_debug < 0) {
			printf("antispam enhanced: cannot open debug log file!\n");
			printf("FILE: %s\n", MYLOG);
			printf("**ALL LOG MESSAGES WILL BE PRINTED ON STDERR**\n");
			fd_for_debug = 99;
		} else {
			write(fd_for_debug, mesg, strlen(mesg));
		}

	} else if (type == 1) {

		if (fd_for_debug < 0) {
			antispamdebug(mesg, 0);
		} else if (fd_for_debug == 99) {
			fprintf(stderr, "%s%s", mesg, (mesg[strlen(mesg)] == '\n') ? " " : "\n");
		} else {
			write(fd_for_debug, mesg, strlen(mesg));
		}

	} else {

		sprintf(buf, "error of type = %d\n", type);
		antispamdebug(buf, 1);

	}

}

#endif	/* ANTISPAM */
