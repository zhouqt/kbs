#include "bbslib.h"

int main()
{
	init_all();
	printf("<nobr><center>%s -- 新用户注册<hr color=\"green\">\n", BBSNAME);
	printf("<font color=\"green\">欢迎加入本站. 以下资料请如实填写.</font>");
	printf("<form method=\"post\" action=\"bbsdoreg\">\n");
	printf("<table width=\"600\">\n");
	printf("<tr><td align=\"right\">*请输入代号:</td><td align=\"left\"><input name=\"userid\" size=\"12\" maxlength=\"12\"> (2-12字符, 必须全为英文字母)</td></tr>\n");
	printf("<tr><td align=\"right\">*请输入密码:</td><td align=\"left\"><input type=\"password\" name=\"pass1\" size=\"12\" maxlength=\"12\"> (4-12字符)</td></tr>\n");
	printf("<tr><td align=\"right\">*请确认密码:</td><td align=\"left\"><input type=\"password\" name=\"pass2\" size=\"12\" maxlength=\"12\"></td></tr>\n");
	printf("<tr><td align=\"right\">*请输入昵称:</td><td align=\"left\"><input name=\"username\" size=\"20\" maxlength=\"32\"> (2-30字符, 中英文不限)</td></tr>\n");
	printf("<tr><td align=\"right\">*请输入您的真实姓名:</td><td align=\"left\"><input name=\"realname\" size=\"20\"> (请用中文, 至少2个汉字)</td></tr>\n");
	printf("<tr><td align=\"right\">*学校系级或工作单位:</td><td align=\"left\"><input name=\"dept\" size=\"40\">  (至少6个字符)</td></tr>\n");
	printf("<tr><td align=\"right\">*详细通讯地址:</td><td align=\"left\"><input name=\"address\" size=\"40\"> (至少6个字符)</td></tr>\n");
	/*printf("<tr><td align=\"right\">星座显示方式:</td> ");
	printf("<td align=\"left\"><input type=\"radio\" name=\"xz\" value=\"0\" checked>颜色区分性别 ");
	printf("<input type=\"radio\" name=\"xz\" value=\"1\">不区分颜色 ");
	printf("<input type=\"radio\" name=\"xz\" value=\"2\">不显示星座</td>\n");*/
	printf("<tr><td align=\"right\">您的email地址(可选):</td><td align=\"left\"><input name=\"email\" size=\"40\"></td></tr>\n");
	printf("<tr><td align=\"right\">您的联络电话(可选):</td><td align=\"left\"><input name=\"phone\" size=\"40\"></td></tr>\n");
	printf("<tr><td align=\"right\">上站留言(可选):</td><td align=\"left\">");
	printf("<textarea name=\"words\" rows=\"3\" cols=\"40\" wrap=\"physicle\"></textarea></td></tr>");
	printf("</table><hr color=\"green\">\n");
	printf("<input type=\"submit\" value=\"提交表格\"> <input type=\"reset\" value=\"重新填写\">\n");
	printf("</form></center>");
	http_quit();
}
