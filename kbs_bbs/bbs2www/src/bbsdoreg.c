/*
 * $Id$
 */
#include "bbslib.h"

int is_bad_id(char *s)
{
    FILE *fp;
    char buf[256], buf2[256];

    fp = fopen(".badname", "r");
    if (fp == 0)
        return 0;
    while (1) {
        if (fgets(buf, 250, fp) == 0)
            break;
        if (sscanf(buf, "%s", buf2) != 1)
            continue;
        if (strcasestr(s, buf2)) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

int badymd(int y, int m, int d)
{
    int max[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    if ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)
        max[2] = 29;
    if (y < 10 || y > 100 || m < 1 || m > 12)
        return 1;
    if (d < 0 || d > max[m])
        return 1;
    return 0;
}

int main()
{
    FILE *fp;
    struct userec x;
    int i, gender, xz;
    char buf[256], filename[80], pass1[80], pass2[80], dept[80], phone[80], salt[80], words[1024];
    int allocid;

    init_all();
    bzero(&x, sizeof(x));
    xz = atoi(getparm("xz"));
    strsncpy(x.userid, getparm("userid"), 13);
    strsncpy(pass1, getparm("pass1"), 13);
    strsncpy(pass2, getparm("pass2"), 13);
    strsncpy(x.username, getparm("username"), 32);
    strsncpy(x.realname, getparm("realname"), 32);
    strsncpy(dept, getparm("dept"), 32);
    strsncpy(x.address, getparm("address"), 32);
    strsncpy(x.email, getparm("email"), 32);
    strsncpy(phone, getparm("phone"), 32);
    strsncpy(words, getparm("words"), 1000);
    if (id_invalid(x.userid))
        http_fatal("帐号必须由英文字母或数字组成，并且第一个字符必须是英文字母!");
    if (strlen(x.userid) < 2)
        http_fatal("帐号长度太短(2-12字符)");
    if (strlen(pass1) < 4 || !strcmp(pass1, x.userid))
        http_fatal("密码太短或与使用者代号相同, 请重新输入");
    if (strcmp(pass1, pass2))
        http_fatal("两次输入的密码不一致, 请确认密码");
    setpasswd(pass1, &x);
    if (strlen(x.username) < 2)
        http_fatal("请输入昵称(昵称长度至少2个字符)");
    if (strlen(x.realname) < 4)
        http_fatal("请输入真实姓名(请用中文, 至少2个字)");
    if (strlen(dept) < 6)
        http_fatal("工作单位的名称长度至少要6个字符(或3个汉字)");
    if (strlen(x.address) < 6)
        http_fatal("通讯地址长度至少要6个字符(或3个汉字)");
    if (badstr(x.passwd) || badstr(x.username) || badstr(x.realname))
        http_fatal("您的注册单中含有非法字符");
    if (badstr(dept) || badstr(x.address) || badstr(x.email) || badstr(phone))
        http_fatal("您的注册单中含有非法字符");
    if (bad_user_id(x.userid))
        http_fatal("不雅帐号或禁止注册的id, 请重新选择");
    if (searchuser(x.userid))
        http_fatal("此帐号已经有人使用,请重新选择。");
    strcpy(x.lasthost, fromhost);
    x.userlevel = PERM_BASIC;
    x.firstlogin = x.lastlogin = time(0);
    x.userdefine = -1;
    x.userdefine &= ~DEF_NOTMSGFRIEND;
    x.notemode = -1;
    x.unuse1 = -1;
    x.unuse2 = -1;
    x.flags = CURSOR_FLAG | PAGER_FLAG;
    x.title = 0;
    allocid = getnewuserid2(x.userid);
    if (allocid > MAXUSERS || allocid <= 0)
        http_fatal("抱歉, 由于某些系统原因, 无法注册新的帐号.");
    newbbslog(BBSLOG_USIES, "APPLY: uid %d from %s", allocid, fromhost);
    update_user(&x, allocid, 1);
    if (!searchuser(x.userid))
        http_fatal("创建用户失败");
    report("new account");
    sprintf(filename, "home/%c/%s", toupper(x.userid[0]), x.userid);
    sprintf(buf, "/bin/mv -f %s " BBSHOME "/homeback/%s", filename, x.userid);
    system(buf);
    sprintf(filename, "mail/%c/%s", toupper(x.userid[0]), x.userid);
    sprintf(buf, "/bin/mv -f %s " BBSHOME "/mailback/%s", filename, x.userid);
    system(buf);

    sprintf(filename, "home/%c/%s", toupper(x.userid[0]), x.userid);
    mkdir(filename, 0755);
#ifndef SMTH
    printf("<center><table><td><td><pre>\n");
    printf("亲爱的新使用者，您好！\n\n");
    printf("欢迎光临 本站, 您的新帐号已经成功被登记了。\n");
    printf("您目前拥有本站基本的权限, 包括阅读文章、环顾四方、接收私人\n");
    printf("信件、接收他人的消息、进入聊天室等等。当您通过本站的身份确\n");
    printf("认手续之后，您还会获得更多的权限。目前您的注册单已经被提交\n");
    printf("等待审阅。一般情况24小时以内就会有答复，请耐心等待。同时请\n");
    printf("留意您的站内信箱。\n");
    printf("如果您有任何疑问，可以去sysop(站长的工作室)版发文求助。\n\n</pre></table>");
    printf("<hr color=green><br>您的基本资料如下:<br>\n");
    printf("<table border=1 width=400>");
    printf("<tr><td>帐号位置: <td>%d\n", getusernum(x.userid));
    printf("<tr><td>使用者代号: <td>%s (%s)\n", x.userid, x.username);
    printf("<tr><td>姓  名: <td>%s<br>\n", x.realname);
    printf("<tr><td>昵  称: <td>%s<br>\n", x.username);
    printf("<tr><td>上站位置: <td>%s<br>\n", x.lasthost);
    printf("<tr><td>电子邮件: <td>%s<br></table><br>\n", x.email);
    newcomer(&x, words);
#endif                          /* not SMTH */
    printf("<center>申请 ID 成功！</center><br>");
    printf("<center><input type=\"button\" onclick=\"window.close()\" value=\"关闭本窗口\"></center>\n");
    sprintf(buf, "%s %-12s %d\n", wwwCTime(time(0)) + 4, x.userid, getusernum(x.userid));
    f_append("wwwreg.log", buf);
    http_quit();
}

int badstr(unsigned char *s)
{
    int i;

    for (i = 0; s[i]; i++)
        if (s[i] != 9 && (s[i] < 32 || s[i] == 255))
            return 1;
    return 0;
}

#ifndef SMTH
int newcomer(struct userec *x, char *words)
{
    FILE *fp;
    char filename[80];

    sprintf(filename, "tmp/%d.tmp", getpid());
    fp = fopen(filename, "w");
    fprintf(fp, "大家好, \n\n");
    fprintf(fp, "我是 %s(%s), 来自 %s\n", x->userid, x->username, fromhost);
    fprintf(fp, "今天初来此地报到, 请大家多多指教.\n\n");
    fprintf(fp, "自我介绍:\n\n");
    fprintf(fp, "%s", words);
    fclose(fp);
    unlink(filename);
}

int adduser(struct userec *x)
{
    int i;
    FILE *fp;

    fp = fopen(".PASSWDS", "r+");
    flock(fileno(fp), LOCK_EX);
    for (i = 0; i < MAXUSERS; i++) {
        if (shm_ucache->userid[i][0] == 0) {
            strncpy(shm_ucache->userid[i], x->userid, 13);
            save_user_data(x);
            break;
        }
    }
    flock(fileno(fp), LOCK_UN);
    fclose(fp);
    system("touch .PASSFLUSH");
}
#endif                          /* not SMTH */
