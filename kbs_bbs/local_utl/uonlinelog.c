#include <time.h>
#include <stdio.h>
#include "bbs.h"
#include "config.h"

FILE *fp;

#define BONLINE_LOGDIR "/home/bbs/bonlinelog"

int do_userlist(struct user_info *uentp, char *arg, int t)
{
    int i;
    int fd, len;
    char user_info_str[256 /*STRLEN*2 */ ], pagec;
    int override;

    t++;
    if (!uentp->active || !uentp->pid) {
        printf(" %4d 啊,我刚走\n", t);
        return 0;
    }
    pagec = ' ';
    sprintf(user_info_str,
             /*---	modified by period	2000-10-21	在线用户数可以大于1000的
                     " %3d%2s%s%-12.12s%s%s %-16.16s%s %-16.16s %c %c %s%-17.17s\033[m%5.5s\n",
             ---*/
            " %4d%2s%-12.12s %-16.16s %-16.16s %c %c %s%-12.12s\033[m%5.5s %d\n", t, uentp->invisible ? "＃" : "．", uentp->userid, uentp->username, uentp->from, pagec, ' ', (uentp->invisible == true)
            ? "\033[34m" : "", modestring(uentp->mode, uentp->destuid, 0,  /* 1->0 不显示聊天对象等 modified by dong 1996.10.26 */
                                       (uentp->in_chat ? uentp->chatid : NULL)), idle_str(uentp), uentp->pid);
    fprintf(fp, "%s", user_info_str);
    return COUNT;
}

main()
{
	char path[256];
	struct stat st;
	time_t now;
	struct tm t;

	now = time(0);
	localtime_r( &now, &t);

    chdir(BBSHOME);
	if( stat( BONLINE_LOGDIR, &st) < 0 ){
		if(mkdir(BONLINE_LOGDIR, 0755) < 0)
			exit(0);
	}
	sprintf(path, "%s/%d", BONLINE_LOGDIR, t.tm_year+1900);
	if( stat(path, &st) < 0){
		if(mkdir(path, 0755) < 0)
			exit(0);
	}
	sprintf(path, "%s/%d/%d", BONLINE_LOGDIR, t.tm_year+1900, t.tm_mon+1);
	if( stat(path, &st) < 0){
		if(mkdir(path, 0755) < 0)
			exit(0);
	}

	sprintf(path, "%s/%d/%d/%d_%d.ulog", BONLINE_LOGDIR, t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour);
	if((fp=fopen(path, "w"))==NULL){
		printf("cannot open log file\n");
		exit(0);
	}

    resolve_utmp();
	get_publicshm();

	fprintf(fp, "%d\n%d\n", get_utmp_number(), getwwwguestcount());
    //fprintf(fp," 序号  用户ID       昵称             来源                 状态     发呆时间 进程号\n");
    apply_ulist_addr(do_userlist, NULL);

    fclose(fp);
}


