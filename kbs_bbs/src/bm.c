
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

    checked global variable
*/

#include "bbs.h"
#include "read.h"
#include <time.h>
#include <ctype.h>

/*Add by SmallPig*/

int listdeny(int page)
{                               /* Haohmaru.12.18.98.为那些变态得封人超过一屏的版主而写 */
    FILE *fp;
    int x = 0, y = 3, cnt = 0, max = 0, len;
    int i;
    char u_buf[STRLEN * 2], line[STRLEN * 2], *nick;

    clear();
    prints("设定无法 Post 的名单\n");
    move(y, x);
    CreateNameList();
    setbfile(genbuf, currboard->filename, "deny_users");
    if ((fp = fopen(genbuf, "r")) == NULL) {
        prints("(none)\n");
        return 0;
    }
    for (i = 1; i <= page * 20; i++) {
        if (fgets(genbuf, 2 * STRLEN, fp) == NULL)
            break;
    }
    while (fgets(genbuf, 2 * STRLEN, fp) != NULL) {
        strtok(genbuf, " \n\r\t");
        strcpy(u_buf, genbuf);
        AddNameList(u_buf);
        nick = (char *) strtok(NULL, "\n\r\t");
        if (nick != NULL) {
            while (*nick == ' ')
                nick++;
            if (*nick == '\0')
                nick = NULL;
        }
        if (nick == NULL) {
            strcpy(line, u_buf);
        } else {
            if (cnt < 20)
                sprintf(line, "%-12s%s", u_buf, nick);
        }
        if ((len = strlen(line)) > max)
            max = len;
/*        if( x + len > 90 )
            line[ 90 - x ] = '\0';*-P-*/
        if (x + len > 79)
            line[79] = '\0';
        if (cnt < 20)           /*haohmaru.12.19.98 */
            prints("%s", line);
        cnt++;
        if ((++y) >= t_lines - 1) {
            y = 3;
            x += max + 2;
            max = 0;
            /*
             * if( x > 90 )  break; 
             */
        }
        move(y, x);
    }
    fclose(fp);
    if (cnt == 0)
        prints("(none)\n");
    return cnt;
}

