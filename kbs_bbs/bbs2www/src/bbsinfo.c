#include "bbslib.h"

int main()
{
	int n, type;
	struct stat st;
	int num;
	char buf[STRLEN];

  	init_all();
	if(!loginok)
		http_fatal("您尚未登录");
	type = atoi(getparm("type"));
	printf("%s -- 用户个人资料<hr color=\"green\">\n", BBSNAME);
	if(type!=0)
	{
		check_info();
		http_quit();
	}
    setmailfile(buf, currentuser->userid, DOT_DIR);
	if( stat( buf, &st ) >= 0 )
		num = st.st_size / (sizeof( struct fileheader ));
	else
		num = 0;
 	printf("<form action=\"bbsinfo?type=1\" method=\"post\">");
  	printf("您的帐号: %s<br>\n", currentuser->userid);
  	printf("您的昵称: <input type=\"text\" name=\"nick\" value=\"%s\" size=\"24\" maxlength=\"%d\"><br>\n",
		currentuser->username, NAMELEN-1);
  	printf("发表大作: %d 篇<br>\n", currentuser->numposts);
  	printf("信件数量: %d 封<br>\n", num);
  	printf("上站次数: %d 次<br>\n", currentuser->numlogins);
  	printf("上站时间: %d 分钟<br>\n", currentuser->stay/60);
  	printf("真实姓名: <input type=\"text\" name=\"realname\" value=\"%s\" size=\"16\" maxlength=\"%d\"><br>\n",
	 	currentuser->realname, NAMELEN-1);
  	printf("居住地址: <input type=\"text\" name=\"address\" value=\"%s\" size=\"40\" maxlength=\"%d\"><br>\n",
 		currentuser->address, STRLEN-1);
  	printf("帐号建立: %s<br>", wwwCTime(currentuser->firstlogin));
  	printf("最近光临: %s<br>", wwwCTime(currentuser->lastlogin));
  	printf("来源地址: %s<br>", currentuser->lasthost);
  	printf("电子邮件: <input type=\"text\" name=\"email\" value=\"%s\" size=\"32\" maxlength=\"%d\"><br>\n", 
		currentuser->email, STRLEN-1);
  	printf("<input type=\"submit\" value=\"确定\"> <input type=\"reset\" value=\"复原\">\n");
  	printf("</form>");
  	printf("<hr>");
	http_quit();
}

int check_info()
{
  	int m, n;
  	char buf[256];

	/* 必须对所有的变量滤掉ANSI控制符 */
	strsncpy(buf, getparm("nick"), NAMELEN);
	for(m=0; m<strlen(buf); m++)
	{
		if(buf[m]<32 && buf[m]>0 || buf[m]==-1)
			buf[m]=' ';
	}
	if(strlen(buf)>1)
		strcpy(currentuser->username, buf);
	else
		printf("警告: 昵称太短!<br>\n");
	strsncpy(buf, getparm("realname"), NAMELEN);
	if(strlen(buf)>1)
		strcpy(currentuser->realname, buf); 
	else
		printf("警告: 真实姓名太短!<br>\n");
	strsncpy(buf, getparm("address"), STRLEN);
	if(strlen(buf)>8)
		strcpy(currentuser->address, buf);
	else
		printf("警告: 居住地址太短!<br>\n");
	strsncpy(buf, getparm("email"), STRLEN);
   	if(strlen(buf)>8 && strchr(buf, '@'))
		strcpy(currentuser->email, buf);
	else
		printf("警告: email地址不合法!<br>\n");
	printf("[%s] 个人资料修改成功.", currentuser->userid);
}
