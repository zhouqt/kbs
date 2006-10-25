/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include "bbs.h"
#include "vote.h"
#include "read.h"
extern int page, range;
struct votebal currvote;
struct votelimit currlimit;     /*Haohmaru.99.11.17.根据版主设的限制条件判断是否让该使用者投票 */
char controlfile[STRLEN], limitfile[STRLEN];
unsigned int result[33];
int vnum;
int voted_flag;
FILE *sug;
static int mk_result(int num);
static int dele_vote(int num);
static int Show_Votes();
int cmpvuid(userid, uv)
char *userid;
struct ballot *uv;
{
    return !strncmp(userid, uv->uid,IDLEN);
}
int setvoteflag(char *bname, int flag)
{
    int pos;
    struct boardheader fh;

    pos = getboardnum(bname, &fh);
    if (pos) {
        if (flag == 0)
            fh.flag = fh.flag & ~BOARD_VOTEFLAG;
        else
            fh.flag = fh.flag | BOARD_VOTEFLAG;
        set_board(pos, &fh,NULL);
    }
    return 0;
}

void b_report(str)
char *str;
{
    char buf[STRLEN];

    sprintf(buf, "%s %s", currboard->filename, str);
    bbslog("user","%s",buf);
}

void setcontrolfile()
{
    setvfile(controlfile, currboard->filename, "control");
}

#ifdef NEWSMTH
int b_rules_edit()
{
    char buf[STRLEN];
    char buf1[STRLEN];
    int aborted;
    int oldmode;
	struct stat st;

    if (!chk_currBM(currBM, getCurrentUser())) {
        return 0;
    }
    clear();
    makevdir(currboard->filename);
    setvfile(buf, currboard->filename, "rules");
        oldmode = uinfo.mode;
        modify_user_mode(EDITUFILE);
        aborted = vedit(buf, false,NULL, NULL, 0);
        modify_user_mode(oldmode);
    if (aborted == -1) {
        pressreturn();
    } else {
		if(currboard->flag & BOARD_RULES) set_board_rule(currboard, 0);
		if(stat(buf, &st)==-1) return FULLUPDATE;
		sprintf(buf1, "%s提交%s治版方针:%ld", getCurrentUser()->userid, currboard->filename, st.st_mtime);
		post_file(getCurrentUser(), "", buf, "BoardRules", buf1, 0, 2, getSession());
		if(normal_board(currboard->filename))
		post_file(getCurrentUser(), "", buf, "BoardManager", buf1, 0, 2, getSession());

		clear();
		move(3,0);
		prints("已经提交新的治版方针,请等待批准\n");
		pressreturn();
    }
    return FULLUPDATE;
}
#endif

int b_notes_edit()
{
    char buf[STRLEN];
    char ans[4];
    int aborted;
    int oldmode;

    if (!chk_currBM(currBM, getCurrentUser())) {
        return 0;
    }
    clear();
    makevdir(currboard->filename);
    setvfile(buf, currboard->filename, "notes");
    getdata(1, 0, "(E)编辑 (D)删除 本讨论区的备忘录? [E]: ", ans, 2,
            DOECHO, NULL, true);
    if (ans[0] == 'D' || ans[0] == 'd') {
        move(2, 0);
        if (askyn("真的要删除本讨论区的备忘录", 0)) {
            move(3, 0);
            prints("备忘录已经删除...\n");
            pressanykey();
            my_unlink(buf);
            aborted = 1;
        } else
            aborted = -1;
    } else {
        oldmode = uinfo.mode;
        modify_user_mode(EDITUFILE);
        aborted = vedit(buf, false,NULL, NULL, 0);
        modify_user_mode(oldmode);
    }
    if (aborted == -1) {
        pressreturn();
    } else {
        setvfile(buf, currboard->filename, "noterec");
        my_unlink(buf);
    }
    return FULLUPDATE;
}

#ifdef FLOWBANNER
void load_board_banner(const char * board)
{
    int i;
	FILE *fp;
    struct boardheader bh;
	const struct boardheader *obh;
    char filename[STRLEN * 2];
	char buf[512];
    sprintf(filename, "boards/%s/banner", board);
	obh = getbcache(board);
    memcpy(&bh, obh, sizeof(struct boardheader));
	i = 0;
    if (NULL != (fp = fopen(filename, "r"))) {
		while ((!feof(fp))&&(i<MAXBANNER)) {
			fgets(buf, 512, fp);
			buf[BANNERSIZE-1] = 0;
			strcpy(bh.banners[i], buf);
			if (*banner_filter(bh.banners[i])) i++;
		}
		fclose(fp);
    }
	bh.bannercount = i;
	set_board(getboardnum(board, NULL), &bh, NULL);
}

int b_banner_edit()
{
    char buf[STRLEN];
    char ans[4];
    int aborted;
    int oldmode;

    if (!chk_currBM(currBM, getCurrentUser())) {
        return 0;
    }
    clear();
    sprintf(buf, "boards/%s/banner", currboard->filename);
    getdata(1, 0, "(E)编辑 (D)删除 本讨论区的流动信息? [E]: ", ans, 2,
            DOECHO, NULL, true);
    if (ans[0] == 'D' || ans[0] == 'd') {
        move(2, 0);
        if (askyn("真的要删除本讨论区的流动信息", 0)) {
            move(3, 0);
            prints("流动信息已经删除...\n");
            pressanykey();
            my_unlink(buf);
            aborted = 1;
        } else
            aborted = -1;
    } else {
        oldmode = uinfo.mode;
        modify_user_mode(EDITUFILE);
        aborted = vedit(buf, false,NULL, NULL, 0);
        modify_user_mode(oldmode);
    }
    if (aborted == -1) {
        pressreturn();
    } else {
    	load_board_banner(currboard->filename);
        setvfile(buf, currboard->filename, "noterec");
        my_unlink(buf);
    }
    return FULLUPDATE;
}
#endif

