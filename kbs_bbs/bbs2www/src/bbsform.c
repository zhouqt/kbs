#include "bbslib.h"

int main()
{
  	FILE *fp;
	int type;

	init_all();
	type = atoi(getparm("type"));
  	if(!loginok)
		http_fatal("您尚未登录, 请重新登录。");
	printf("%s -- 填写注册单<hr color=\"green\">\n", BBSNAME);
	check_if_ok();
	if(type==1)
	{
		check_submit_form();
		http_quit();
	}
  	printf("您好, %s, 注册单通过后即可获得注册用户的权限, 下面各项务必请认真填写<br><hr>\n", currentuser->userid);
  	printf("<form method=\"post\" action=\"bbsform?type=1\">\n");
  	printf("真实姓名: <input name=\"realname\" type=\"text\" maxlength=\"8\" size=\"8\" value=\"%s\">(请用中文,不能输入的汉字请用拼音)<br>\n", 
		nohtml(currentuser->realname));
  	printf("服务单位: <input name=\"career\" type=\"text\" maxlength=\"32\" size=\"32\">(学校系级或单位全称)<br>\n");
  	printf("目前住址: <input name=\"address\" type=\"text\" maxlength=\"32\" size=\"32\" value=\"%s\">(请具体到寝室或门牌号码)<br>\n", 
		nohtml(currentuser->address));
  	printf("联络电话: <input name=\"phone\" type=\"text\" maxlength=\"32\" size=\"32\">(包括可连络时间,若无可用呼机或Email地址代替)<br>\n");
  	printf("出生年月: <input name=\"birth\" type=\"text\" maxlength=\"32\" size=\"32\">(年.月.日(公元))<br><hr><br>\n");
  	printf("<input type=\"submit\"> <input type=\"reset\">");
	http_quit();
}

int check_if_ok()
{
  	if(user_perm(currentuser, PERM_LOGINOK))
		http_fatal("您的身份确认已经成功, 欢迎加入本站的行列.");
    if ((time(0)-currentuser->firstlogin) < REGISTER_WAIT_TIME)
	{
		hprintf("您首次登入本站未满"REGISTER_WAIT_TIME_NAME"...");
		hprintf( "请先四处熟悉一下，在满"REGISTER_WAIT_TIME_NAME"以后再填写注册单。");
		exit(-1);
	}

  	if(has_fill_form())
		http_fatal("目前站长尚未处理您的注册单，请耐心等待.");
}

int check_submit_form()
{
	FILE *fp;

  	fp=fopen("new_register", "a");
	if(fp==0)
		http_fatal("注册文件错误，请通知SYSOP");
  	fprintf(fp, "usernum: %d, %s\n",
			getusernum(currentuser->userid), wwwCTime(time(0)));
  	fprintf(fp, "userid: %s\n", currentuser->userid);
	/* 下面的各个信息应过滤掉非法字符之后再写入文件.*/
  	fprintf(fp, "realname: %s\n", getparm("realname"));
	fprintf( fp, "career: %s\n", getparm("career") );
  	fprintf(fp, "addr: %s\n", getparm("address"));
  	fprintf(fp, "phone: %s\n", getparm("phone"));
	fprintf(fp, "birth: %s\n", getparm("birth"));
  	fprintf(fp, "----\n" );
  	fclose(fp);
  	printf("您的注册单已成功提交. 站长检验过后会给您发信, 请留意您的信箱.");
}
