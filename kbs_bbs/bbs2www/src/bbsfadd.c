#include "bbslib.h"

int addtooverride2(char *uident, char *exp)
{
    friends_t tmp;
    int  n;
    char buf[STRLEN];

    memset(&tmp, 0, sizeof(tmp));
    sethomefile(buf,currentuser->userid, "friends" );
    if((!HAS_PERM(currentuser,PERM_ACCOUNTS) && !HAS_PERM(currentuser,PERM_SYSOP))
		   	&& (get_num_records(buf, sizeof(struct friends)) >= MAXFRIENDS) )
    {
        hprintf("抱歉，本站目前仅可以设定 %d 个好友.", MAXFRIENDS);
        return -1;
    }
    if( myfriend( searchuser(uident) , NULL) )
        return -1;
	strsncpy(tmp.id, uident, sizeof(tmp.id));
	strsncpy(tmp.exp, exp, sizeof(tmp.exp));
    n=append_record(buf, &tmp, sizeof(friends_t));
    if(n != -1)
        getfriendstr();
    else
        report("append friendfile error");
    return n;
}

int main()
{
   	FILE *fp;
	char path[80], userid[80], exp[80];
	struct userec *x = NULL;
	int rv;

	init_all();
   	if(!loginok)
	   	http_fatal("您尚未登录，请先登录");
	sethomefile(path, currentuser->userid,"friends");
   	printf("<center>%s -- 好友名单 [使用者: %s]<hr color=\"green\">\n", BBSNAME, currentuser->userid);
	strsncpy(userid, getparm("userid"), 13);
	strsncpy(exp, getparm("exp"), 32);
	if(userid[0]==0 || exp[0]==0)
   	{
		if(userid[0])
		   	printf("<font color=\"red\">请输入好友说明</font>");
		printf("<form action=\"bbsfadd\">\n");
		printf("请输入欲加入的好友帐号: <input type=\"text\" name=\"userid\" value=\"%s\"><br>\n",
			userid);
		printf("请输入对这个好友的说明: <input type=\"text\" name=\"exp\">\n", 
			exp);
		printf("<br><input type=\"submit\" value=\"确定\"></form>\n");
		http_quit();
	}
	getuser(userid, &x);
	if(x == NULL)
	   	http_fatal("错误的使用者帐号");
	rv = addtooverride2(x->userid, exp);
	if (rv == -1)
		http_fatal("您的好友名单已达到上限, 不能添加新的好友");
	else if (rv == -2)
		http_fatal("此人已经在你的好友名单里了");
	else if (rv == -3)
		http_fatal("添加至好友名单失败");

   	printf("[%s]已加入您的好友名单.<br>\n <a href=bbsfall>返回好友名单</a>",
			userid);
	http_quit();
}