int b_sec_notes_edit(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{
    char buf[STRLEN];
    char ans[4];
    int aborted;

    if (!chk_currBM(currBM, getCurrentUser())) {
        return 0;
    }
    clear();
    makevdir(currboard->filename);
    setvfile(buf, currboard->filename, "secnotes");
    getdata(1, 0, "(E)编辑 (D)删除 本讨论区的秘密备忘录? [E]: ", ans, 2,
            DOECHO, NULL, true);
    if (ans[0] == 'D' || ans[0] == 'd') {
        move(2, 0);
        if (askyn("真的要删除本讨论区的秘密备忘录", 0)) {
            move(3, 0);
            prints("秘密备忘录已经删除...\n");
            pressanykey();
            my_unlink(buf);
            aborted = 1;
        } else
            aborted = -1;
    } else
        aborted = vedit(buf, false,NULL, NULL, 0);
    if (aborted == -1) {
        pressreturn();
    } else {
        setvfile(buf, currboard->filename, "noterec");
        my_unlink(buf);
    }
    return FULLUPDATE;
}

int b_jury_edit(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{                               /* stephen 2001.11.1: 编辑版面仲裁名单 */
    char buf[STRLEN];
    char ans[4];
    int aborted;

    if (!((HAS_PERM(getCurrentUser(), PERM_JURY)
           && HAS_PERM(getCurrentUser(), PERM_BOARDS))
          || HAS_PERM(getCurrentUser(), PERM_SYSOP))) {
        return 0;
    }
    clear();
    makevdir(currboard->filename);
    setvfile(buf, currboard->filename, "jury");
    getdata(1, 0, "(E)编辑 (D)删除 本讨论区仲裁委员名单? (C)取消 [C]: ",
            ans, 2, DOECHO, NULL, true);
    if (ans[0] == 'D' || ans[0] == 'd') {
        move(2, 0);
        if (askyn("真的要删除本讨论区仲裁委员名单", 0)) {
            move(3, 0);
            prints("仲裁委员名单已经删除...\n");
            pressanykey();
            my_unlink(buf);
            aborted = 111;
        } else
            aborted = -1;
    } else if (ans[0] == 'E' || ans[0] == 'e')
        aborted = vedit(buf, false,NULL, NULL, 0);
    else {
        prints("取消");
        aborted = -1;
    }
    if (aborted == -1) {
        pressreturn();
    } else {
        char secu[STRLEN];

        if (aborted == 111) {
            sprintf(secu, "删除 %s 版的仲裁委员名单", currboard->filename);
            securityreport(secu, NULL, NULL, getSession());
            post_file(getCurrentUser(), "", buf, "JuryMail", secu, 0, 2, getSession());
        } else {
            sprintf(secu, "修改 %s 版的仲裁委员名单", currboard->filename);
            securityreport(secu, NULL, NULL, getSession());
            post_file(getCurrentUser(), "", buf, "syssecurity", secu, 0, 2, getSession());
            post_file(getCurrentUser(), "", buf, "JuryMail", secu, 0, 2, getSession());
        }
        setvfile(buf, currboard->filename, "juryrec");
        my_unlink(buf);
    }
    return FULLUPDATE;
}

int b_suckinfile(fp, fname)
FILE *fp;
char *fname;
{
    char inbuf[256];
    FILE *sfp;

    if ((sfp = fopen(fname, "r")) == NULL)
        return -1;
    while (fgets(inbuf, sizeof(inbuf), sfp) != NULL)
        fputs(inbuf, fp);
    fclose(sfp);
    return 0;
}

int vote_close()
{
    time_t closetime;

    closetime = currvote.opendate + currvote.maxdays * 86400;
    if (closetime <= time(NULL)) {
        mk_result(vnum);
    }
    return 0;
}

int b_close(struct boardheader *fh,void* arg)
{
    int end;

    currboard= fh;
    currboardent=getbid(fh->filename,NULL);
    setcontrolfile();
    end = get_num_records(controlfile, sizeof(currvote));
    for (vnum = end; vnum >= 1; vnum--) {
        get_record(controlfile, &currvote, sizeof(currvote), vnum);
        vote_close();
    }
    return 0;
}

int b_closepolls()
{
    struct stat st;
    FILE *cfp;
    char buf[80];
    time_t now;
    struct boardheader* bh;
    int bid;

    now = time(NULL);
    strcpy(buf, "vote/lastpolling");
    if (stat(buf, &st) != -1 && st.st_mtime > now - 3600) {
        return 0;
    }
    move(t_lines - 1, 0);
    prints("对不起，系统关闭投票中，请稍候...");
    refresh();
    if ((cfp = fopen(buf, "w")) == NULL) {
        bbslog("3error","%s","lastpoll write error");
        return 0;
    }
    fprintf(cfp, "%s", ctime(&now));
    bh=currboard;
    bid=currboardent;
    fclose(cfp);
    apply_boards(b_close,NULL);
    currboard=bh;
    currboardent=bid;
    return 0;
}
int count_result(struct ballot *ptr, int idx, char *arg)
{
    int i;

    if (ptr == NULL) {
        if (sug != NULL) {
            fclose(sug);
            sug = NULL;
        }
        return 0;
    }
    if (ptr->msg[0][0] != '\0') {
        if (currvote.type == VOTE_ASKING) {
            fprintf(sug, "\033[44m%s 的作答如下：\033[m\n", ptr->uid);
        } else
            fprintf(sug, "\033[44m%s 的建议如下：\033[m\n", ptr->uid);
        for (i = 0; i < 3; i++)
            fprintf(sug, "%s\n", ptr->msg[i]);
    }
    result[32]++;
    if (currvote.type == VOTE_ASKING) {
        return 0;
    }
    if (currvote.type != VOTE_VALUE) {
        for (i = 0; i < 32; i++) {
            if ((ptr->voted >> i) & 1)
                (result[i])++;
        }
    } else {
        result[31] += ptr->voted;
        result[(ptr->voted * 10) / (currvote.maxtkt + 1)]++;
    }
    return 0;
}

int get_result_title()
{
    char buf[STRLEN];

    if (currlimit.numlogins || currlimit.numposts || currlimit.stay
        || currlimit.day) {
        fprintf(sug, "⊙ 此次投票的允许资格为:\n");
        fprintf(sug, "1. 上站次数需大于 %d 次 .\n", currlimit.numlogins);
        fprintf(sug, "2. 文章数目需大于 %d 篇.\n", currlimit.numposts);
        fprintf(sug, "3. 上站总时数需大于 %d 小时.\n", currlimit.stay);
        fprintf(sug, "4. 上站物理时间需大于 %d 天.\n", currlimit.day);
    }
    if (currvote.type < 1 || currvote.type > 5)
        currvote.type = 1;
    fprintf(sug, "⊙ 投票开启于：%.24s  类别：%s\n",
            ctime(&currvote.opendate)
            , vote_type[currvote.type - 1]);
    fprintf(sug, "⊙ 主题：%s\n", currvote.title);
    if (currvote.type == VOTE_VALUE)
        fprintf(sug, "⊙ 此次投票的值不可超过：%d\n\n", currvote.maxtkt);
    fprintf(sug, "⊙ 票选题目描述：\n\n");
    sprintf(buf, "vote/%s/desc.%lu", currboard->filename, currvote.opendate);
    b_suckinfile(sug, buf);
    return 0;
}
static int mk_result(int num)
{
    char fname[255], nname[255];
    char sugname[255];
    char title[255];
    int i;
    unsigned int total = 0;

    setcontrolfile();
    sprintf(fname, "vote/%s/flag.%lu", currboard->filename, currvote.opendate);
    count_result(NULL, NULL, 0);
    sprintf(sugname, "vote/%s/tmp.%d", currboard->filename, (int)getpid());
    if ((sug = fopen(sugname, "w")) == NULL) {
        bbslog("3error","%s","open vote tmp file error");
        prints("Error: 结束投票错误...\n");
        pressanykey();
    }
    (void) memset(result, 0, sizeof(result));
    if (apply_record
        (fname, (APPLY_FUNC_ARG) count_result, sizeof(struct ballot), 0, 0,
         false) == -1) {
        bbslog("user","%s","Vote apply flag error");
    }
    fprintf(sug,
            "\033[44m\033[36m——————————————┤使用者%s├——————————————\033[m\n\n\n",
            (currvote.type != VOTE_ASKING) ? "建议或意见" : "此次的作答");
    fclose(sug);
    sug = NULL;
    sprintf(nname, "vote/%s/results", currboard->filename);
    if ((sug = fopen(nname, "w")) == NULL) {
        bbslog("user","%s","open vote newresult file error");
        prints("Error: 结束投票错误...\n");
    }
    /*    fprintf( sug, "** 投票开启於：\033[1m%.24s\033[m  类别：\033[1m%s\033[m\n", ctime( &currvote.opendate )
       ,vote_type[currvote.type-1]);
       fprintf( sug, "** 主题：\033[1m%s\033[m\n",currvote.title);
       if(currvote.type==VOTE_VALUE)
       fprintf( sug, "** 此次投票的值不可超过：\033[1m%d\033[m\n\n" ,currvote.maxtkt);
       fprintf( sug, "** 票选题目描述：\n\n" );
       sprintf( buf, "vote/%s/desc.%d",currboard->filename,currvote.opendate );
       b_suckinfile( sug, buf ); */
    get_result_title();
    fprintf(sug, "** 投票结果:\n\n");
    if (currvote.type == VOTE_VALUE) {
        total = result[32];
        for (i = 0; i < 10; i++) {
            fprintf(sug,
                    "\033[1m  %4d\033[m 到 \033[1m%4d\033[m 之间有 \033[1m%4d\033[m 票  约占 \033[1m%d%%\033[m\n",
                    (i * currvote.maxtkt) / 10 + ((i == 0) ? 0 : 1),
                    ((i + 1) * currvote.maxtkt) / 10, result[i]
                    , (result[i] * 100) / ((total <= 0) ? 1 : total));
        }
        fprintf(sug, "此次投票结果平均值是: \033[1m%d\033[m\n",
                result[31] / ((total <= 0) ? 1 : total));
    } else if (currvote.type == VOTE_ASKING) {
        total = result[32];
    } else {
        for (i = 0; i < currvote.totalitems; i++) {
            total += result[i];
        }
        for (i = 0; i < currvote.totalitems; i++) {
            /*            fprintf(sug, "(%c) %-40s  %4d 票  约占 \033[1m%d%%\033[m\n",'A'+i,
               currvote.items[i], result[i] , (result[i]*100)/((total<=0)?1:total)); 
             */
            fprintf(sug, "(%c) %-40s  %4d 票  约占 \033[1m%d%%\033[m\n", 'A' + i,
                    currvote.items[i], result[i],
                    (result[i] * 100) /
                    ((result[32] <= 0) ? 1 : result[32]));
        }
    }
    fprintf(sug, "\n投票总人数 = \033[1m%d\033[m 人\n", result[32]);
    fprintf(sug, "投票总票数 =\033[1m %d\033[m 票\n\n", total);
    fprintf(sug,
            "\033[44m\033[36m——————————————┤使用者%s├——————————————\033[m\n\n\n",
            (currvote.type != VOTE_ASKING) ? "建议或意见" : "此次的作答");
    b_suckinfile(sug, sugname);
    unlink(sugname);
    fclose(sug);
    sug = NULL;
    sprintf(title, "[公告] %s 版的投票结果", currboard->filename);
    mail_file("deliver", nname, currvote.userid, title, 0, NULL);
    if (normal_board(currboard->filename))
    {
        post_file(getCurrentUser(), "", nname, "vote", title, 0, 1, getSession());
    }
    post_file(getCurrentUser(), "", nname, currboard->filename, title, 0, 1, getSession());
    dele_vote(num);
    return 0;
}
int check_result(int num)
{
    char fname[STRLEN], nname[STRLEN];
    char sugname[STRLEN];
    char title[STRLEN];
    int i;
    unsigned int total = 0;

    setcontrolfile();
    sprintf(fname, "vote/%s/flag.%lu", currboard->filename, currvote.opendate);
    count_result(NULL, 0, 0);
    sprintf(sugname, "vote/%s/tmp.%d", currboard->filename, (int)getpid());
    if ((sug = fopen(sugname, "w")) == NULL) {
        bbslog("user","%s","open vote tmp file error");
        prints("Error: 检查投票错误...\n");
        pressanykey();
    }
    (void) memset(result, 0, sizeof(result));
    if (apply_record
        (fname, (APPLY_FUNC_ARG) count_result, sizeof(struct ballot), 0, 0,
         false) == -1) {
        bbslog("user","%s","Vote apply flag error");
    }
    fprintf(sug,
            "\033[44m\033[36m——————————————┤使用者%s├——————————————\033[m\n\n\n",
            (currvote.type != VOTE_ASKING) ? "建议或意见" : "此次的作答");
    fclose(sug);
    sug = NULL;
    sprintf(nname, "vote/%s/results", currboard->filename);
    if ((sug = fopen(nname, "w")) == NULL) {
        bbslog("user","%s","open vote newresult file error");
        prints("Error: 结束投票错误...\n");
    }
    get_result_title();
    fprintf(sug, "** 投票结果:\n\n");
    if (currvote.type == VOTE_VALUE) {
        total = result[32];
        for (i = 0; i < 10; i++) {
            fprintf(sug,
                    "\033[1m  %4d\033[m 到 \033[1m%4d\033[m 之间有 \033[1m%4d\033[m 票  约占 \033[1m%d%%\033[m\n",
                    (i * currvote.maxtkt) / 10 + ((i == 0) ? 0 : 1),
                    ((i + 1) * currvote.maxtkt) / 10, result[i]
                    , (result[i] * 100) / ((total <= 0) ? 1 : total));
        }
        fprintf(sug, "此次投票结果平均值是: \033[1m%d\033[m\n",
                result[31] / ((total <= 0) ? 1 : total));
    } else if (currvote.type == VOTE_ASKING) {
        total = result[32];
    } else {
        for (i = 0; i < currvote.totalitems; i++) {
            total += result[i];
        }
        for (i = 0; i < currvote.totalitems; i++) {
            fprintf(sug, "(%c) %-40s  %4d 票  约占 \033[1m%d%%\033[m\n", 'A' + i,
                    currvote.items[i], result[i],
                    (result[i] * 100) /
                    ((result[32] <= 0) ? 1 : result[32]));
        }
    }
    fprintf(sug, "\n投票总人数 = \033[1m%d\033[m 人\n", result[32]);
    fprintf(sug, "投票总票数 =\033[1m %d\033[m 票\n\n", total);
    fprintf(sug,
            "\033[44m\033[36m——————————————┤使用者%s├——————————————\033[m\n\n\n",
            (currvote.type != VOTE_ASKING) ? "建议或意见" : "此次的作答");
    b_suckinfile(sug, sugname);
    unlink(sugname);
    fclose(sug);
    sug = NULL;
    sprintf(title, "[检查] %s 版的投票结果", currboard->filename);
    mail_file(getCurrentUser()->userid, nname, getCurrentUser()->userid, title, BBSPOST_MOVE, NULL);
    return 0;
}

int get_vitems(bal)
struct votebal *bal;
{
    int num, oldnum=-1;
    char buf[STRLEN];

    for (num = 0; num < 32; num++)
        bal->items[num][0] = '\0';
    move(3, 0);
    prints("请依序输入可选择项, 按 ENTER 完成设定.\n");

    while(1) {
    for (num = 0; num < 32; num++) {
        sprintf(buf, "%c) ", num + 'A');
        getdata((num % 16) + 4, (num / 16) * 40, buf, bal->items[num], 36,
                DOECHO, NULL, false);
        if (strlen(bal->items[num]) == 0) {
            if (num != 0)
                break;
            num = -1;
        }
    }
    if(oldnum != -1)
    {
        int i;
        for(i=num+1; i<oldnum; i++)
        {
            move((i % 16) + 4, (i / 16) * 40);
            prints("                                        ");
        }
    }
    oldnum = num;
    move(20, 0);
    getdata(21, 0, "需要再编辑选项内容吗(Y/N)? [N]: ", buf, 2, DOECHO, NULL, true);
    if(buf[0] != 'Y' && buf[0] != 'y')
        break;
    }

    bal->totalitems = num;
    return num;
}

int vote_maintain(bname)
char *bname;
{
    char buf[STRLEN * 2];
    struct votebal *ball = &currvote;
    struct votelimit *v_limit = &currlimit;
    int aborted;

    setcontrolfile();
    if (!HAS_PERM(getCurrentUser(), PERM_SYSOP))
            if (!chk_currBM(currBM, getCurrentUser())) {
                return 0;
            }
    stand_title("开启投票箱");
    makevdir(bname);
    for (;;) {
        getdata(2, 0,
                "(1)是非, (2)单选, (3)复选, (4)数值 (5)问答 (6)取消 ? : ",
                genbuf, 2, DOECHO, NULL, true);
        genbuf[0] -= '0';
        if (genbuf[0] == 6) {
            prints("取消此次投票\n");
            sleep(1);
            return FULLUPDATE;
        }
        if (genbuf[0] < 1 || genbuf[0] > 5)
            continue;
        ball->type = (int) genbuf[0];
        break;
    }
    ball->opendate = time(NULL);
    prints("按任何键开始编辑此次 [投票的描述]: \n");
    pressanykey();
    setvfile(genbuf, bname, "desc");
    sprintf(buf, "%s.%lu", genbuf, ball->opendate);
    aborted = vedit(buf, false, NULL, NULL, 0);
    if (aborted) {
        clear();
        prints("取消此次投票\n");
        pressreturn();
        return FULLUPDATE;
    }
    clear();
    getdata(0, 0, "此次投票所须天数 (不可０天): ", buf, 4, DOECHO, NULL,
            true);
    if (*buf == '\n' || atoi(buf) == 0 || *buf == '\0')
        strcpy(buf, "1");
    ball->maxdays = atoi(buf);
    for (;;) {
        getdata(1, 0, "投票箱的标题: ", ball->title, 61, DOECHO, NULL,
                true);
        if (strlen(ball->title) > 0)
            break;
        bell();
    }
    switch (ball->type) {
    case VOTE_YN:
        ball->maxtkt = 0;
        strcpy(ball->items[0], "赞成  （是的）");
        strcpy(ball->items[1], "不赞成（不是）");
        strcpy(ball->items[2], "没意见（不清楚）");
        ball->maxtkt = 1;
        ball->totalitems = 3;
        break;
    case VOTE_SINGLE:
        get_vitems(ball);
        ball->maxtkt = 1;
        break;
    case VOTE_MULTI:
        get_vitems(ball);
        for (;;) {
            getdata(21, 0, "一个人最多几票? [1]: ", buf, 5, DOECHO, NULL,
                    true);
            ball->maxtkt = atoi(buf);
            if (ball->maxtkt <= 0)
                ball->maxtkt = 1;
            if (ball->maxtkt > ball->totalitems)
                continue;
            break;
        }
        break;
    case VOTE_VALUE:
        for (;;) {
            getdata(3, 0, "输入数值最大不得超过 [100] : ", buf, 7, DOECHO,
                    NULL, true);
            ball->maxtkt = atoi(buf);
            if (ball->maxtkt <= 0)
                ball->maxtkt = 100;
            break;
        }
        break;
    case VOTE_ASKING:
        /*                    getdata(3,0,"此问答题作答行数之限制 :",buf,3,DOECHO,NULL,true) ;
           ball->maxtkt = atof(buf) ;
           if(ball->maxtkt <= 0) ball->maxtkt = 10; */
        ball->maxtkt = 0;
        currvote.totalitems = 0;
        break;
    }
    setvoteflag(bname, 1);
    clear();
    /*Haohmaru.99.11.17.根据投票管理员设的限制条件判断是否让该使用者投票 */
    if (HAS_PERM(getCurrentUser(), PERM_SYSOP)
        || HAS_PERM(getCurrentUser(), PERM_JURY)) {
        getdata(1, 0, "是否对投票资格进行限制(Y/N) [Y]:", buf, 3, DOECHO,
                NULL, true);
        if (buf[0] != 'N' && buf[0] != 'n') {
            getdata(2, 0,
                    "请输入对上站次数的限制(0为没有限制),上站次数大于:",
                    buf, 5, DOECHO, NULL, true);
            v_limit->numlogins = atoi(buf);
            getdata(3, 0,
                    "请输入对文章数目的限制(0为没有限制),文章数目大于:",
                    buf, 5, DOECHO, NULL, true);
            v_limit->numposts = atoi(buf);
            getdata(4, 0,
                    "请输入对上站总时数的限制(0为没有限制),上站总时数大于(小时):",
                    buf, 5, DOECHO, NULL, true);
            v_limit->stay = atoi(buf);
            getdata(5, 0,
                    "请输入对上站物理时间的限制(0为没有限制),上站物理时间大于(天):",
                    buf, 5, DOECHO, NULL, true);
            v_limit->day = atoi(buf);
        } else {
            v_limit->numlogins = 0;
            v_limit->numposts = 0;
            v_limit->stay = 0;
            v_limit->day = 0;
        }
    } else {
        v_limit->numlogins = 0;
        v_limit->numposts = 0;
        v_limit->stay = 0;
        v_limit->day = 0;
    }
    clear();
    sprintf(limitfile, "vote/%s/limit.%lu", bname, ball->opendate);
    if (append_record(limitfile, v_limit, sizeof(struct votelimit)) == -1) {
        prints("发生严重的错误，无法写入限制文件，请通告站长");
        b_report("Append limit file Error!!");
    }
    /*Haohmaru.99.10.26.add below 8 lines */
    getdata(1, 0, "确定开启投票?[Y] :", buf, 3, DOECHO, NULL, true);
    if (buf[0] == 'N' || buf[0] == 'n') {
        clear();
        prints("取消此次投票\n");
        unlink(limitfile);
        pressreturn();
        return FULLUPDATE;
    }
    strcpy(ball->userid, getCurrentUser()->userid);
    if (append_record(controlfile, ball, sizeof(*ball)) == -1) {
        prints("发生严重的错误，无法开启投票，请通告站长");
        b_report("Append Control file Error!!");
    } else {
        char votename[STRLEN];
        int i;

        b_report("OPEN VOTE");
        prints("投票箱已经开启了！\n");
        range++;
		gettmpfilename( votename, "votetmp" );
        //sprintf(votename, "tmp/votetmp.%d", getpid());
        if ((sug = fopen(votename, "w")) != NULL) {
            sprintf(buf, "[通知] %s 举办投票：%s", bname, ball->title);
            get_result_title();
            if (ball->type != VOTE_ASKING && ball->type != VOTE_VALUE) {
                fprintf(sug, "\n【选项如下】\n");
                for (i = 0; i < ball->totalitems; i++) {
                    fprintf(sug, "(\033[1m%c\033[m) %-40s\n", 'A' + i,
                            ball->items[i]);
                }
            }
            fclose(sug);
            sug = NULL;
            if (normal_board(bname)) {
                post_file(getCurrentUser(), "", votename, "vote", buf, 0, 1, getSession());
            }
            post_file(getCurrentUser(), "", votename, bname, buf, 0, 1, getSession());
            unlink(votename);
        }
    }
    pressreturn();
    return FULLUPDATE;
}
int vote_flag(char *bname, char val, int mode)
{
    char buf[STRLEN], flag;
    int fd, num, size;

    num = getSession()->currentuid - 1;
    switch (mode) {
    case 2:
        sprintf(buf, "Welcome.rec");    /*进站的 Welcome 画面 */
        break;
    case 1:
        setvfile(buf, bname, "noterec");        /*讨论区备忘录的旗标 */
        break;
    default:
        return -1;
    }
    if (num >= MAXUSERS) {
        bbslog("user","%s","Vote Flag, Out of User Numbers");
        return -1;
    }
    if ((fd = open(buf, O_RDWR | O_CREAT, 0600)) == -1) {
        char buf[STRLEN];

        sprintf(buf, "%s Flag file open Error.", bname);
        bbslog("user","%s",buf);
        return -1;
    }
    flock(fd, LOCK_EX);
    size = (int) lseek(fd, 0, SEEK_END);
    memset(buf, 0, sizeof(buf));
    while (size <= num) {
        write(fd, buf, sizeof(buf));
        size += sizeof(buf);
    }
    lseek(fd, num, SEEK_SET);
    read(fd, &flag, 1);
    if ((flag == 0 && val != 0)) {
        lseek(fd, num, SEEK_SET);
        write(fd, &val, 1);
    }
    flock(fd, LOCK_UN);
    close(fd);
    return flag;
}

int vote_check(bits)
int bits;
{
    int i, count;

    for (i = count = 0; i < 32; i++) {
        if ((bits >> i) & 1)
            count++;
    }
    return count;
}
struct _setperm_select {
    unsigned int pbits;
    unsigned int basic;
    unsigned int oldbits;
};
int vote_select(struct _select_def *conf)
{
    int count;
    struct _setperm_select *arg = (struct _setperm_select *) conf->arg;

    if (conf->pos == conf->item_count)
        return SHOW_QUIT;
    arg->pbits ^= (1 << (conf->pos - 1));
    count = vote_check(arg->pbits);
    if (count > currvote.maxtkt) {
        if (currvote.maxtkt == 1) {
            arg->pbits = (1 << (conf->pos - 1));
            return SHOW_REFRESH;
        } else
            arg->pbits ^= (1 << (conf->pos - 1));
        return SHOW_CONTINUE;
    }
    move(2, 0);
    clrtoeol();
    prints("你已经投了 %d 票", count);
    return SHOW_REFRESHSELECT;
}
int showvoteitems(struct _select_def *conf, int i)
{
    char buf[STRLEN];
    struct _setperm_select *arg = (struct _setperm_select *) conf->arg;

    i = i - 1;
    if (i == conf->item_count - 1) {
        prints("--退出--");
    } else {
        sprintf(buf, "%c.%2.2s%-36.36s", 'A' + i,
                ((arg->pbits >> i) & 1 ? "◎" : "  "), currvote.items[i]);
        prints("%s", buf);
    }
    return SHOW_CONTINUE;
}

void show_voteing_title()
{
    time_t closedate;
    char buf[STRLEN];

    if (currvote.type != VOTE_VALUE && currvote.type != VOTE_ASKING)
        sprintf(buf, "可投票数: %d 票", currvote.maxtkt);
    else
        buf[0] = '\0';
    closedate = currvote.opendate + currvote.maxdays * 86400;
    prints("投票将结束于: %24s  %s  %s\n",
           ctime(&closedate), buf, (voted_flag) ? "(修改前次投票)" : "");
    prints("投票主题是: \033[1m%-50s\033[m类型: \033[1m%s\033[m \n", currvote.title,
           vote_type[currvote.type - 1]);
}

int getsug(uv)
struct ballot *uv;
{
    int i, line;

    move(0, 0);
    clrtobot();
    if (currvote.type == VOTE_ASKING) {
        show_voteing_title();
        line = 3;
        prints("请填入您的作答(三行):\n");
    } else {
        line = 1;
        prints("请填入您宝贵的意见(三行):\n");
    }
    move(line, 0);
    for (i = 0; i < 3; i++) {
        prints(": %s\n", uv->msg[i]);
    }
    for (i = 0; i < 3; i++) {
        getdata(line + i, 0, ": ", uv->msg[i], STRLEN - 2, DOECHO, NULL,
                false);
        if (uv->msg[i][0] == '\0')
            break;
    }
    return i;
}

int multivote(uv)
struct ballot *uv;
{
    unsigned int i;

    i = uv->voted;
    move(0, 0);
    show_voteing_title();
    uv->voted =
        setperms(uv->voted, 0, "选票", currvote.totalitems, showvoteitems,
                 vote_select);
    if (uv->voted == i)
        return -1;
    return 1;
}

int valuevote(uv)
struct ballot *uv;
{
    unsigned int chs;
    char buf[10];

    chs = uv->voted;
    move(0, 0);
    show_voteing_title();
    prints("此次作答的值不能超过 \033[1m%d\033[m", currvote.maxtkt);
    if (uv->voted != 0)
        sprintf(buf, "%d", uv->voted);
    else
        memset(buf, 0, sizeof(buf));
    do {
        getdata(3, 0, "请输入一个值? [0]: ", buf, 5, DOECHO, NULL, false);
        uv->voted = abs(atoi(buf));
    } while ((int) uv->voted > currvote.maxtkt && buf[0] != '\n'
             && buf[0] != '\0');
    if (buf[0] == '\n' || buf[0] == '\0' || uv->voted == chs)
        return -1;
    return 1;
}
int user_vote(int num)
{
    char fname[STRLEN], bname[STRLEN];
    char buf[STRLEN];
    struct ballot uservote, tmpbal;
    struct votelimit userlimit;
    int votevalue;
    int aborted = false, pos;

    if(!haspostperm(getCurrentUser(),currboard->filename)
#ifdef NEWSMTH
        ||!check_score_level(getCurrentUser(),currboard)
#endif /* NEWSMTH */
        )
	    return -1;
    move(t_lines - 2, 0);
    get_record(controlfile, &currvote, sizeof(struct votebal), num);
    sprintf(fname, "vote/%s/flag.%lu", currboard->filename, currvote.opendate);
    if ((pos =
         search_record(fname, &uservote, sizeof(uservote),
                       (RECORD_FUNC_ARG) cmpvuid,
                       getCurrentUser()->userid)) <= 0) {
        (void) memset(&uservote, 0, sizeof(uservote));
        voted_flag = false;
    } else {
        voted_flag = true;
    }
    strcpy(uservote.uid, getCurrentUser()->userid);
    sprintf(bname, "desc.%lu", currvote.opendate);
    setvfile(buf, currboard->filename, bname);
    ansimore(buf, true);
    move(0, 0);
    /*Haohmaru.99.11.17.根据版主设的限制条件判断是否让该使用者投票 */
    clear();
    userlimit.numlogins = 0;
    userlimit.numposts = 0;
    userlimit.stay = 0;
    userlimit.day = 0;
    sprintf(limitfile, "vote/%s/limit.%lu", currboard->filename, currvote.opendate);
    get_record(limitfile, &userlimit, sizeof(struct votelimit), 1);
    if ((currvote.type <= 0) || (currvote.type > 5))
        currvote.type = 1;
    if ((getCurrentUser()->numposts < userlimit.numposts
         || getCurrentUser()->numlogins < userlimit.numlogins
         || getCurrentUser()->stay < userlimit.stay * 60 * 60
         || (time(NULL) - getCurrentUser()->firstlogin) <
         userlimit.day * 24 * 60 * 60)) {
        prints
            ("对不起,你不满足版主规定的此次投票所需条件,无法参加投票,谢谢参与,下次再见! :)");
        pressanykey();
        return -1;
    }
    clrtobot();
    switch (currvote.type) {
    case VOTE_SINGLE:
    case VOTE_MULTI:
    case VOTE_YN:
        votevalue = multivote(&uservote);
        if (votevalue == -1)
            aborted = true;
        break;
    case VOTE_VALUE:
        votevalue = valuevote(&uservote);
        if (votevalue == -1)
            aborted = true;
        break;
    case VOTE_ASKING:
        uservote.voted = 0;
        aborted = !getsug(&uservote);
        break;
    }
    clear();
    if (aborted == true) {
        prints("保留 【%s】原来的的投票。\n", currvote.title);
    } else {
        if (currvote.type != VOTE_ASKING)
            getsug(&uservote);
        pos =
            search_record(fname, &tmpbal, sizeof(tmpbal),
                          (RECORD_FUNC_ARG) cmpvuid, getCurrentUser()->userid);
        if (pos) {
            substitute_record(fname, &uservote, sizeof(uservote), pos);
        } else if (append_record(fname, &uservote, sizeof(uservote)) == -1) {
            move(2, 0);
            clrtoeol();
            prints("投票失败! 请通知站长参加那一个选项投票\n");
            pressreturn();
        }
        prints("\n已经帮你(□)投入票箱中...\n");
    }
    pressanykey();
    return 0;
}

void voteexp()
{
    clrtoeol();
    prints("\033[44m编号 开启投票箱者 开启日 %-40s类别 天数 人数\033[m\n",
           "投票主题", "");
}
int printvote(struct votebal *ent, int idx, int *i)
{
    struct ballot uservote;
    char buf[STRLEN + 80], *date;
    char flagname[STRLEN];
    int num_voted;

    if (ent == NULL) {
        move(2, 0);
        voteexp();
        *i = 0;
        return 0;
    }
    (*i)++;
    if (*i > page + 19 || *i > range)
        return QUIT;
    else if (*i <= page)
        return 0;
    sprintf(buf, "flag.%lu", ent->opendate);
    setvfile(flagname, currboard->filename, buf);
    if (search_record
        (flagname, &uservote, sizeof(uservote), (RECORD_FUNC_ARG) cmpvuid,
         getCurrentUser()->userid) <= 0) {
        voted_flag = false;
    } else
        voted_flag = true;
    num_voted = get_num_records(flagname, sizeof(struct ballot));
    date = ctime(&ent->opendate) + 4;
    if ((ent->type <= 0) || (ent->type > 5))
        ent->type = 1;
    sprintf(buf, " %s%3d %-12.12s %-6.6s %-40.40s%-4.4s %3d %5d\033[m\n",
            (voted_flag == false) ? "\033[1m" : "", *i, ent->userid, date,
            ent->title, vote_type[ent->type - 1], ent->maxdays, num_voted);
/*
    sprintf(buf," %s%3d %-12.12s %-6.6s %-40.40s%-4.4s %3d  %4d\033[m\n",(voted_flag==false)?"\033[1m":"",*i,ent->userid,
            date,ent->title,vote_type[ent->type-1],ent->maxdays,num_voted);
*/
    prints("%s", buf);
    return 0;
}
static int dele_vote(int num)
{
    char buf[STRLEN];

    sprintf(buf, "vote/%s/flag.%lu", currboard->filename, currvote.opendate);
    unlink(buf);
    sprintf(buf, "vote/%s/desc.%lu", currboard->filename, currvote.opendate);
    unlink(buf);
    sprintf(buf, "vote/%s/limit.%lu", currboard->filename, currvote.opendate);    /*Haohmaru.99.11.18 */
    unlink(buf);
    if (delete_record(controlfile, sizeof(currvote), num, NULL, NULL) ==
        -1) {
        prints("发生错误，请通知站长....");
        pressanykey();
    }
    range--;
    if (get_num_records(controlfile, sizeof(currvote)) == 0) {
        setvoteflag(currboard->filename, 0);
    }
    return 0;
}

int vote_results(bname)
char *bname;
{
    char buf[STRLEN];

    setvfile(buf, bname, "results");
    if (ansimore(buf, true) == -1) {
        move(3, 0);
        prints("目前没有任何投票的结果。\n");
        clrtobot();
        pressreturn();
    } else
        clear();
    return FULLUPDATE;
}

int b_vote_maintain(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{
	int ret;
	int oldmode;
#ifdef NEW_HELP
	int oldhelpmode = helpmode;
	helpmode = HELP_VOTE;
#endif

    oldmode = uinfo.mode;
	modify_user_mode(VOTING);
    ret =  vote_maintain(currboard->filename);
    modify_user_mode(oldmode);

#ifdef NEW_HELP
	helpmode = oldhelpmode;
#endif
	return ret;
}

int vote_title()
{
    docmdtitle("[投票箱列表]",
                "\033[m离开[\033[1;32m←\033[m,\033[1;32me\033[m] 求助[\033[1;32mh\033[m] 进行投票[\033[1;32m→\033[m,\033[1;32mr <cr>\033[m] 上,下选择[\033[1;32m↑\033[m,\033[1;32m↓\033[m] \033[1m高亮度\033[m表示尚未投票");
    move(2, 0);
    prints("\033[0;1;37;44m编号 开启投票箱者 开启日 投票主题                                类别 天数 人数");
    clrtoeol();
    update_endline();
    return 0;
}

int vote_key(ch, allnum, pagenum)
int ch;
int allnum, pagenum;
{
    int deal = 0, ans;
    char buf[STRLEN];

    switch (ch) {
    case 'v':
    case 'V':
    case '\n':
    case '\r':
    case 'r':
    case KEY_RIGHT:
        user_vote(allnum + 1);
        deal = 1;
        break;
    case 'R':
        vote_results(currboard->filename);
        deal = 1;
        break;
    case 'H':
    case 'h':
        show_help("help/votehelp");
        deal = 1;
        break;
    case 'A':
    case 'a':
        if (!chk_currBM(currBM, getCurrentUser()))
            return true;
        vote_maintain(currboard->filename);
        deal = 1;
        break;
    case 'O':
    case 'o':
        if (!chk_currBM(currBM, getCurrentUser()))
            return true;
        clear();
        deal = 1;
        get_record(controlfile, &currvote, sizeof(struct votebal),
                   allnum + 1);
        prints("\033[31m警告!!\033[m\n");
        prints("投票箱标题：\033[1m%s\033[m\n", currvote.title);
        ans = askyn("你确定要提早结束这个投票吗", 0);
        if (ans != 1) {
            move(2, 0);
            prints("取消删除行动\n");
            pressreturn();
            clear();
            break;
        }
        mk_result(allnum + 1);
        sprintf(buf, "提早结束投票 %s", currvote.title);
        /* securityreport(buf, NULL, NULL, getSession()); */
        bbslog("user","%s",buf);
        break;
    case '@':
        if (!HAS_PERM(getCurrentUser(), PERM_SYSOP))
            return true;
        clear();
        deal = 1;
        get_record(controlfile, &currvote, sizeof(struct votebal),
                   allnum + 1);
        prints("检查投票：\x1b[1m%s\x1b[m\n", currvote.title);
        check_result(allnum + 1);
        break;
    case 'D':
    case 'd':
        if (!chk_currBM(currBM, getCurrentUser())) {
            return 1;
        }
        deal = 1;
        get_record(controlfile, &currvote, sizeof(struct votebal),
                   allnum + 1);
        clear();
        prints("\033[31m警告!!\033[m\n");
        prints("投票箱标题：\033[1m%s\033[m\n", currvote.title);
        ans = askyn("你确定要强制关闭这个投票吗", 0);
        if (ans != 1) {
            move(2, 0);
            prints("取消删除行动\n");
            pressreturn();
            clear();
            break;
        }
        sprintf(buf, "强制关闭投票 %s", currvote.title);
        /* securityreport(buf, NULL, NULL, getSession()); */
        bbslog("user","%s",buf);
        dele_vote(allnum + 1);
        break;
    default:
        return 0;
    }
    if (deal) {
        Show_Votes();
        vote_title();
    }
    return 1;
}
static int Show_Votes()
{
    int i;

    move(3, 0);
    clrtobot();
    i = 0;
    setcontrolfile();
    if (apply_record
        (controlfile, (APPLY_FUNC_ARG) printvote, sizeof(struct votebal),
         &i, 0, false) == -1) {
        prints("错误，没有投票箱开启....");
        pressreturn();
        return 0;
    }
    clrtobot();
    return 0;
}

int b_vote(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{
    int num_of_vote;
    int voting;
#ifdef NEW_HELP
	int oldhelpmode = helpmode;
#endif

    if (!HAS_PERM(getCurrentUser(), PERM_LOGINOK))
        return 0;               /* Leeward 98.05.15 */
    setcontrolfile();
    num_of_vote = get_num_records(controlfile, sizeof(struct votebal));
    if (num_of_vote == 0) {
        move(3, 0);
        clrtobot();
        prints("抱歉, 目前并没有任何投票举行。\n");
        pressreturn();
        setvoteflag(currboard->filename, 0);
        return FULLUPDATE;
    }
    setlistrange(num_of_vote);
    clear();
#ifdef NEW_HELP
	helpmode = HELP_VOTE;
#endif
    voting = choose(false, 0, vote_title, vote_key, Show_Votes, user_vote);
#ifdef NEW_HELP
	helpmode = oldhelpmode;
#endif
    clear();
    return /*user_vote( currboard->filename ) */ FULLUPDATE;
}

int m_vote(void){
    modify_user_mode(ADMIN);
    vote_maintain(DEFAULTBOARD);
    return 0;
}

int x_vote(void){
    struct boardheader* bh;
    int bid;
    modify_user_mode(XMENU);
    bh=currboard;
    bid=currboardent;
    b_vote(NULL,NULL,NULL);
    currboard=bh;
    currboardent=bid;
    return 0;
}

int x_results(void){
    modify_user_mode(XMENU);
    return vote_results(DEFAULTBOARD);
}

