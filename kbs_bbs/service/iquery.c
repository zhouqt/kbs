/******************************************************
Áîºü³åËÑË÷2003, ×÷Õß: bad@smth.org  Qian Wenjie
ÔÚË®Ä¾Çå»ªbbsÏµÍ³ÉÏÔËĞĞ

±¾ÓÎÏ·ÊÇ×ÔÓÉÈí¼ş£¬ÇëËæÒâ¸´ÖÆÒÆÖ²
ÇëÔÚĞŞ¸ÄºóµÄÎÄ¼şÍ·²¿±£Áô°æÈ¨ĞÅÏ¢
******************************************************/

#define BBSMAIN
#include "bbs.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>

#define MAX_KEEP 100

char res_title[MAX_KEEP][80],res_filename[MAX_KEEP][200],res_path[MAX_KEEP][200];
int res_total=0,toomany=0,wh=0,res_flag[MAX_KEEP],show_mode=0;

char qn[60];

int get_word()
{
    clear();
    prints("Áîºü³åËÑË÷");
    move(6, 0);
    prints("    ÎÒÏë²é                    ¹Ø¼ü×Ö");
    move(7, 0);
    prints("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-");
    move(8, 0);
    prints("    µçÓ°Ó°ÆÀ                  movie ÎŞ¼äµÀ");
    move(9, 0);
    prints("    Á÷ĞĞÀÖ¸è´Ê                popmusic ÎÒ»³ÄîÓĞÒ»ÄêµÄÏÄÌì");
    move(10, 0);
    prints("    ĞÂÎÅÒªµã                  news ¿ÆË÷ÎÖ");
    move(11, 0);
    prints("    ÎäÏÀĞ¡Ëµ                  emprise ´óÌÆĞĞïÚ");
    move(12, 0);
    prints("    Ê³ÎïÊ³Æ×                  food ÌÇ´×ÅÅ¹Ç");
    move(13, 0);
    prints("    ÓéÀÖÒªÎÅ                  estar ²Ì×¿åû ÖÓĞÀÍ©");
    move(14, 0);
    prints("    °²È«¼¼Êõ                  Ê¹ÓÃsoftice");
    move(15, 0);
    prints("    µçÄÔÓ²¼ş                  Òº¾§ÏÔÊ¾Æ÷");
    move(16, 0);
    prints("    ÓÎÏ·¹¦ÂÔ                  palsword Ö§Ïß");
    move(17, 0);
    prints("    ±à³ÌËã·¨                  Îå×ÓÆå Ëã·¨");
    move(18, 0);
    prints("    ÓÄÄ¬Ğ¦»°                  joke Å¼ºÍÅ¼mm");
    move(19, 0);
    prints("    °üº¬bad²»°üº¬goodÎÄÕÂ     bad -good");
    move(20, 0);
    prints("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-");
    getdata(2, 0, "²éÑ¯¹Ø¼ü×Ö: ", qn, 60, 1, 0, 0);
    move(3, 0);
    if(qn[0]) {
        prints("¿ªÊ¼²éÑ¯....");
        refresh();
    }
    return qn[0];
}

int get_pos(char * s)
{
    struct stat st;
    FILE* fp;
    char buf[240],buf2[100],tt[100];
    int i,j,k;
    if(stat(s, &st)==-1) return -1;
    strcpy(buf, s);
    i=strlen(buf)-1;
    while(buf[i]!='/') i--;
    i++;
    strcpy(buf2, buf+i);
    strcpy(buf+i, ".Names");
    fp=fopen(buf, "r");
    if(fp==NULL) return -1;
    tt[0]=0;
    j=0;
    while(!feof(fp))
    {
	if(!fgets(buf, 240, fp)) {
	    fclose(fp);
	    return -1;
	}
	if(buf[0]) buf[strlen(buf)-1]=0;
	if(!strncmp(buf, "Name=", 5)) {
	    strcpy(tt, buf+5);
	}
	if(!strncmp(buf, "Path=~/", 7)) {
	    j++;
	    if(!strcmp(buf+7, buf2)) {
		fclose(fp);
		return j;
	    }
	}
    }
    fclose(fp);
    return -1;
}

