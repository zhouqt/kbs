#include "bbslib.h"

int cmpfnames2(char *userid, struct friends *uv)
{
	return !strcasecmp(userid, uv->id);
}

int deleteoverride2(char *uident)
{
    int deleted;
    struct friends fh;
	char buf[STRLEN];

    sethomefile( buf, currentuser->userid,"friends" );
    deleted = search_record( buf, &fh, sizeof(fh), cmpfnames2, uident );
    if(deleted > 0)
    {
        if(delete_record(buf, sizeof(fh), deleted) != -1)
            getfriendstr();
        else
        {
            deleted=-1;
            report("delete friend error");
        }
    }
    return (deleted>0)?1:-1;
}

int main()
{
   	FILE *fp;
   	int i, total=0;
	char userid[80];

	init_all();
   	if(!loginok)
	   	http_fatal("您尚未登录，请先登录");
	getfriendstr();
   	printf("<center>%s -- 好友名单 [使用者: %s]<hr color=\"green\">\n",
		   	BBSNAME, currentuser->userid);
	strsncpy(userid, getparm("userid"), 13);
	if(userid[0] == 0)
   	{
		printf("<form action=\"bbsfdel\">\n");
		printf("请输入欲删除的好友帐号: <input type=\"text\"><br>\n");
		printf("<input type=\"submit\">\n");
		printf("</form>");
		http_quit();
	}
	if(get_friends_num() <= 0)
	   	http_fatal("您没有设定任何好友");
   	if(!isfriend(userid))
	   	http_fatal("此人本来就不在你的好友名单里");
	if (deleteoverride2(userid) == -1)
	   	http_fatal("从好友名单中删除该用户失败");
   	printf("[%s]已从您的好友名单中删除.<br>\n <a href=\"bbsfall\">返回好友名单</a>", userid);
	http_quit();
}
