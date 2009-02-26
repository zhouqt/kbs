#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#ifdef _USE_NCURSE_
#include<curses.h>
#endif

#include"gamedefs.h"
#include"gamedata.h"
#include"gamedraw.h"
#include"gameinit.h"
#include"gameplay.h"
#include"gametty.h"
#include"screens.h"

#ifdef ENABLE_DL
#define MAXDATA 1458
#define RECORD_FILE "game/worker/high_record"
#define HOME_FILE "box_record"

struct high_record {
    int shortest;
    char userid[255];
};

static int load_highrecord(int level,struct high_record* hr,int myrecord)
{
    int fd,ret;

    bzero(hr,sizeof(*hr));
    if (level==-1) {
        hr->shortest=-1;
        return 0;
    }
    if (myrecord!=-1)
        fd=open(RECORD_FILE,O_RDWR|O_CREAT,0644);
    else
        fd=open(RECORD_FILE,O_RDONLY|O_CREAT,0644);

    if (fd==-1) {
        bbslog("3error","Box:can't open %s:%s",RECORD_FILE,strerror(errno));
        return -1;
    }

    lseek(fd,sizeof(*hr)*(level-1),SEEK_SET);
    if (myrecord!=-1)
        readw_lock(fd,0,SEEK_SET,sizeof(*hr)*MAXDATA);
    else
        readw_lock(fd,sizeof(*hr)*(level-1),SEEK_SET,sizeof(*hr));
    read(fd,hr,sizeof(*hr));
    ret=0;
    if (myrecord!=-1) {
        if (hr->shortest>myrecord||hr->shortest<=0) {
            int i;
            int count;
            struct high_record allrecord[MAXDATA];
            count=0;
            lseek(fd,0,SEEK_SET);
            read(fd,&allrecord,sizeof(*hr)*MAXDATA);
            for (i=0;i<MAXDATA;i++) {
                if (!strcasecmp(allrecord[i].userid,getCurrentUser()->userid))
                    count++;
            }
            if (count>20)
                ret = count;
            else {
                strcpy(hr->userid,getCurrentUser()->userid);
                hr->shortest=myrecord;
                lseek(fd,sizeof(*hr)*(level-1),SEEK_SET);
                write(fd,hr,sizeof(*hr));
                ret=1;
            }
        }
    }
    if (myrecord!=-1)
        un_lock(fd,0,SEEK_SET,sizeof(*hr)*MAXDATA);
    else
        un_lock(fd,sizeof(*hr)*(level-1),SEEK_SET,sizeof(*hr));
    close(fd);
    return ret;
}

#endif

static void update_endline1(struct high_record* hr, int steps)
{
    char buf[256];
    move(scr_lns-1,0);
    if (hr->shortest==0)
        sprintf(buf,"步数: \x1b[36m%4d\x1b[33m   尚无成功者，你加油哦     ",
                steps);
    else
        if (hr->shortest==-1)
            sprintf(buf,"步数: \x1b[36m%-4d                                       ",
                    steps);
        else
            if (hr->userid[0]) {
                sprintf(buf,"步数:\x1b[36m%-4d\x1b[33m纪录:\x1b[36m%-4d\x1b[33m保持者:\x1b[36m%-12s ",
                        steps,hr->shortest,hr->userid);
            } else
                sprintf(buf,"步数: \x1b[36m%-4d\x1b[33m本站纪录: \x1b[36m%-4d                ",
                        steps,hr->shortest);
    prints("\x1b[1;44;33m");
    prints(buf);
    prints("  \x1b[33m重绘[\x1b[32mR\x1b[33m] 反悔[\x1b[32mDel\x1b[33m] 退出[\x1b[32m回车\x1b[33m] 重来[\x1b[32mTAB\x1b[33m]\x1b[m");
}