void do_query_all(int w, char * s)
{
    struct sockaddr_in addr;
    FILE* sockfp;
    int sockfd, i, j, k;
    char buf[256];
    char ip[20], s1[30], s2[30], *pp;
    
    strcpy(ip, sysconf_str("QUERY_SERVER"));
    
    res_total = -2;
    if(strstr(s, "·¨ÂÖ¹¦")) {
        res_total = -1;
        return;
    }
    
    j=1;
    for(i=0;i<strlen(s);i++)
        if(s[i]>='a'&&s[i]<='z'||s[i]>='A'&&s[i]<='Z'||s[i]>='0'&&s[i]<='9'||s[i]<0) {
            j=0;
            break;
        }
    if(j) {
        res_total = -1;
        return;
    }

    if((sockfd=socket(AF_INET, SOCK_STREAM, 0))==-1) return;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family=AF_INET;    
    addr.sin_addr.s_addr=inet_addr(ip);
    addr.sin_port=htons(4875);
    if(connect(sockfd, (struct sockaddr*)&addr, sizeof(addr))<0) return;
    sockfp=fdopen(sockfd, "r+");
    fprintf(sockfp, "\n%d\n%s\n", w, s);
    fflush(sockfp);
    fscanf(sockfp, "%d %d %d\n", &toomany, &i, &res_total);
    for(i=0;i<res_total;i++) {
        fgets(buf, 256, sockfp);
        if(buf[0]&&buf[strlen(buf)-1]=='\n') buf[strlen(buf)-1]=0;
        strncpy(res_title[i], buf, 80);
        res_title[i][79] = 0;

        fgets(buf, 256, sockfp);
        if(buf[0]&&buf[strlen(buf)-1]=='\n') buf[strlen(buf)-1]=0;
        strncpy(res_filename[i], buf, 200);
        res_filename[i][199] = 0;
    }
    fclose(sockfp);
    close(sockfd);
    for(i=0;i<res_total;i++)
	if(get_pos(res_filename[i])==-1) {
	    strcpy(res_path[i], "ÎŞĞ§ÎÄÕÂ");
	    res_flag[i]=1;
	}
        else {
	    char buf[200],buf2[200];
	    res_flag[i]=0;
	    if(!strncmp(res_filename[i], "0Announce/groups/", 17)) {
		j=17;
		while(res_filename[i][j]!='/') j++;
		j++;
		k=j;
		while(res_filename[i][j]!='/') j++;
		strcpy(buf,res_filename[i]+k);
		buf[j-k]=0;
		strcpy(res_path[i],buf);
		k=strlen(res_path[i]);
		while(1) {
		    j++;
		    while(res_filename[i][j]!='/'&&res_filename[i][j]) j++;
		    strcpy(buf2, res_filename[i]);
		    buf2[j] = 0;
		    sprintf(res_path[i]+k, "-%d", get_pos(buf2));
		    k = strlen(res_path[i]);
		    if(!res_filename[i][j]) break;
		}
	    }
	    else {
		j=10;
		strcpy(res_path[i], "¾«»ªÇø");
		k=strlen(res_path[i]);
		while(1) {
		    j++;
		    while(res_filename[i][j]!='/'&&res_filename[i][j]) j++;
		    strcpy(buf2, res_filename[i]);
		    buf2[j] = 0;
		    sprintf(res_path[i]+k, "-%d", get_pos(buf2));
		    k = strlen(res_path[i]);
		    if(!res_filename[i][j]) break;
		}
	    }
	}
    for(i=0;i<res_total;i++) 
    if(res_title[i][0]){
        if(strlen(res_title[i])>30){
	    j=30;
	    while(res_title[i][j]!=' '&&res_title[i][j])j++;
	    res_title[i][j]=0;
        }
	j=strlen(res_title[i])-1;
	if((j>=0)&&res_title[i][j]==' ')j--;
	j++;
	res_title[i][j]=0;
//	if(j>=8&&!strcmp(res_title[i]+j-7," (×ªÔØ)"))
//	    res_title[i][j-7]=0;
    }
    for(i=0;i<res_total;i++) {
	pp=res_filename[i]+strlen(res_filename[i])-1;
	while(*pp!='/') pp--;
	strcpy(s1, pp+1);
	if(strlen(s1)>7)
	for(j=i+1;j<res_total;j++) {
	    pp=res_filename[j]+strlen(res_filename[j])-1;
	    while(*pp!='/') pp--;
	    strcpy(s2, pp+1);
	    if(!strcmp(s1,s2)) {
		res_flag[j]=1;
		strcpy(res_path[j], "ÖØ¸´ÎÄÕÂ");
	    }
	}
    }
}