int addtodeny(char *uident)
{                               /* 添加 禁止POST用户 */
    char buf2[50], strtosave[256], date[STRLEN] = "0";
    int maxdeny;

    /*
     * Haohmaru.99.4.1.auto notify 
     */
    time_t now;
    char buffer[STRLEN];
    FILE *fn;
    char filename[STRLEN];
    int autofree = 0;
    char filebuf[STRLEN];
    char denymsg[STRLEN];
    int denyday;
    int reasonfile;

    now = time(0);
    strncpy(date, ctime(&now) + 4, 7);
    setbfile(genbuf, currboard->filename, "deny_users");
    if (seek_in_file(genbuf, uident) || !strcmp(currboard->filename, "denypost"))
        return -1;
    if (HAS_PERM(getCurrentUser(), PERM_SYSOP) || HAS_PERM(getCurrentUser(), PERM_OBOARDS))
        maxdeny = 70;
    else
        maxdeny = 14;

    *denymsg = 0;
    if ((reasonfile = open("etc/deny_reason", O_RDONLY)) != -1) {
        int reason = -1;
        int maxreason;
        char *file_buf;
        char *denymsglist[50];
        struct stat st;

        move(3, 0);
        clrtobot();
        if (fstat(reasonfile, &st) == 0) {
            int i;

            file_buf = (char *) malloc(st.st_size);
            read(reasonfile, file_buf, st.st_size);
            maxreason = 1;
            denymsglist[0] = file_buf;
            prints("%s", "封禁理由列表.\n");
            for (i = 1; i < st.st_size; i++) {
                if (file_buf[i] == '\n') {
                    file_buf[i] = 0;
                    prints("%d.%s\n", maxreason, denymsglist[maxreason - 1]);
                    if (i == st.st_size - 1)
                        break;
                    if (file_buf[i + 1] == '\r') {
                        if (i + 1 == st.st_size - 1)
                            break;
                        denymsglist[maxreason] = &file_buf[i + 2];
                        maxreason++;
                        i += 2;
                    } else {
                        denymsglist[maxreason] = &file_buf[i + 1];
                        maxreason++;
                        i++;
                    }
                }
            }
            prints("%s", "0.手动输入封禁理由");
            while (1) {
                getdata(2, 0, "请从列表选择封禁理由(0为手工输入,*退出):", denymsg, 2, DOECHO, NULL, true);
                if (denymsg[0] == '*') {
                    free(file_buf);
                    close(reasonfile);
                    return 0;
                }
                if (isdigit(denymsg[0])) {
                    reason = atoi(denymsg);
                    if (reason == 0) {
                        denymsg[0] = 0;
                        move(2, 0);
                        clrtobot();
                        break;
                    }
                    if (reason <= maxreason) {
                        strncpy(denymsg, denymsglist[reason - 1], STRLEN - 1);
                        denymsg[STRLEN - 1] = 0;
                        move(2, 0);
                        clrtobot();
                        prints("封禁理由: %s\n", denymsg);
                        break;
                    }
                }
                move(3, 0);
				clrtoeol();
                prints("%s", "输入错误!");
            }
            free(file_buf);
        }
        close(reasonfile);
    }

    while (0 == strlen(denymsg)) {
        getdata(2, 0, "输入说明(按*取消): ", denymsg, 30, DOECHO, NULL, true);
    }
    if (denymsg[0] == '*')
        return 0;
#ifdef MANUAL_DENY
    autofree = askyn("该封禁是否自动解封？(选 \033[1;31mY\033[m 表示进行自动解封)", true);
#else
    autofree = true;
#endif
    sprintf(filebuf, "输入天数(最长%d天)(按*取消封禁)", maxdeny);
    denyday = 0;
    while (!denyday) {
        getdata(3, 0, filebuf, buf2, 4, DOECHO, NULL, true);
	if (buf2[0] == '*')return 0; 
        if ((buf2[0] < '0') || (buf2[0] > '9'))
            continue;           /*goto MUST1; */
        denyday = atoi(buf2);
        if ((denyday < 0) || (denyday > maxdeny))
            denyday = 0;        /*goto MUST1; */
        else if (!(HAS_PERM(getCurrentUser(), PERM_SYSOP) || HAS_PERM(getCurrentUser(), PERM_OBOARDS)) && !denyday)
            denyday = 0;        /*goto MUST1; */
        else if ((HAS_PERM(getCurrentUser(), PERM_SYSOP) || HAS_PERM(getCurrentUser(), PERM_OBOARDS)) && !denyday && !autofree)
            break;
    }
#ifdef ZIXIA
	int ndenypic,dpcount;
	clear();
	dpcount=CountDenyPic(DENYPIC);
	sprintf(filebuf, "选择封禁附图(1-%d)(0为随机选择,V为看图片)[0]:",dpcount);
	ndenypic=-1;
	while(ndenypic>dpcount || ndenypic<0)
	{
		getdata(0,0,filebuf,buf2,4, DOECHO, NULL, true);
		if(buf2[0]=='v' ||buf2[0]=='V' )
		{
			ansimore(DENYPIC, 0);
			continue;
		}
		if(buf2[0]=='\0')
		{
			ndenypic=0;
			break;
		}
		if ((buf2[0] < '0') || (buf2[0] > '9'))
		       continue; 
		ndenypic = atoi(buf2);
	}
#endif
    if (denyday && autofree) {
        struct tm *tmtime;
        time_t undenytime = now + denyday * 24 * 60 * 60;

        tmtime = gmtime(&undenytime);

        sprintf(strtosave, "%-12.12s %-30.30s%-12.12s %2d月%2d日解\x1b[%lum", uident, denymsg, getCurrentUser()->userid, tmtime->tm_mon + 1, tmtime->tm_mday, undenytime);   /*Haohmaru 98,09,25,显示是谁什么时候封的 */
    } else {
        struct tm *tmtime;
        time_t undenytime = now + denyday * 24 * 60 * 60;

        tmtime = gmtime(&undenytime);
        sprintf(strtosave, "%-12.12s %-30.30s%-12.12s %2d月%2d日后\x1b[%lum", uident, denymsg, getCurrentUser()->userid, tmtime->tm_mon + 1, tmtime->tm_mday, undenytime);
    }

    if (addtofile(genbuf, strtosave) == 1) {
        struct userec *lookupuser, *saveptr;
        int my_flag = 0;        /* Bigman. 2001.2.19 */
        struct userec saveuser;

        /*
         * Haohmaru.4.1.自动发信通知并发文章于版上 
         */
		gettmpfilename( filename, "deny" );
        //sprintf(filename, "tmp/%s.deny", getCurrentUser()->userid);
        fn = fopen(filename, "w+");
        memcpy(&saveuser, getCurrentUser(), sizeof(struct userec));
        saveptr = getCurrentUser();
        getCurrentUser() = &saveuser;
        sprintf(buffer, "%s被取消在%s版的发文权限", uident, currboard->filename);

        if ((HAS_PERM(getCurrentUser(), PERM_SYSOP) || HAS_PERM(getCurrentUser(), PERM_OBOARDS)) && !chk_BM_instr(currBM, getCurrentUser()->userid)) {
            my_flag = 0;
            fprintf(fn, "寄信人: SYSOP (System Operator) \n");
            fprintf(fn, "标  题: %s\n", buffer);
            fprintf(fn, "发信站: %s (%24.24s)\n", BBS_FULL_NAME, ctime(&now));
            fprintf(fn, "来  源: %s\n", NAME_BBS_ENGLISH);
            fprintf(fn, "\n");
            fprintf(fn, "由于您在 \x1b[4m%s\x1b[m 版 \x1b[4m%s\x1b[m，我很遗憾地通知您， \n", currboard->filename, denymsg);
            if (denyday)
                fprintf(fn, DENY_DESC_AUTOFREE " \x1b[4m%d\x1b[m 天", denyday);
            else
                fprintf(fn, DENY_DESC_NOAUTOFREE);
            if (!autofree)
                fprintf(fn, "，到期后请回复\n此信申请恢复权限。\n");
            fprintf(fn, "\n");
#ifdef ZIXIA
            ndenypic=GetDenyPic(fn, DENYPIC, ndenypic, dpcount);
#endif
            fprintf(fn, "                            %s" NAME_SYSOP_GROUP DENY_NAME_SYSOP "：\x1b[4m%s\x1b[m\n", NAME_BBS_CHINESE, getCurrentUser()->userid);
            fprintf(fn, "                              %s\n", ctime(&now));
            strcpy(getCurrentUser()->userid, "SYSOP");
            strcpy(getCurrentUser()->username, NAME_SYSOP);
            /*strcpy(getCurrentUser()->realname, NAME_SYSOP);*/
        } else {
            my_flag = 1;
            fprintf(fn, "寄信人: %s \n", getCurrentUser()->userid);
            fprintf(fn, "标  题: %s\n", buffer);
            fprintf(fn, "发信站: %s (%24.24s)\n", BBS_FULL_NAME, ctime(&now));
            fprintf(fn, "来  源: %s \n", SHOW_USERIP(getCurrentUser(), getSession()->fromhost));
            fprintf(fn, "\n");
            fprintf(fn, "由于您在 \x1b[4m%s\x1b[m 版 \x1b[4m%s\x1b[m，我很遗憾地通知您， \n", currboard->filename, denymsg);
            if (denyday)
                fprintf(fn, DENY_DESC_AUTOFREE " \x1b[4m%d\x1b[m 天", denyday);
            else
                fprintf(fn, DENY_DESC_NOAUTOFREE);
            if (!autofree)
                fprintf(fn, "，到期后请回复\n此信申请恢复权限。\n");
            fprintf(fn, "\n");
#ifdef ZIXIA
            ndenypic=GetDenyPic(fn, DENYPIC, ndenypic, dpcount);
#endif
            fprintf(fn, "                              " NAME_BM ":\x1b[4m%s\x1b[m\n", getCurrentUser()->userid);
            fprintf(fn, "                              %s\n", ctime(&now));
        }
        fclose(fn);
        mail_file(getCurrentUser()->userid, filename, uident, buffer, 0, NULL);
        fn = fopen(filename, "w+");
        fprintf(fn, "由于 \x1b[4m%s\x1b[m 在 \x1b[4m%s\x1b[m 版的 \x1b[4m%s\x1b[m 行为，\n", uident, currboard->filename, denymsg);
        if (denyday)
            fprintf(fn, DENY_BOARD_AUTOFREE " \x1b[4m%d\x1b[m 天。\n", denyday);
        else
            fprintf(fn, DENY_BOARD_NOAUTOFREE "\n");
#ifdef ZIXIA
	GetDenyPic(fn, DENYPIC, ndenypic, dpcount);
#endif
        if (my_flag == 0) {
            fprintf(fn, "                            %s" NAME_SYSOP_GROUP DENY_NAME_SYSOP "：\x1b[4m%s\x1b[m\n", NAME_BBS_CHINESE, saveptr->userid);
        } else {
            fprintf(fn, "                              " NAME_BM ":\x1b[4m%s\x1b[m\n", getCurrentUser()->userid);
        }
        fprintf(fn, "                              %s\n", ctime(&now));
        fclose(fn);
        post_file(getCurrentUser(), "", filename, currboard->filename, buffer, 0, 2, getSession());
        /*
         * unlink(filename); 
         */
        getCurrentUser() = saveptr;

        sprintf(buffer, "%s 被 %s 封禁本版POST权", uident, getCurrentUser()->userid);
        getuser(uident, &lookupuser);

        if (PERM_BOARDS & lookupuser->userlevel)
            sprintf(buffer, "%s 封某版" NAME_BM " %s 在 %s", getCurrentUser()->userid, uident, currboard->filename);
        else
            sprintf(buffer, "%s 封 %s 在 %s", getCurrentUser()->userid, uident, currboard->filename);
        post_file(getCurrentUser(), "", filename, "denypost", buffer, 0, 8, getSession());
        unlink(filename);
        bmlog(getCurrentUser()->userid, currboard->filename, 10, 1);
    }
    return 0;
}


