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

char res_title[MAX_KEEP][80],res_filename[MAX_KEEP][200];
int res_total=0,toomany=0;

char qn[60];

int get_word()
{
    clear();
    prints("Áîºü³åËÑË÷");
    getdata(2, 0, "²éÑ¯¹Ø¼ü×Ö: ", qn, 60, 1, 0, 0);
    return qn[0];
}

static int choose_file_refresh(struct _select_def *conf)
{
    clear();
    docmdtitle("[Áîºü³åËÑË÷]",
              "  ÍË³ö[\x1b[1;32m¡û\x1b[0;37m,\x1b[1;32me\x1b[0;37m] ²ì¿´[\x1b[1;32mEnter\x1b[0;37m] Ñ¡Ôñ[\x1b[1;32m¡ü\x1b[0;37m,\x1b[1;32m¡ı\x1b[0;37m]                        ×÷Õß: \x1b[31;1mbad@smth.org\x1b[m");
    move(2, 0);
    if(toomany)
        prints("[0;1;37;44m    %4s %-30s %s    ¹²ËÑË÷µ½ %d Ïî£¬±£ÁôÇ° %d Ïî", "±àºÅ", "±êÌâ", "Â·¾¶", toomany, res_total);
    else
        prints("[0;1;37;44m    %4s %-30s %s    ¹²ËÑË÷µ½ %d Ïî", "±àºÅ", "±êÌâ", "Â·¾¶", res_total);
    clrtoeol();
    resetcolor();
    update_endline();
    return SHOW_CONTINUE;
}

static int choose_file_show(struct _select_def *conf, int i)
{
    struct room_struct * r;
    char f1[160],f2[160];
    strcpy(f1, res_title[i-1]);
    strcpy(f2, res_filename[i-1]);
    prints("  %3d  %-30s %s", i, f1, f2);
    return SHOW_CONTINUE;
}

static int choose_file_select(struct _select_def *conf)
{
    char * ss = res_filename[conf->pos-1];
    ansimore_withzmodem(ss, 1, "uhoh");
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
        pts[i].y = i + 3;
    }
    grouplist_conf.item_per_page = BBS_PAGESIZE;
    grouplist_conf.flag = LF_VSCROLL | LF_BELL | LF_LOOP | LF_MULTIPAGE;
    grouplist_conf.prompt = "¡ô";
    grouplist_conf.item_pos = pts;
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

int show_res()
{
    int i;
    if(res_total<=0) {
        move(4,0);
        if(res_total==-2) prints("¸ººÉÌ«ÖØ£¬ÇëÉÔºóÔÙÊ¹ÓÃ");
        else prints("Ê²Ã´¶¼Ã»ËÑµ½£¡");
        igetkey();
    }
    else {
        choose_file();
    }
}

void do_query_all(char * s)
{
    struct sockaddr_in addr;
    FILE* sockfp;
    int sockfd, i;
    char buf[256];
    char ip[20]="166.111.8.235";
    
    res_total = -2;
    
    if((sockfd=socket(AF_INET, SOCK_STREAM, 0))==-1) return;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family=AF_INET;    
    addr.sin_addr.s_addr=inet_addr(ip);
    addr.sin_port=htons(4875);
    if(connect(sockfd, (struct sockaddr*)&addr, sizeof(addr))<0) return;
    sockfp=fdopen(sockfd, "r+");
    fprintf(sockfp, "0\n%s\n", s);
    fflush(sockfp);
    fscanf(sockfp, "%d %d %d\n", &toomany, &i, &res_total);
    if(toomany==res_total) toomany=0;
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
}

int iquery_main()
{
    qn[0] = 0;
    while(get_word()) {
        do_query_all(qn);
        show_res();
    }
}