static int choose_file_refresh(struct _select_def *conf)
{
    clear();
    docmdtitle("[Áîºü³åËÑË÷]",
              "  ÍË³ö[\x1b[1;32m¡û\x1b[0;37m,\x1b[1;32me\x1b[0;37m] ²ì¿´[\x1b[1;32mEnter\x1b[0;37m] Ñ¡Ôñ[\x1b[1;32m¡ü\x1b[0;37m,\x1b[1;32m¡ı\x1b[0;37m] ·­Ò³²éÕÒ[\x1b[1;32m[\x1b[0;37m,\x1b[1;32m]\x1b[0;37m]         ×÷Õß: \x1b[31;1mbad@smth.org\x1b[m");
    move(2, 0);
    prints("[0;1;37;44m    %4s %-35s %s    %d-%d ¹²%d  ¹Ø¼ü×Ö:%s", "±àºÅ", "±êÌâ", "Â·¾¶", wh*MAX_KEEP+1, wh*MAX_KEEP+res_total, toomany, qn);
    clrtoeol();
    resetcolor();
    update_endline();
    return SHOW_CONTINUE;
}

static int choose_file_show(struct _select_def *conf, int ii)
{
    struct room_struct * r;
    prints("  %3d  %-35s %s", ii+wh*MAX_KEEP, res_title[ii-1], res_path[ii-1]);
    clrtoeol();
    if(show_mode) {
        char buf[10*1024],out[10*1024],out2[10*1024];
        FILE* fp;
        int i,j,k,l,fsize=0,t=0;
        fp = fopen(res_filename[ii-1], "rb");
        if(!fp) {
            prints("\n"); clrtoeol();
            prints("\n"); clrtoeol();
            prints("\n"); clrtoeol();
            return SHOW_CONTINUE;
        }
        fsize = fread(buf, 1, 10*1024, fp);
        fclose(fp);
        memset(out, 0, sizeof(out));
        i=0;
        if(qn[i]=='=') {
            while(i<strlen(qn)&&qn[i]!=' ') i++;
            if(i>=strlen(qn)) i=0;
        }
        while(i<strlen(qn)) {
            if(qn[i]>='a'&&qn[i]<='z'||qn[i]>='A'&&qn[i]<='Z') {
                j=i;
                while(qn[j]>='a'&&qn[j]<='z'||qn[j]>='A'&&qn[j]<='Z'||qn[j]>='0'&&qn[j]<='9') j++;
                for(k=0;k<fsize-(j-i);k++)
                    if(!strncmp(qn+i,buf+k,j-i)&&(k==0||!(buf[k-1]>='a'&&buf[k-1]<='z'||buf[k-1]>='A'&&buf[k-1]<='Z'))&&
                        (k==fsize-1||!(buf[k+j-i]>='a'&&buf[k+j-i]<='z'||buf[k+j-i]>='A'&&buf[k+j-i]<='Z')))
                        for(l=0;l<j-i;l++) if(!out[k+l]){out[k+l]=1;t++;}
                i=j-1;
            }
            if(qn[i]>='0'&&qn[i]<='9') {
                j=i;
                while(qn[j]>='0'&&qn[j]<='9') j++;
                for(k=0;k<fsize-(j-i);k++)
                    if(!strncmp(qn+i,buf+k,j-i)&&(k==0||!(buf[k-1]>='0'&&buf[k-1]<='9'))&&
                        (k==fsize-1||!(buf[k+j-i]>='0'&&buf[k+j-i]<='9')))
                        for(l=0;l<j-i;l++) if(!out[k+l]){out[k+l]=1;t++;}
                i=j-1;
            }
            if(qn[i]<0&&qn[i+1]<0) {
                j=i+2;
                for(k=0;k<fsize-(j-i);k++)
                    if(!strncmp(qn+i,buf+k,j-i))
                        for(l=0;l<j-i;l++) if(!out[k+l]){out[k+l]=1;t++;}
                i=j-1;
            }
            i++;
        }
        if(t>=20) {
            for(k=0;k<fsize-4;k++) {
                if(out[k]==0&&out[k+1]==1&&out[k+2]==0) {
                    out[k+1]=0;
                    t--;
                }
                if(out[k]==0&&out[k+1]==1&&out[k+2]==1&&out[k+3]==0) {
                    out[k+1]=0;
                    out[k+2]=0;
                    t-=2;
                }
                if(t<10) break;
            }
        }
        if(t==0) {
            prints("\n"); clrtoeol();
            prints("\n"); clrtoeol();
            prints("\n"); clrtoeol();
            return SHOW_CONTINUE;
        }
        while(t<180) {
            t=0;
            for(k=0;k<fsize;k++) {
                out2[k]=0;
                if(out[k]||k>0&&out[k-1]||k<fsize-1&&out[k+1]) {
                    if(out[k]==1)
                        out2[k]=1;
                    else
                        out2[k]=2;
                    t++;
                }
            }
            memcpy(out,out2,fsize);
        }
        prints("\n          ");
        j=0;
        for(i=0;i<fsize;i++)
        if(out[i]) {
            if(i>0&&out[i-1]==0) {
                prints("..."); j+=3;
            }
            if(out[i]==1)
                setfcolor(RED,1);
            else
                resetcolor();
            prints("%c",buf[i]);
            j++;
            if(j>=69) {
                prints("\n          ");
                j=0;
            }
        }
        resetcolor();
    }
    return SHOW_CONTINUE;
}