int deny_user(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{                               /* 禁止POST用户名单 维护主函数 */
    char uident[STRLEN];
    int page = 0;
    char ans[10];
    int count;

    /*
     * Haohmaru.99.4.1.auto notify 
     */
    time_t now;
    int id;
    FILE *fp;
    int find;                   /*Haohmaru.99.12.09 */

/*   static page=0; *//*
 * * Haohmaru.12.18 
 */
    now = time(0);
    if (!HAS_PERM(getCurrentUser(), PERM_SYSOP))
        if (!chk_currBM(currBM, getCurrentUser())) {
            return DONOTHING;
        }

    while (1) {
        char querybuf[0xff];
        char LtNing[24];
        if (fileinfo == NULL) LtNing[0] = '\0';
        else sprintf(LtNing, "(O)增加%s ", fileinfo->owner);

      Here:
        clear();
        count = listdeny(0);
        if (count > 0 && count < 20)    /*Haohmaru.12.18,看下一屏 */
            snprintf(querybuf, 0xff, "%s(A)增加 (D)删除 or (E)离开 [E]: ", LtNing);
        else if (count >= 20)
            snprintf(querybuf, 0xff, "%s(A)增加 (D)删除 (N)后面第N屏 or (E)离开 [E]: ", LtNing);
        else
            snprintf(querybuf, 0xff, "%s(A)增加 or (E)离开 [E]: ", LtNing);

        getdata(1, 0, querybuf, ans, 7, DOECHO, NULL, true);
        *ans = (char) toupper((int) *ans);

        if (*ans == 'A' || (*ans == 'O' && fileinfo != NULL)) {
            struct userec *denyuser;

            move(1, 0);
            if (*ans == 'A')
                usercomplete("增加无法 POST 的使用者: ", uident);
            else
                strncpy(uident, fileinfo->owner, STRLEN - 4);
            /*
             * Haohmaru.99.4.1,增加被封ID正确性检查 
             */
            if (!(id = getuser(uident, &denyuser))) {   /* change getuser -> searchuser , by dong, 1999.10.26 */
                move(3, 0);
                prints("非法 ID");
                clrtoeol();
                pressreturn();
                goto Here;
            }
            strncpy(uident, denyuser->userid, IDLEN);
            uident[IDLEN] = 0;
            
            /*
             * windinsn.04.5.17,不准封禁 guest 和 SYSOP
             */
            if (!strcasecmp(uident,"guest") 
                || !strcasecmp(uident,"SYSOP")
#ifdef ZIXIA
                || !strcasecmp(uident,"SuperMM")
#endif
            ) {
                move(3, 0);
                prints("不能封禁 %s", uident);
                clrtoeol();
                pressreturn();
                goto Here;
            }
            
            if (*uident != '\0') {
                addtodeny(uident);
            }
        } else if ((*ans == 'D') && count) {
            int len;

            move(1, 0);
            sprintf(genbuf, "删除无法 POST 的使用者: ");
            getdata(1, 0, genbuf, uident, 13, DOECHO, NULL, true);
            find = 0;           /*Haohmaru.99.12.09.原来的代码如果被封者已自杀就删不掉了 */
            setbfile(genbuf, currboard->filename, "deny_users");
            if ((fp = fopen(genbuf, "r")) == NULL) {
                prints("(none)\n");
                return 0;
            }
            len = strlen(uident);
            while (fgets(genbuf, 256 /*STRLEN*/, fp) != NULL) {
                if (!strncasecmp(genbuf, uident, len)) {
                    if (genbuf[len] == 32) {
                        strncpy(uident, genbuf, len);
                        uident[len] = 0;
                        find = 1;
                        break;
                    }
                }
            }
            fclose(fp);
            if (!find) {
                move(3, 0);
                prints("该ID不在封禁名单内!");
                clrtoeol();
                pressreturn();
                goto Here;
            }
            /*---	add to check if undeny time reached.	by period 2000-09-11	---*/
            {
                char *lptr;
                time_t ldenytime;

                /*
                 * now the corresponding line in genbuf 
                 */
                if (NULL != (lptr = strrchr(genbuf, '[')))
                    sscanf(lptr + 1, "%lu", &ldenytime);
                else
                    ldenytime = now + 1;
                if (ldenytime > now) {
                    move(3, 0);
                    prints(genbuf);
                    if (false == askyn("该用户封禁时限未到，确认要解封？", false /*, false */ ))
                        goto Here;      /* I hate Goto!!! */
                }
            }
            /*---		---*/
            move(1, 0);
            clrtoeol();
            if (uident[0] != '\0') {
                if (deldeny(getCurrentUser(), currboard->filename, uident, 0, getSession())) {
                }
            }
        } else if (count > 20 && isdigit(ans[0])) {
            if (ans[1] == 0)
                page = *ans - '0';
            else
                page = atoi(ans);
            if (page < 0)
                break;          /*不会封人超过10屏吧?那可是200人啊!  会的！ */
            listdeny(page);
            pressanykey();
        } else
            break;
    }                           /*end of while */
    clear();
    return FULLUPDATE;
}

/* etnlegend, 2005.12.26, 俱乐部授权管理接口结构更新 */
static int set_user_club_perm(struct userec *user,const struct boardheader *board,int write_perm){
    if(!user||!board||!(board->flag&(BOARD_CLUB_READ|BOARD_CLUB_WRITE))
        ||!(board->clubnum>0)||(board->clubnum>MAXCLUB))
        return -1;
    if(!write_perm)
        user->club_read_rights[(board->clubnum-1)>>5]|=(1<<((board->clubnum-1)&0x1F));
    else
        user->club_write_rights[(board->clubnum-1)>>5]|=(1<<((board->clubnum-1)&0x1F));
    return 0;
}
static int del_user_club_perm(struct userec *user,const struct boardheader *board,int write_perm){
    if(!user||!board||!(board->flag&(BOARD_CLUB_READ|BOARD_CLUB_WRITE))
        ||!(board->clubnum>0)||(board->clubnum>MAXCLUB))
        return -1;
    if(!write_perm)
        user->club_read_rights[(board->clubnum-1)>>5]&=~(1<<((board->clubnum-1)&0x1F));
    else
        user->club_write_rights[(board->clubnum-1)>>5]&=~(1<<((board->clubnum-1)&0x1F));
    return 0;
}
static int get_user_club_perm(const struct userec *user,const struct boardheader *board,int write_perm){
    if(!user||!board||!(board->flag&(BOARD_CLUB_READ|BOARD_CLUB_WRITE))
        ||!(board->clubnum>0)||(board->clubnum>MAXCLUB))
        return 0;
    if(!write_perm)
        return (user->club_read_rights[(board->clubnum-1)>>5]&(1<<((board->clubnum-1)&0x1F)));
    else
        return (user->club_write_rights[(board->clubnum-1)>>5]&(1<<((board->clubnum-1)&0x1F)));
}
static int func_query_club_users(struct userec *user,void *varg){
    if(user->userid[0]&&get_user_club_perm(user,currboard,*(int*)varg))
        AddNameList(user->userid);
    return 0;
}
static int func_remove_club_users(struct userec *user,void *varg){
    if(user->userid[0]&&get_user_club_perm(user,currboard,*(int*)varg)&&!del_user_club_perm(user,currboard,*(int*)varg)){
        if(!(*(int*)varg)&&user==getCurrentUser()&&!check_read_perm(user,currboard))
            set_user_club_perm(user,currboard,*(int*)varg);
        else
            AddNameList(user->userid);
    }
    return 0;
}
static int func_count(char *userid,void *varg){
    (*(int*)varg)++;
    return 0;
}
static int func_dump_users(char *userid,void *varg){
    ((char**)(((void**)varg)[0]))[(*(int*)(((void**)varg)[1]))]=userid;
    (*(int*)(((void**)varg)[1]))++;
    return 0;
}
static int func_send_mail(char *userid,void *varg){
    sprintf(genbuf,"%s 被 %s 取消 %s 俱乐部%s权限",userid,getCurrentUser()->userid,currboard->filename,
        (!(*(int*)(((void**)varg)[1]))?"读取":"发表"));
    mail_buf(getCurrentUser(),(char*)(((void**)varg)[0]),userid,genbuf,getSession());
    deliverreport(genbuf,(char*)(((void**)varg)[0]));
    return 0;
}
static void trimstr(char *s){
    char *p;
    if(s){
        for(p=s+strlen(s)-1;*p==32;p--)
            continue;
        *(p+1)=0;
        for(p=s;*p==32;p++)
            continue;
        memmove(s,p,(strlen(p)+1)*sizeof(char));
    }
    return;
}
typedef int (*APPLY_USERS_FUNC) (int(*)(struct userec*,void*),void*);
int clubmember(struct _select_def *conf,struct fileheader *fh,void *varg){
    static const char *title="\033[1;32m[设定俱乐部授权用户]\033[m";
    static const char *echo="\033[1;37m查阅[\033[1;36mV\033[1;37m]/增加[\033[1;36mA\033[1;37m]"
        "/删除[\033[1;36mD\033[1;37m]/批量[\033[1;36mI\033[1;37m]/清理[\033[1;36mC\033[1;37m]"
        "/群信[\033[1;36mM\033[1;37m]/退出[E] [E]: \033[m";
    static const char *choice="\033[1;37m操作读取[R]/发表[\033[1;36mP\033[1;37m]权限列表 [R]: \033[m";
    static char comment[128]="";
    APPLY_USERS_FUNC func=(APPLY_USERS_FUNC)apply_users;
    FILE *fp;
    struct userec *user;
    char buf[256],line[256],fn[256],userid[16],ans[4],**p_users;
    int i,j,k,write_perm;
    void *arg[2];
    if(!chk_currBM(currBM,getCurrentUser()))
        return DONOTHING;
    if(!(currboard->flag&(BOARD_CLUB_READ|BOARD_CLUB_WRITE))||!(currboard->clubnum>0)||(currboard->clubnum>MAXCLUB))
        return DONOTHING;
    clear();
    move(0,0);
    prints("%s",title);
    if((currboard->flag&BOARD_CLUB_READ)&&(currboard->flag&BOARD_CLUB_WRITE)){
        getdata(1,0,(char*)choice,ans,2,DOECHO,NULL,true);
        ans[0]=toupper(ans[0]);
        write_perm=(ans[0]=='P');
        move(0,0);
        clrtoeol();
        prints("%s \033[1;36m<%s>\033[m",title,(!write_perm?"读取":"发表"));
    }
    else
        write_perm=(currboard->flag&BOARD_CLUB_WRITE);
    while(1){
        move(1,0);
        clrtobot();
        getdata(1,0,(char*)echo,ans,2,DOECHO,NULL,true);
        ans[0]=toupper(ans[0]);
        if(ans[0]=='V'){
            CreateNameList();
            func(func_query_club_users,&write_perm);
            i=0;
            ApplyToNameList(func_count,&i);
            move(1,0);
            clrtoeol();
            if(!i){
                prints("\033[1;33m%s\033[0;33m<Enter>\033[m","尚无授权用户...");
                WAIT_RETURN;
                continue;
            }
            namecomplete("按 \033[1;33m<Space>\033[m 列示, 可输入前缀查询: ",buf);
        }
        else if(ans[0]=='A'){
            move(1,0);
            clrtoeol();
            usercomplete("增加俱乐部授权用户: ",buf);
            move(1,0);
            clrtobot();
            snprintf(userid,16,"%s",buf);
            if(!userid[0]||!getuser(userid,&user)){
                prints("\033[1;33m%s\033[0;33m<Enter>\033[m","错误的用户名...");
                WAIT_RETURN;
                continue;
            }
            if(!strcmp(user->userid,"guest")){
                prints("\033[1;33m%s\033[0;33m<Enter>\033[m","不允许对 guest 进行授权...");
                WAIT_RETURN;
                continue;
            }
            if(get_user_club_perm(user,currboard,write_perm)){
                prints("\033[1;33m%s\033[0;33m<Enter>\033[m","该用户已经被授权...");
                WAIT_RETURN;
                continue;
            }
            prints("\033[1;37m增加俱乐部授权用户: \033[1;32m%s\033[m",user->userid);
            sprintf(genbuf,"附加说明 [%s]: ",comment);
            getdata(2,0,genbuf,buf,64,DOECHO,NULL,true);
            if(buf[0]){
                trimstr(buf);
                snprintf(comment,128,"%s",buf);
            }
            sprintf(genbuf,"确认授予 %s 本俱乐部%s权限 [y/N]: ",user->userid,(!write_perm?"读取":"发表"));
            getdata(3,0,genbuf,ans,2,DOECHO,NULL,true);
            ans[0]=toupper(ans[0]);
            if(ans[0]!='Y')
                continue;
            move(4,0);
            if(set_user_club_perm(user,currboard,write_perm)){
                prints("\033[1;33m%s\033[0;33m<Enter>\033[m","操作过程中发生错误...");
                WAIT_RETURN;
                continue;
            }
            sprintf(buf,"附加说明: %s",comment);
            sprintf(genbuf,"%s 由 %s 授予 %s 俱乐部%s权限",user->userid,getCurrentUser()->userid,
                currboard->filename,(!write_perm?"读取":"发表"));
            mail_buf(getCurrentUser(),buf,user->userid,genbuf,getSession());
            deliverreport(genbuf,buf);
            prints("\033[1;32m%s\033[0;33m<Enter>\033[m","增加成功!");
            WAIT_RETURN;
        }
        else if(ans[0]=='D'){
            CreateNameList();
            func(func_query_club_users,&write_perm);
            move(1,0);
            clrtoeol();
            namecomplete("删除俱乐部授权用户: ",buf);
            move(1,0);
            clrtobot();
            snprintf(userid,16,"%s",buf);
            if(!userid[0]||!getuser(userid,&user)){
                prints("\033[1;33m%s\033[0;33m<Enter>\033[m","错误的用户名...");
                WAIT_RETURN;
                continue;
            }
            if(!get_user_club_perm(user,currboard,write_perm)){
                prints("\033[1;33m%s\033[0;33m<Enter>\033[m","该用户尚未被授权...");
                WAIT_RETURN;
                continue;
            }
            prints("\033[1;37m删除俱乐部授权用户: \033[1;32m%s\033[m",user->userid);
            sprintf(genbuf,"附加说明 [%s]: ",comment);
            getdata(2,0,genbuf,buf,64,DOECHO,NULL,true);
            if(buf[0]){
                trimstr(buf);
                snprintf(comment,128,"%s",buf);
            }
            sprintf(genbuf,"确认取消 %s 本俱乐部%s权限 [y/N]: ",user->userid,(!write_perm?"读取":"发表"));
            getdata(3,0,genbuf,ans,2,DOECHO,NULL,true);
            ans[0]=toupper(ans[0]);
            if(ans[0]!='Y')
                continue;
            move(4,0);
            if(del_user_club_perm(user,currboard,write_perm)){
                prints("\033[1;33m%s\033[0;33m<Enter>\033[m","操作过程中发生错误...");
                WAIT_RETURN;
                continue;
            }
            sprintf(buf,"附加说明: %s",comment);
            sprintf(genbuf,"%s 被 %s 取消 %s 俱乐部%s权限",user->userid,getCurrentUser()->userid,
                currboard->filename,(!write_perm?"读取":"发表"));
            mail_buf(getCurrentUser(),buf,user->userid,genbuf,getSession());
            deliverreport(genbuf,buf);
            prints("\033[1;32m%s\033[0;33m<Enter>\033[m","删除成功!");
            WAIT_RETURN;
        }
        else if(ans[0]=='I'){
            move(1,0);
            clrtoeol();
            sprintf(genbuf,"附加说明 [%s]: ",comment);
            getdata(1,0,genbuf,buf,64,DOECHO,NULL,true);
            if(buf[0]){
                trimstr(buf);
                snprintf(comment,128,"%s",buf);
            }
            move(3,0);
            prints("%s",
                "    [批量操作俱乐部授权列表信息文件格式]\n\n"
                "    以行为单位, 每行操作一位用户, 附加前缀如下:\n"
                "    # 起始的行为注释行, 无作用;\n"
                "    - 起始的行表示删除其后的用户;\n"
                "    + 起始的行表示增加其后的用户;\n"
                "    无前缀时默认操作为增加...\n\n"
                "按 \033[0;33m<Enter>\033[m 键后开始编辑批量修改列表: ");
            WAIT_RETURN;
            saveline(0,0,NULL);
            j=uinfo.mode;
            modify_user_mode(EDITANN);
            sprintf(fn,"tmp/club_perm_%ld_%d",time(NULL),getpid());
            k=vedit(fn,0,NULL,NULL,0);
            modify_user_mode(j);
            clear();
            saveline(0,1,NULL);
            move(1,0);
            if(k==-1){
                unlink(fn);
                prints("\033[1;33m%s\033[0;33m<Enter>\033[m","取消...");
                WAIT_RETURN;
                continue;
            }
            else{
                sprintf(genbuf,"\033[1;33m确认批量操作俱乐部%s授权列表 [y/N]: \033[m",(!write_perm?"读取":"发表"));
                getdata(1,0,genbuf,ans,2,DOECHO,NULL,true);
                ans[0]=toupper(ans[0]);
                if(ans[0]!='Y')
                    continue;
            }
            if(!(fp=fopen(fn,"r")))
                continue;
            i=0;
            j=0;
            sprintf(buf,"附加说明: %s",comment);
            while(fgets(line,256,fp)){
                k=strlen(line);
                if(line[k-1]==10||line[k-1]==13)
                    line[k-1]=0;
                trimstr(line);
                switch(line[0]){
                    case 0:
                    case 10:
                    case 13:
                    case '#':
                        continue;
                    case '-':
                        trimstr(&line[1]);
                        if(!getuser(&line[1],&user)||!get_user_club_perm(user,currboard,write_perm))
                            continue;
                        if(!del_user_club_perm(user,currboard,write_perm)){
                            sprintf(genbuf,"%s 被 %s 取消 %s 俱乐部%s权限",user->userid,getCurrentUser()->userid,
                                currboard->filename,(!write_perm?"读取":"发表"));
                            mail_buf(getCurrentUser(),buf,user->userid,genbuf,getSession());
                            deliverreport(genbuf,buf);
                            j++;
                        }
                        break;
                    case '+':
                        line[0]=32;
                        trimstr(line);
                    default:
                        if(!getuser(line,&user)||get_user_club_perm(user,currboard,write_perm))
                            continue;
                        if(!set_user_club_perm(user,currboard,write_perm)){
                            sprintf(genbuf,"%s 由 %s 授予 %s 俱乐部%s权限",user->userid,getCurrentUser()->userid,
                                currboard->filename,(!write_perm?"读取":"发表"));
                            mail_buf(getCurrentUser(),buf,user->userid,genbuf,getSession());
                            deliverreport(genbuf,buf);
                            i++;
                        }
                        break;
                }
            }
            fclose(fp);
            unlink(fn);
            move(2,0);
            prints("\033[1;37m增加 \033[1;33m%d\033[1;37m 位用户, 删除 \033[1;33m%d\033[1;37m 位用户..."
                "\033[1;32m%s\033[0;33m<Enter>\033[m",i,j,"操作已完成!");
            WAIT_RETURN;
        }
        else if(ans[0]=='M'){
            /* 注: 俱乐部群信部分原作者为 asing@zixia */
            if(HAS_PERM(getCurrentUser(),PERM_DENYMAIL)||!HAS_PERM(getCurrentUser(),PERM_LOGINOK))
                continue;
            CreateNameList();
            func(func_query_club_users,&write_perm);
            i=0;
            ApplyToNameList(func_count,&i);
            move(1,0);
            clrtoeol();
            if(!i){
                prints("\033[1;33m%s\033[0;33m<Enter>\033[m","尚无授权用户...");
                WAIT_RETURN;
                continue;
            }
            if(!(p_users=(char**)malloc(i*sizeof(char*))))
                continue;
            i=0;
            arg[0]=p_users;
            arg[1]=&i;
            ApplyToNameList(func_dump_users,arg);
            getdata(1,0,"设定群信标题: ",buf,40,DOECHO,NULL,true);
            sprintf(genbuf,"[俱乐部 %s 群信] %s",currboard->filename,buf);
            snprintf(buf,ARTICLE_TITLE_LEN,"%s",genbuf);
            saveline(0,0,NULL);
            j=do_gsend(p_users,buf,i);
            free(p_users);
            clear();
            saveline(0,1,NULL);
            move(1,0);
            if(j){
                prints("\033[1;33m%s\033[0;33m<Enter>\033[m","发送群信过程中发生错误...");
                WAIT_RETURN;
                continue;
            }
            prints("\033[1;32m%s\033[0;33m<Enter>\033[m","群信已发送!");
            WAIT_RETURN;
        }
        else if(ans[0]=='C'){
            move(1,0);
            clrtoeol();
            if(!HAS_PERM(getCurrentUser(),(PERM_OBOARDS|PERM_SYSOP))){
                prints("\033[1;33m\033[0;33m<Enter>\033[m","当前用户不具有清理俱乐部授权列表的权限...");
                WAIT_RETURN;
                continue;
            }
            sprintf(genbuf,"附加说明 [%s]: ",comment);
            getdata(1,0,genbuf,buf,64,DOECHO,NULL,true);
            if(buf[0]){
                trimstr(buf);
                snprintf(comment,128,"%s",buf);
            }
            sprintf(genbuf,"\033[1;31m确认清理本俱乐部%s授权列表 [y/N]: \033[m",(!write_perm?"读取":"发表"));
            getdata(2,0,genbuf,ans,2,DOECHO,NULL,true);
            ans[0]=toupper(ans[0]);
            if(ans[0]!='Y')
                continue;
            CreateNameList();
            func(func_remove_club_users,&write_perm);
            sprintf(buf,"附加说明: %s",comment);
            arg[0]=buf;
            arg[1]=&write_perm;
            ApplyToNameList(func_send_mail,&arg);
            move(3,0);
            prints("\033[1;32m%s\033[0;33m<Enter>\033[m","已完成清理!");
            WAIT_RETURN;
        }
        else
            break;
    }
    clear();
    return FULLUPDATE;
}

