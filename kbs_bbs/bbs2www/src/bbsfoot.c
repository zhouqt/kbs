/*
 * $Id$
 */
#include "bbslib.h"

int main()
{
	int dt=0, mail_total, mail_unread;
	char *id="guest";

	init_all();
	if(loginok)
	{
		id=currentuser->userid;
		dt=abs(time(0) - u_info->logintime)/60;
				set_idle_time(u_info, time(0));
	}
	printf("<script language=\"JavaScript\">\n"
"<!--\n"
"function Init() {\n"
"  servertime=new Date()\n"
"  servertime.setTime(%d*1000)\n"
"  staytime=%d\n"
"  localtime=new Date()\n"
"  Time()\n"
"}\n"
"function Time(){\n"
" var now=new Date()\n"
" var Timer=new Date()\n"
" Timer.setTime(servertime.getTime()+now.getTime()-localtime.getTime());\n"
" var hours=Timer.getHours()\n"
" var minutes=Timer.getMinutes()\n"
" if (hours==0)\n"
" hours=12\n"
" if (minutes<=9)\n"
" minutes=\"0\"+minutes\n"
" var year=Timer.getYear();\n"
" if (year < 1900)   \n"
"	 year = year + 1900; \n"

" myclock=year+\"年\"+(Timer.getMonth()+1)+\"月\"+Timer.getDate()+\"日\"+hours+\":\"+minutes\n"
" var staysec=(now.getTime()-localtime.getTime())/60000+staytime;\n"
" stayclock=parseInt(staysec/60)+\"小时\"+parseInt(staysec%60)+\"分钟\"\n"
" document.clock.myclock.value=myclock\n"
" document.clock.stay.value=stayclock\n"
" setTimeout(\"Time()\",58000)\n"
"}\n"
"//JavaScript End-->\n"
"</script>",time(0)+20,loginok?dt:0);
 /*
if (document.layers){\n\
document.layers.position.document.write(myclock)\n\
document.layers.position.document.close()\n\
document.layers.stay.document.write(stayclock)\n\
document.layers.stay.document.close()\n\
}\n\
else if (document.all)\n\
position.innerHTML=myclock\n\
stay.innerHTML=stayclock\n\
setTimeout(\"Time()\",58000)\n\
 }\n\
 */

	printf("<style type=\"text/css\">\nA {color: #0000FF}\n</style>\n");
  	printf("<body onload=\"Init()\"><form name=\"clock\">");
  	printf("时间[<INPUT class=\"readonly\" TYPE=\"text\" NAME=\"myclock\" size=\"18\">] ");
	/*
  	printf("时间[<span id=\"myclock\"></span>] ");
	*/
	printf("在线[<a href=\"bbsusr\" target=\"f3\">%d</a>] ", count_online());
	printf("帐号[<a href=\"bbsqry?userid=%s\" target=\"f3\">%s</a>] ", id, id);
	if(loginok)
	{
		mail_total=mails(id, 0);
		mail_unread=mails(id, 1);
		if(mail_unread==0)
		{
			printf("信箱[<a href=\"bbsmail\" target=\"f3\">%d封</a>] ",
					mail_total);
		}
		else
		{
			printf("信箱[<a href=\"bbsmail\" target=\"f3\">%d(新信<font color=\"red\">%d</font>)</a>] ", 
				mail_total, mail_unread);
		}
	}
	printf("停留[<INPUT class=\"readonly\" TYPE=\"text\" NAME=\"stay\" size=\"10\">]</form>");
	/*
	printf("停留[<span id=\"stay\"></span>]");
	*/
  	printf("</body></html>");
}

int mails(char *id, int unread_only) {
        struct fileheader x;
        char path[80];
	int total=0, unread=0;
        FILE *fp;
	if(!loginok) return 0;
        sprintf(path, "mail/%c/%s/.DIR", toupper(id[0]), id);
        fp=fopen(path, "r");
        if(fp==0) return 0;
        while(fread(&x, sizeof(x), 1, fp)>0) {
                total++;
                if(!(x.accessed[0] & FILE_READ)) unread++;
        }
        fclose(fp);
	if(unread_only) return unread;
	return total;
}