static int choose_file_select(struct _select_def *conf)
{
    char * ss;
    int ch;
again:
    ss = res_filename[conf->pos-1];
    ch = ansimore_withzmodem(ss, 0, res_title[conf->pos-1]);
    move(t_lines-1, 0);
    prints("[0;1;31;44m[µÚ%d/%dÆª]  [33mÂ·¾¶:%s©¦ËÑË÷¹Ø¼ü×Ö:%s", conf->pos+wh*MAX_KEEP, toomany, res_path[conf->pos-1], qn);
    clrtoeol();
    if(ch==0) ch=igetkey();
    switch(ch){
        case KEY_UP:
        case 'l':
        case 'k':
            conf->pos--;
            if(conf->pos<=0) conf->pos = res_total;
            goto again;
        case KEY_DOWN:
        case ' ':
        case 'n':
        case 'j':
            conf->pos++;
            if(conf->pos>res_total) conf->pos = 1;
            goto again;
        case Ctrl('Y'):
            zsend_file(ss, res_title[conf->pos-1]);
            break;
    }
    return SHOW_DIRCHANGE;
}

static int choose_file_getdata(struct _select_def *conf, int pos, int len)
{
    conf->item_count = res_total;
    return SHOW_CONTINUE;
}

static int choose_file_prekey(struct _select_def *conf, int *key)
{
    switch (*key) {
    case 'e':
    case 'q':
        *key = KEY_LEFT;
        break;
    case 'p':
    case 'k':
        *key = KEY_UP;
        break;
    case ' ':
    case 'N':
        *key = KEY_PGDN;
        break;
    case 'n':
    case 'j':
        *key = KEY_DOWN;
        break;
    }
    return SHOW_CONTINUE;
}

