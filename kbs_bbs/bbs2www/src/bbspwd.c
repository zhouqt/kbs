#include "bbslib.h"

int main()
{
	int type;
  	char pw1[40], pw2[40], pw3[40];
	int i;

	init_all();
	if(!loginok)
		http_fatal("您尚未登录, 请先登录");
	type = atoi(getparm("type"));
	if(type == 0)
	{
		printf("%s -- 修改密码 [用户: %s]<hr color=\"green\">\n", 
			BBSNAME, currentuser->userid);
		printf("<form action=\"bbspwd?type=1\" method=\"post\">\n");
		printf("你的旧密码: <input maxlength=\"39\" size=\"12\" type=\"password\" name=\"pw1\"><br>\n");
		printf("你的新密码: <input maxlength=\"39\" size=\"12\" type=\"password\" name=\"pw2\"><br>\n");
		printf("再输入一次: <input maxlength=\"39\" size=\"12\" type=\"password\" name=\"pw3\"><br><br>\n");
		printf("<input type=\"submit\" value=\"确定修改\">\n");
		http_quit();
	}
  	strsncpy(pw1, getparm("pw1"), sizeof(pw1));
  	strsncpy(pw2, getparm("pw2"), sizeof(pw2));
  	strsncpy(pw3, getparm("pw3"), sizeof(pw3));
  	if(strcmp(pw2, pw3))
		http_fatal("两次密码不相同");
  	if(strlen(pw2)<2)
		http_fatal("新密码太短");
  	if(!checkpasswd2(pw1, getcurrusr()))
		http_fatal("密码不正确");
  	/*strcpy(currentuser.passwd, crypt1(pw2, pw2));*/
	setpasswd(pw2, currentuser);
  	printf("[%s] 密码修改成功.", currentuser->userid);
}

