/*
 * $Id$
 */
#include "bbslib.h"

int main()
{
    FILE *fp;
    char userid[80], filename[80], dir[80], title[ARTICLE_TITLE_LEN], buf[80], *content;
    int t, i, sig, backup;
    struct fileheader x;
    struct userec *u = NULL;

    init_all();
    if (!loginok)
        http_fatal("匆匆过客不能写信，请先登录");
    if (!can_send_mail())
        http_fatal("您不能发送信件");
    strsncpy(userid, getparm("userid"), 40);
    strncpy(title, getparm("title"), ARTICLE_TITLE_LEN - 1);
	title[ARTICLE_TITLE_LEN - 1] = '\0';
    backup = strlen(getparm("backup"));
    if (strchr(userid, '@') || strchr(userid, '|')
        || strchr(userid, '&') || strchr(userid, ';')) {
        http_fatal("错误的收信人帐号");
    }
    getuser(userid, &u);
    if (u == 0)
        http_fatal("错误的收信人帐号");
    strcpy(userid, u->userid);
    for (i = 0; i < strlen(title); i++)
        if (title[i] < 27 && title[i] >= -1)
            title[i] = ' ';
    sig = atoi(getparm("signature"));
    content = getparm("text");
    if (title[0] == 0)
        strcpy(title, "没主题");
    sprintf(filename, "tmp/%s.%d.tmp", userid, getpid());
    if (f_append(filename, unix_string(content)) < 0)
        http_fatal("发信失败");
    
    if ((i=post_mail(userid, title, filename, currentuser->userid, currentuser->username, fromhost, sig, backup))!=0)
    {
        switch (i) {
        case -1:
        	http_fatal("发信失败:无法创建文件");
        case -2:
        	http_fatal("发信失败:对方拒收你的邮件");
        case -3:
        	http_fatal("发信失败:对方信箱满");
        case -5:
        	http_fatal("发信失败:不能添加到.DIR");
        case -6:
            printf("发信警告:不能保存到发件箱<br>\n");
            break;
        default:
        	http_fatal("发信失败");
        }
    }
    printf("信件已寄给%s.<br>\n", userid);
    unlink(filename);
    printf("<a href=\"javascript:history.go(-2)\">返回</a>");
    http_quit();
}