static int choose_file_key(struct _select_def *conf, int key)
{
    struct fileheader fh;
    char buf[240],buf2[80];
    int i;
    POINT *pts;
    switch (key) {
    case ']':
        if((wh+1)*MAX_KEEP+1<=toomany) {
            move(t_lines-1, 0);
            prints("²éÕÒÊı¾İ....");
            clrtoeol();
            refresh();
            wh++;
            do_query_all(wh, qn);
            return SHOW_DIRCHANGE;
        }
	break;
    case '[':
        if(wh>0) {
            move(t_lines-1, 0);
            prints("²éÕÒÊı¾İ....");
            clrtoeol();
            refresh();
            wh--;
            do_query_all(wh, qn);
            return SHOW_DIRCHANGE;
        }
	break;
    case 'o':
    case 'O':
	t_friends();
	return SHOW_REFRESH;
    case 'v':
	i_read_mail();
	return SHOW_REFRESH;
    case 'F':
	bzero(&fh, sizeof(struct fileheader));
	strcpy(buf, res_filename[conf->pos-1]);
	i=strlen(buf)-1;
	while(buf[i]!='/') i--;
	strcpy(buf2, buf+i+1);
	buf[i]=0;
	strncpy(fh.title, res_title[conf->pos-1], STRLEN);
	strncpy(fh.filename, buf2, FILENAME_LEN);
	if(!doforward(buf, &fh, 0)) prints("ÎÄÕÂ×ª¼Ä³É¹¦");
	else prints("ÎÄÕÂ×ª¼ÄÊ§°Ü");
	pressanykey();
	return SHOW_REFRESH;
    case '+':
        free(conf->item_pos);
        show_mode = 1;
        pts = (POINT *) malloc(sizeof(POINT) * BBS_PAGESIZE);
        for (i = 0; i < BBS_PAGESIZE; i++) {
            pts[i].x = 2;
            pts[i].y = i*4+3;
        }
        conf.item_per_page = BBS_PAGESIZE/4;
        conf.item_pos = pts;
        return SHOW_DIRCHANGE;
    case '-':
        free(conf->item_pos);
        show_mode = 0;
        pts = (POINT *) malloc(sizeof(POINT) * BBS_PAGESIZE);
        for (i = 0; i < BBS_PAGESIZE; i++) {
            pts[i].x = 2;
            pts[i].y = i + 3;
        }
        conf.item_per_page = BBS_PAGESIZE;
        conf.item_pos = pts;
        return SHOW_DIRCHANGE;
    }
    return SHOW_CONTINUE;
}

int choose_file()
{
    struct _select_def grouplist_conf;
    int i;
    POINT *pts;

    bzero(&grouplist_conf, sizeof(struct _select_def));
    grouplist_conf.item_count = res_total;
    pts = (POINT *) malloc(sizeof(POINT) * BBS_PAGESIZE);
    for (i = 0; i < BBS_PAGESIZE; i++) {
        pts[i].x = 2;
        if(show_mode) pts[i].y = i*4+3;
        else pts[i].y = i + 3;
    }
    if(show_mode)
        grouplist_conf.item_per_page = BBS_PAGESIZE/4;
    else
        grouplist_conf.item_per_page = BBS_PAGESIZE;
    grouplist_conf.item_pos = pts;
    grouplist_conf.flag = LF_VSCROLL | LF_BELL | LF_LOOP | LF_MULTIPAGE;
    grouplist_conf.prompt = "¡ô";
    grouplist_conf.arg = NULL;
    grouplist_conf.title_pos.x = 0;
    grouplist_conf.title_pos.y = 0;
    grouplist_conf.pos = 1;     /* initialize cursor on the first mailgroup */
    grouplist_conf.page_pos = 1;        /* initialize page to the first one */

    grouplist_conf.on_select = choose_file_select;
    grouplist_conf.show_data = choose_file_show;
    grouplist_conf.pre_key_command = choose_file_prekey;
    grouplist_conf.key_command = choose_file_key;
    grouplist_conf.show_title = choose_file_refresh;
    grouplist_conf.get_data = choose_file_getdata;
    list_select_loop(&grouplist_conf);
    free(pts);
}

int show_res(int o)
{
    int i;
    if(res_total<=0) {
        if(o) move(t_lines-1, 0);
        else move(4,0);
        if(res_total==-2) prints("¸ººÉÌ«ÖØ£¬ÇëÉÔºóÔÙÊ¹ÓÃ");
        else prints("Ê²Ã´¶¼Ã»ËÑµ½£¡");
        clrtoeol();
        igetkey();
    }
    else {
        choose_file();
    }
}

int iquery_board(char * board, char * w)
{
    sprintf(qn, "=%s %s", board, w);
    wh = 0;
    do_query_all(wh, qn);
    show_res(1);
}

int iquery_main()
{
    qn[0] = 0;
    while(get_word()) {
        wh = 0;
        do_query_all(wh, qn);
        show_res(0);
    }
}