#ifdef ENABLE_DL
int worker_main()
#else
int main(int argc, char **argv)
#endif
{
    char fname[256];
    char defname[] = "game.dat";
    Boolean b_play;
    int dir;
    int inch;
    int num_step;
    int num;
    unsigned char steplog[MaxSteps];

#ifdef ENABLE_DL
    char buf[10];
    struct high_record high;

    while (1) {
        num = 0;
        clear();
        ansimore2("game/worker/welcome", false, 0, 0);
        while (num <= 0 || num > MAXDATA) {
            buf[0]=0;
            getdata(5, 61, "", buf, 5, 1, NULL, true);
            if (buf[0] == 0)
                return 0;
            num = atoi(buf);
        }
        sprintf(fname, "game/worker/%s.%d", defname, num);
        clear();
#else
    {
        if (argc > 2) {
            sprintf(fname, "%s/%s.%s", argv[1], defname, argv[2]);
            num=atoi(argv[2]);
        } else if (2 == argc) {
            if (strlen(argv[1]) > 255)
                exit(1);
            sprintf(fname, "%s.%s", defname, argv[1]);
            num=atoi(argv[1]);
        } else {
            strcpy(fname, defname);
            num==-1;
        }
#endif
        num_step=0;
        load_highrecord(num, &high, -1);
        if (TRUE == InitData(fname)) {
            if (TRUE == InitPad()) {
                b_play = TRUE;
                DrawPad();
                update_endline1(&high,num_step);
                inch = 0;
                while (b_play) {
                    if (inch) {
                        showdiff();
                        cleardiff();
                        move(scr_lns-1, scr_cols - 1);
                        refresh();
                    }
                    if (wingame()) {
                        int newrecord;
                        char buf[100];
                        newrecord=0;
                        if (num_step<high.shortest||high.shortest==0)
                            newrecord=load_highrecord(num, &high, num_step);
                        clear();
                        move(12,15);
                        prints("祝贺你，你成功的完成了这一关");
                        move(13,15);
                        if (newrecord==1) {
                            sprintf(buf,"你也创造了本关使用步数的纪录(%d步)！好厉害～",num_step);
                        } else
                            if (newrecord!=0) {
                                sprintf(buf,"你也创造了本关使用步数的纪录(%d步)！",num_step);
                                prints(buf);
                                move(14,15);
                                sprintf(buf,"不过,让点创纪录机会给别人吧~你都有%d个记录了:P\n",newrecord);
                            } else {
                                sprintf(buf,"你用了%d步，纪录是%d步。",num_step,high.shortest);
                            }
                        prints(buf);
                        move(23, 0);
                        clrtoeol();
                        prints("\x1b[m                                \x1b[5;1;33m按任何键继续 ..\x1b[m");
                        igetkey();
                        break;
                    }
                    if (num_step>MaxSteps) {
                        char askbuf[100];
                        clear();
                        sprintf(askbuf,"你已经用了%d步还没有成功，重来么？(Y/N)",num_step);
                        getdata(12,0,askbuf,buf,2,1,NULL,true);
                        if (toupper(buf[0])=='N')
                            break;
                        b_play = InitPad();
                        DrawPad();
                        num_step=0;
                    }
                    inch = igetkey();
                    if ((' ' == inch)||(inch == '\n')||(inch=='\r'))
                        break;
                    dir = NullDir;
                    switch (inch) {
                        case 'i':
                        case 'I':
                        case KEY_UP:
                            dir = North;
                            break;
                        case 'j':
                        case 'J':
                        case KEY_LEFT:
                            dir = West;
                            break;
                        case 'l':
                        case 'L':
                        case KEY_RIGHT:
                            dir = East;
                            break;
                        case 'k':
                        case 'K':
                        case KEY_DOWN:
                            dir = South;
                            break;
                        case 12:   //      Ctrl-L
                        case 'R':   //      Ctrl-L
                            clear();
                            DrawPad();
                            break;
                        case '\t':
                            b_play = InitPad();
                            DrawPad();
                            num_step=0;
                            update_endline1(&high,num_step);
                            break;
                        case Ctrl('H'):
                        case '\177':
                        case KEY_DEL:
                            if (num_step!=0) {
                                num_step--;
                                regretmove(steplog[num_step]);
                                update_endline1(&high,num_step);
                            }
                            break;
                        default:
                            break;
                    }
                    if (NullDir != dir) {
                        int ret;
                        if ((ret=workermove(dir))!=0) {
                            if (ret==2)
                                steplog[num_step]=dir+8;
                            else
                                steplog[num_step]=dir;
                            num_step++;
                            update_endline1(&high,num_step);
                        }
                    }
                }
            }
            DestroyPad();
        }
        DestroyData();
    }
    return 0;
}
