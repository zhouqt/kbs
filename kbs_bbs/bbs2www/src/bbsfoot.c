#include "bbslib.h"

int main() {
	int dt=0, mail_total, mail_unread;
	char *id="guest";

	init_all();
	printf("<style type=text/css>\nA {color: #0000FF}\n</style>\n");
  	printf("<body bgcolor=#c0c0f0>\n");
        if(loginok) {
		id=currentuser->userid;
		dt=abs(time(0) - *(int*)(u_info->from+32))/60;
                set_idle_time(u_info, time(0));
        }
  	printf("时间[%16.16s </a>] ", wwwCTime(time(0)));
	printf("在线[<a href=bbsusr target=f3>%d</a>] ", count_online());
	printf("帐号[<a href=bbsqry?userid=%s target=f3>%s</a>] ", id, id);
	if(loginok) {
		mail_total=mails(id, 0);
		mail_unread=mails(id, 1);
		if(mail_unread==0) {
			printf("信箱[<a href=bbsmail target=f3>%d封</a>] ", mail_total);
		} else {
			printf("信箱[<a href=bbsmail target=f3>%d(新信<font color=red>%d</font>)</a>] ", 
				mail_total, mail_unread);
		}
	}
	printf("停留[%d小时%d分]", dt/60, dt%60);
  	printf("<script>setTimeout('self.location=self.location', 240000);</script>");
  	printf("</body>");
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

