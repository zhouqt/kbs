#include "bbs.h"
#include "mvote.h"
#define EDITOR "ve"
#define MAXITEMS (30)
#define MAXTERM  (30)
#define MAXLEN  (256)
#define Limit_title (40)
#define Limit_Pageline (19)
#define Ans_noans (0)
#define Ans_yes (1)
#define Ans_not (-1)
#define No_Ans (0)

char    username[ STRLEN ];
int     vote_out[MAXITEMS];

char currentboard[80];
struct vote_class *vote, *temp ;
int choice_num;
char *uname;
int CanOpen;            /* 用来判断现在是否可以改变票箱, True or False */

char *choice_type[] = { "是非", "单选", "复选", "数字", "问答", NULL };

report()
{};


suckinfile(fp, fname)
FILE *fp;
char *fname;
{
    char inbuf[256];
    FILE *sfp;
    if ((sfp = fopen(fname, "r")) == NULL) return -1;
    while (fgets(inbuf, sizeof(inbuf), sfp) != NULL)
        fputs(inbuf, fp);
    fclose(sfp);
    return 0;
}

pause()
{
    char        junk[ STRLEN ];

    printf( "请按 [Enter] 键继续 ...                            " );
    igetstr( junk, sizeof(junk) );
}

prompt( mesg, buf, maxlen )
char    *mesg, *buf;
{
    gotoxy( 1, 23 );
    clreol();
    printf( "%s", mesg );
    return igetstr( buf, maxlen );
}

message( mesg )
char    *mesg;
{
    gotoxy( 1, 24 );
    clreol();
    printf( "%s", mesg );
}

mksure( mesg )
char    *mesg;
{
    gotoxy( 1, 23 );
    clreol();
    printf( "%s", mesg );
    igetstr( genbuf, sizeof(genbuf) );
    if( strcmp( genbuf, "Y" ) == 0 ||
            strcmp( genbuf, "y" ) == 0 )  return 1;
    return 0;
}

errmsg( mesg )
char    *mesg;
{
    gotoxy( 1, 24 );
    clreol();
    printf( "%s", mesg );
    igetkey();
}

more( fname )
char *fname;
{
    FILE *fp;
    int i, ch, printline, endofile, is_new;
    char inbuf[1200];

    if((fp=fopen(fname,"r")) == NULL) {
        return -1;
    }
    clrscr();
    for(printline=22, endofile=NA, is_new=YEA;;) {
        if(is_new == NA) {
            gotoxy(1,24);
            clreol();
        } else
            is_new = NA ;
        for(i=0;i<printline;i++) {
            if(fgets(inbuf, 80, fp) == NULL) {
                endofile = YEA ;
                break;
            }
            printf("%s\r", inbuf) ;
        }
        if(endofile == NA)
            message("[↓/Space] 下一页  [Enter] 下一行  [←/Q] 中断 : ") ;
        else {
            gotoxy(1,24);
            pause();
            fclose(fp);
            return FULLUPDATE;
        }
        for(;;) {
            inbuf[0] = YEA;
            ch=igetkey();
            switch( ch ) {
            case Ctrl('Z'): r_lastmsg(); /* Leeward 98.07.30 support msgX */
                break;
        case KEY_DOWN: case ' ':
                printline = 22;
                break;
            case '\r':
                printline = 1;
                break;
    case KEY_LEFT: case 'Q': case 'q':
                gotoxy(1,24);
                pause();
                fclose(fp);
                return FULLUPDATE;
                break;
defaults:
                inbuf[0] = NA;
            }
            if(inbuf[0] == YEA) break;
        }
    }
}

execute( command, argument )
char    *command, *argument;
{
    sprintf( genbuf, "%s %s", command, argument );
    system( genbuf );
}

tty_exec( command, argument )
char    *command, *argument;
{
    clrscr();
    reset_tty();
    sprintf( genbuf, "%s %s", command, argument );
    system( genbuf );
    restore_tty();
}

valid( str )
char    *str;
{
    char        ch;

    while( (ch = *str++) != '\0' ) {
        if( (ch >= 'A' && ch <= 'Z') ||
                (ch >= 'a' && ch <= 'z') ||
                (ch >= '0' && ch <= '9') ||
                strchr( "@[]-._", ch ) != NULL ) {
            ;
        } else {
            return 0;
        }
    }
    return 1;
}

relocate(is_free, filename)
int is_free;
char *filename;
{
    int i;

    if(is_free) {
        free(vote);
    }
    sprintf(genbuf, "%s//%s", currentboard, filename);
    choice_num = get_num_records(genbuf, sizeof(struct vote_class));
    vote = (struct vote_class*)malloc(choice_num*sizeof(struct vote_class)) ;

    for(i=0;i<choice_num;i++) {
        get_record(genbuf, &vote[i], sizeof(struct vote_class),i+1) ;
    }
    return;
}

pull_ballots(probnum, choice)
int probnum, choice;
{
    int fd;

    sprintf(genbuf,"%s//ballots.%d", currentboard, probnum);
    if ((fd = open(genbuf, O_WRONLY|O_CREAT|O_APPEND, 0600)) == 0) {
        printf("\n\r投票箱没开洞?! 放弃...\n");
        pause();
        return (-1) ;
    }
    else {
        struct stat statb;
        flock(fd, LOCK_EX);
        sprintf(genbuf, "%d\n",choice);
        write(fd, genbuf, strlen(genbuf));
        flock(fd, LOCK_UN);
        fstat(fd, &statb);
        close(fd);
    }
    return 0;
}

vote_yn(probnum)
int probnum;
{
    int ans;

    for(;;) {
        gotoxy(1,23);
        printf("[Y] 同意, [N] 反对, [enter] 无意见 ");
        ans = igetstr(genbuf, 2);
        if(genbuf[0] == 'Y' || genbuf[0] == 'y') {
            ans = Ans_yes;
            break;
        }
        if(genbuf[0] == 'N' || genbuf[0] == 'n') {
            ans = Ans_not;
            break;
        }
        if(ans == 0) {
            ans = Ans_noans;
            break;
        }
    }
    gotoxy(1,23);
    clreol();
    pull_ballots(probnum, ans) ;
    return( FULLUPDATE ) ;
}

vote_multi(ball)
struct vote_class *ball;
{
    int num, i, ptr, lastptr, mode, ch, VoteNum;
    struct vote_class tball;

    sprintf(genbuf, "%s//.MENU.%d",currentboard, ball->qid);
    num = get_num_records(genbuf, sizeof(struct vote_class));
    if(num == 0) return FULLUPDATE;
    temp = (struct vote_class*)malloc(num*sizeof(struct vote_class));
    for(i=0;i<num;i++) {
        get_record(genbuf, &temp[i], sizeof(struct vote_class),i+1) ;
        temp[i].type = No_Ans ;
    }
    dovotelist(temp, 1, -1, num, 0);

    ptr=1;
    VoteNum = 0 ;
    for(mode=FULLUPDATE;;mode=PARTUPDATE) {
        gotoxy(1,21);
        printf("\033[1;34m题目: \033[1;33m%s\033[0m",ball->title);
        if(CanOpen==NA) {
            gotoxy(1,23);
            printf("\033[1;31m一个人最多 %d 票!\033[0m\n\r",ball->constrain);
        }
        ch = igetkey();
        lastptr = ptr ;
        switch( ch ) {
        case Ctrl('Z'): r_lastmsg(); /* Leeward 98.07.30 support msgX */
            break;
        case KEY_UP :
            if(ptr <= 1)
                ptr = num;
            else
                ptr--;
            break;
        case KEY_DOWN :
            if(ptr >= num)
                ptr= 1;
            else
                ptr++;
            break;
        case KEY_LEFT :
            if(mksure("[Y]确定并离开, [N]继续选择: [N] ")) {
                for(i=0;i<num;i++) {
                    if(temp[i].IsAns == NA)
                        continue;
                    pull_ballots(ball->qid,i);
                }
                free(temp);
                return FULLUPDATE;
            }
            break;
        case KEY_RIGHT :
            sprintf(genbuf, "%s//.instruct.%d.%d", currentboard,ball->qid,ptr);
            if(more( genbuf )==-1) {
                gotoxy(1,22);
                printf("此票箱无额外说明!\n");
                gotoxy(1,24);
                pause();
            }
            mode = FULLUPDATE;
            break;
        case 'i' :
            if(CanOpen == NA) {
                mode = DONOTHING;
                break;
            }
            sprintf(genbuf, "bin/ve %s/.instruct.%d.%d",currentboard, ball->qid,ptr);
            system( genbuf );
            mode = FULLUPDATE ;
            break;
        case 'v':
            if(CanOpen == YEA) {
                gotoxy(1,20);
                printf("设定票所中, 无法投票!");
                gotoxy(1,23);
                pause();
                break;
            }
            if(temp[ptr-1].IsAns == YEA) {
                VoteNum-- ;
                temp[ptr-1].IsAns = NA ;
            } else {
                if(VoteNum >= ball->constrain) break;
                VoteNum++ ;
                temp[ptr-1].IsAns = YEA;
            }
            ball->IsAns = YEA ;
            mode = FULLUPDATE ;
            break;
        case Ctrl('P'):
                        if(CanOpen == NA) break;
            gotoxy(1,20);
            printf("请输入第 [%d] 个选项叙述 (按 enter 结束)\n\r",num+1);
            clreol();
            tball.qid = num ;
            if(igetstr(tball.title,75)==0)
                break;
            if(!mksure("确定吗(Y/N) ? [N] : "))
                break;
            sprintf(genbuf,"%s//.MENU.%d",currentboard,ball->qid);
            tball.IsAns = NA ;
            append_record(genbuf, &tball, sizeof(tball));
            free(temp);
            num = get_num_records(genbuf, sizeof(struct vote_class));
            if(num == 0) return FULLUPDATE;
            temp = (struct vote_class*)malloc(num*sizeof(struct vote_class));
    for(i=0;i<num;i++) {
                get_record(genbuf, &temp[i], sizeof(struct vote_class), i+1);
                temp[i].type = No_Ans ;
            }
            mode = FULLUPDATE ;
            break;
            defaults:
            break;
        }
        if(mode == FULLUPDATE)
            dovotelist(temp, ptr,-1, num, 0);
        else
            dovotelist(temp, ptr,lastptr,num, 0) ;
    }
    gotoxy(1,23);
    pause();
    gotoxy(1,23);
    clreol();
    free(temp);
    return FULLUPDATE ;
}

vote_value(ball)
struct vote_class *ball;
{
    int ans;
    gotoxy(1,23);
    printf("请输入数字(介於 0 至 %d 间) : ",ball->constrain);
    if((ans=igetstr(genbuf,10)) == 0) {
        gotoxy(1,24) ;
        printf("错误!");
        return ;
    }
    ans = atof(genbuf) ;
    if(ans < 0 || ans > ball->constrain) {
        gotoxy(1,24) ;
        printf("数值范围超过!");
        return ;
    }
    pull_ballots(ball->qid, ans);
    ball->IsAns = YEA;
    return( PARTUPDATE ) ;
}

vote_asking(ball)
struct vote_class *ball;
{
    FILE *fpin, *fpout;
    int i;

    clrscr();
    gotoxy(1,2);
    printf("\033[0;31m人生海海, 有些事也要让你知道 : \033[0m");
    gotoxy(1,4);
    if(ball->constrain == 0) ball->constrain = 10;
    printf("本问答题限制您仅能有 %d 行作答, 多馀部份系统将会截掉!",ball->constrain);
    gotoxy(1,6);
    printf("若您准备开始回答, 就请按 enter !");
    gotoxy(1,8);
    pause();
    sprintf(genbuf, "%s//temp.%s",currentboard, uname);
    execute( EDITOR, genbuf );
    if(! mksure("确定[Y], 放弃[N] ? [N] ")) {
        sprintf(genbuf, "rm -rf %s//temp.%s",currentboard,uname);
        system(genbuf);
        return ;
    }
    ball->IsAns = YEA;
    sprintf(genbuf, "%s//temp.%s",currentboard, uname);
    if((fpin = fopen(genbuf,"r")) == NULL) {
        pause();
        return FULLUPDATE;
    }
    sprintf(genbuf, "%s//ballots.%d",currentboard, ball->qid);
    if((fpout = fopen(genbuf,"a")) == NULL) {
        pause();
        return FULLUPDATE;
    }
    fputs("[某人说]\n", fpout);
    for(i=0;i<ball->constrain;) {
        if(fgets(genbuf, 80, fpin) != NULL) {
            if(genbuf[0] == '\n' || genbuf[0] == '\r')
                continue;
            fputs(genbuf, fpout) ;
            i++;
        }
        else
            break;
    }
    fputs("\n",fpout);
    fclose(fpin);
    fclose(fpout);
    sprintf(genbuf, "rm -rf %s//temp.%s",currentboard, uname);
    system(genbuf);
    return FULLUPDATE;
}

OpenMulti(ball)
struct vote_class *ball;
{
    int i, num;

    struct vote_class tball;
    clrscr();
    gotoxy(10,18);
    printf("题目: %s", ball->title);
    for(i=1;i<MAXTERM;i++) {
        gotoxy(1,20);
        printf("请输入第 [%d] 个选项叙述 (按 enter 结束) :\n\r",i);
        clreol();
        if((num=igetstr(tball.title,75))==0)
            break;
        tball.qid = i-1;
        if(!mksure("确定吗(Y/N) ? [N] : ")) {
            i--;
            continue;
        }
        tball.IsAns = NA ;
        sprintf(genbuf, "%s//.MENU.%d",currentboard, ball->qid);
        append_record(genbuf, &tball, sizeof(tball) );
        vote_multi(ball);
        break;
    }
}

init_OpenVote() /* add by sch */
{
    char ans[10], genbuf[80], *newline;
    struct stat statb ;
    FILE *fp;
    time_t closetime, numseconds ;

    clrscr();
    gotoxy(1,1);
    printf("\033[1;32m打开多重投票器!\033[0m");
    if(stat(currentboard, &statb) != 0) {
        sprintf(genbuf, "mkdir %s", currentboard);
        system(genbuf);
    }
    sprintf(genbuf,"%s//control", currentboard);
    if ((fp = fopen(genbuf, "r")) != NULL) {
        fgets(genbuf, sizeof(genbuf), fp);
        fclose(fp);
        if (newline = index(genbuf, '\n')) *newline = '\0';
        closetime = atof(genbuf);
        gotoxy(1,3) ;
        printf("投票所使用中!\n");
        printf("\n\r本次投票预计在 %s 结束", ctime(&closetime));
        printf("\n\n\r若想提早结束投票, 请通知 sysop 修改 'control'.");
        gotoxy(1,23);
        pause();
        clrscr();
        return -1;
    }
    printf("\n\n\r请先编辑投票引言\n");
    gotoxy(1,23);
    pause();
    sprintf(genbuf, "%s//desc", currentboard);
    for(;;) {
        execute( EDITOR, genbuf );
        execute( "chmod 0644", genbuf );
        if(mksure("确定吗(Y/N)? [N]"))
            break;
        if(mksure("是否要作修正(Y/N)? [N]"))
            continue ;
        sprintf(genbuf, "rm -r %s",currentboard);
        system( genbuf );
        gotoxy(1,1);
        printf("\n\r\033[0;33m放弃开启投票所!\033[0m");
        gotoxy(1,23);
        pause();
        clrscr();
        return(-1) ;
    }
    sprintf(genbuf, "%s//prepare", currentboard);
    fp = fopen(genbuf, "w");
    clrscr();
    gotoxy(1,1);
    printf("预计投票几天: ");
    igetstr(genbuf, 3);
    numseconds = atof(genbuf) * 86400.0;
    time(&closetime);
    closetime += numseconds;
    fprintf(fp, "%lu\n", closetime);
    fclose(fp);
    return 0;
}

NewVote()
{
    struct vote_class ball;
    struct stat statb;
    int ans;

    if(CanOpen == NA) return ;
    clrscr();
    gotoxy(1,1);
    printf("\033[0;32m开启新票箱!\033[0m");
    gotoxy(1,3);
    printf("票箱型式:");
    for(;;) {
        gotoxy(1,4);
        printf("[1]是非, [2]单选, [3]复选, [4]数值, [5]问答 : ");
        igetstr(genbuf,2);
        genbuf[0] -= '0' ;
        if(genbuf[0] < 1 || genbuf[0] > 5)
            continue;
        ball.type = (int)genbuf[0] ;
        break;
    }
    gotoxy(1,6);
    printf("票箱上标题\n\r");
    igetstr(ball.title, 78);
    sprintf(genbuf, "%s//.MENU", currentboard);
    ball.qid = get_num_records(genbuf, sizeof(struct vote_class)) ;
    switch(ball.type) {
    case Vote_YesNo :
        ball.constrain = 0;
        break;
    case Vote_Single:
        OpenMulti(&ball);
        ball.constrain = 1;
        break;
    case Vote_Multi:
        OpenMulti(&ball) ;
        for(;;) {
            prompt("一个人最多几票[1] : ",genbuf,6);
            ball.constrain = atof(genbuf);
            if(ball.constrain == 0) ball.constrain = 1;
            gotoxy(1,21);
            printf("一个人有 %d 张票 !",ball.constrain);
            if(mksure("确定吗(Y/N) ? [N] : "))
                break;
        }
        break;
    case Vote_Value:
        for(;;) {
            gotoxy(1,8);
            printf("输入数值最大不得超过 [100] : ");
            clreol();
            igetstr(genbuf,10);
            ball.constrain = atof(genbuf);
            if(ball.constrain == 0) ball.constrain = 100;
            gotoxy(1,10);
            printf("数值介於 0 与 %d 之间 !",ball.constrain);
            if(mksure("确定吗(Y/N) ? [N] : "))
                break;
        }
        break;
    case Vote_Asking:
        gotoxy(1,8);
        printf("此问答题作答行数之限制 :") ;
        igetstr(genbuf,10);
        ball.constrain = atof(genbuf) ;
        if(ball.constrain == 0) ball.constrain = 10;
        break;
defaults:
        ball.constrain = 1;
        break;
    }
    sprintf(genbuf, "%s//.MENU", currentboard);
    ball.IsAns = NA ;
    append_record(genbuf, &ball, sizeof(ball) );
    gotoxy(1,22);
    printf("\033[0;32m新增选票箱\033[0m");
    gotoxy(1,23);
    pause();
    return 0;
}

dovotelist(vote_cache, ptr, lastptr, max_vote, print_type)
struct vote_class *vote_cache ;
int ptr, lastptr, max_vote, print_type;
{
    int i, num_rec, page, startline, endline;
    struct stat statb;
    struct vote_class *ball;

    page = ptr/Limit_Pageline ;
    startline = ptr % Limit_Pageline;
    endline = (page+1)*Limit_Pageline;
    if(endline > max_vote) endline = max_vote;
    else endline--;
    if(page != lastptr/Limit_Pageline || lastptr < 1) {
        clrscr();
        gotoxy(1,1);
        printf("\033[1;32m多重投票选单\n\r");
        printf("\033[1;35m[↑/↓] 上/下一题  [→] 查看说明  [←]离开或结束投票\n\r");
        printf("[v] 进入选项或开始作答\033[0m");
        gotoxy(1,Limit_title);
        printf("\033[1;37m  题号 已/未 %s   标    题.............................\033[0m",print_type?"属性":"    ");
        gotoxy(1,Limit_title);
        for(i=page*Limit_Pageline;i<endline+((page==0)?0:1);i++) {
            ball = &vote_cache[((page==0)? i:i-1)];
            printf("\n\r%c %3d  %s  %s   %s",
                   (((page==0)?i+1:i)==ptr) ? '>': ' ', ball->qid+1,
                   ball->IsAns ? "已投":"未投",print_type?choice_type[ball->type-1]:"",
                   ball->title);
        }
    }
    else {
        gotoxy(1,Limit_title + lastptr%Limit_Pageline + ((page==0)?0:1) ) ;
        printf(" ") ;
        gotoxy(1,Limit_title + startline + ((page==0)?0:1) );
        printf(">") ;
    }
    if(CanOpen == YEA) {
        gotoxy(1,23);
        printf("\033[1;31m[ctrl+p] 增加票箱或选项, [i] 编辑说明, [→] 查看说明, [←]离开选项设定 \033[0m");
    }
}

enter_vote(ball)
struct vote_class *ball;
{
    int num,i;

    switch(ball->type) {
    case Vote_YesNo :
        vote_yn(ball->qid);
        ball->IsAns = YEA;
        break;
    case Vote_Single:
        vote_multi(ball);
        break;
    case Vote_Multi :
        vote_multi(ball);
        break;
    case Vote_Value:
        vote_value(ball) ;
        break;
    case Vote_Asking:
        vote_asking(ball) ;
        break;
defautls :
        break;
    }
    return FULLUPDATE ;
}

DoMenu()
{
    int ch, lastptr, ptr, mode;

    /*        sprintf(genbuf, "%s//.MENU", currentboard);
            num_rec = get_num_records(genbuf, sizeof(struct vote_class)); */
    dovotelist(vote, 1, -1, choice_num,1);
    ptr = 1;
    for(mode=PARTUPDATE;;mode=PARTUPDATE) {
        ch = igetkey();
        lastptr = ptr ;
        switch( ch ) {
        case Ctrl('Z'): r_lastmsg(); /* Leeward 98.07.30 support msgX */
            break;
        case KEY_UP :
            if(ptr <= 1)
                ptr = choice_num;
            else
                ptr--;
            break;
        case KEY_DOWN :
            if(ptr >= choice_num)
                ptr= 1;
            else
                ptr++;
            break;
        case 'v' :
            if(CanOpen == YEA && vote[ptr-1].type != (Vote_Single|Vote_Multi)) {
                gotoxy(1,20);
                printf("设定票所中, 无法投票!");
                pause();
                break;
            }
            if(vote[ptr-1].IsAns == YEA) break;
            mode = enter_vote(&vote[ptr-1]);
            break;
        case KEY_LEFT :
            if(mksure("确定吗? [N] : "))
                return 0;
            mode = FULLUPDATE ;
            break;
        case KEY_RIGHT :
            sprintf(genbuf, "%s//.instruct.%d", currentboard,ptr);
            if(more( genbuf )==-1) {
                gotoxy(1,23);
                printf("\033[1;37m此票箱没有任何说明!\033[0m\n\r");
                pause();
            }
            mode = FULLUPDATE;
            break;
        case 'i' :
            if(CanOpen == NA) {
                mode = DONOTHING;
                break;
            }
            sprintf(genbuf, "bin/ve %s//.instruct.%d",currentboard,ptr);
            system( genbuf );
            mode = FULLUPDATE ;
            break;
        case Ctrl('P'):
                if(CanOpen == NA) {
                    mode = DONOTHING;
                    break;
                }
            NewVote();
            relocate(1,".MENU");
            mode = FULLUPDATE;
            break;
        }
        if(mode == FULLUPDATE)
            dovotelist(vote, ptr,-1, choice_num, 1);
        else if(mode == PARTUPDATE)
            dovotelist(vote, ptr,lastptr,choice_num,1) ;
    }
}

/* search mark sch */

count_vote(ques)
int ques;
{
    FILE *fp1;
    int i, results;
    for(i=0;i<MAXITEMS;i++)
        vote_out[i] = 0;
    sprintf(genbuf, "%s//ballots.%d",currentboard, ques);
    if((fp1=fopen( genbuf, "r" )) == NULL)
        return 0;
    for(i=0;fscanf(fp1,"%d",&results) != EOF;i++)
        vote_out[results]++;
}

write_item(fps,ques)
FILE *fps;
int ques;
{
    int i;
    sprintf(genbuf, ".MENU.%d",ques);
    relocate(1,genbuf);
    count_vote(ques);
    for(i=0;i<choice_num;i++)
        fprintf( fps, "      %-60s %d\n",vote[i].title, vote_out[i]);
    return 0;
}

count_every(fps, filename)
FILE *fps;
char *filename;
{
    int i, j, count, average, tep;
    FILE *fp;

    relocate(1,filename);
    for(i=0;i<choice_num;i++) {
        fprintf(fps, "～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～\n");
        fprintf(fps, "  %-60s\n", vote[i].title);
        switch(vote[i].type) {
        case Vote_YesNo:
            count_vote(i);
            fprintf(fps, "       同  意 :  %d\n",vote_out[0]);
            fprintf(fps, "       不同意 :  %d\n",vote_out[1]);
            fprintf(fps, "       无意见 :  %d\n",vote_out[2]);
            break;
        case Vote_Single:
        case Vote_Multi:
            count_vote(i);
            write_item(fps, i);
            relocate(1,filename);
            break;
        case Vote_Value:
            sprintf(genbuf,"%s/ballots.%d",currentboard,i);
            count = 0;
            if((fp=fopen(genbuf,"r")) != NULL) {
                for(j=0;;j++) {
                    if(fscanf(fp,"%d",&tep)==EOF)
                        break;
                    count += tep;
                }
                average = count / j ;
                fclose(fp);
            }
            else break;
        case Vote_Asking:
            fprintf(fps,"---------------------------------------------------------------------------\n");
            sprintf(genbuf,"%s//ballots.%d",currentboard, i);
            suckinfile(fps, genbuf);
            fprintf(fps,"\n总点数 : %d",count);
            fprintf(fps,"\n总平均 : %d",average);
            fprintf(fps,"\n======================================ＥＮＤ===============================\n");
            break;
        }
    }
}

multi_close(endtime)
time_t endtime;
{
    FILE *fps,*fps1;
    int totalvote;
    char inbuf[1024];

    sprintf(genbuf, "%s//newresult",currentboard);
    if(fps=fopen(genbuf,"w")) {
        fprintf(fps,"＠＠＠＠＠＠＠＠多　重　投　票　器＠＠＠＠＠＠＠＠\n");
        fprintf(fps,"＾＾＾＾＾＾＾＾＾＾＾＾＾＾＾＾＾＾＾＾＾＾＾＾＾\n\n");
        sprintf(genbuf, "%s//desc",currentboard);
        suckinfile(fps, genbuf);
        fprintf(fps,"＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊\n");
        fprintf(fps,"本投票所关闭时间: %s\n",ctime(&endtime));
        sprintf(genbuf,"%s//voted",currentboard);
        if((fps1=fopen(genbuf,"r")) != NULL) {
            for(totalvote=0;;totalvote++)
                if(fgets(genbuf, 80, fps1) == NULL)
                    break;
        }
        else
            totalvote = 0;
        fprintf(fps,"本次投票共有 %d 人参与!\n\n", totalvote);
        count_every(fps,".MENU");
        fprintf(fps,"\n\n");
        fclose(fps);
        fclose(fps1);
        sprintf(genbuf, "mv %s//newresult %s//results",currentboard,currentboard);
        system(genbuf);
        sprintf(genbuf, "rm %s//control %s//desc %s//voted", currentboard, currentboard,currentboard);
        system(genbuf);
        sprintf(genbuf, "rm %s/ballots.* %s/.MENU* %s//.instru*", currentboard,currentboard,currentboard);
        system(genbuf);
        return 0;
    }
}

resultreport()
{
    FILE *fps;

    clrscr();
    sprintf(genbuf, "%s//results",currentboard);
    if((fps=fopen(genbuf,"r")) == NULL) {
        gotoxy(1,3);
        printf("\033[0;35m什麽! 投票结果? 这里没有所谓的投票结果, 会不会是你消息错误\033[0m");
        gotoxy(1,24);
        pause();
        return;
    }
    fclose(fps);
    gotoxy(1,3);
    printf("投票结果已经揭晓了!");
    gotoxy(1,23);
    pause();
    more(genbuf);
    return;
}

main( argc, argv )
char    *argv[];
{
    int i;
    FILE *fps;

    if( argc < 3 ) {
        printf("used: <command> userid board |open|") ;
        return;
    }

    strcpy(username, argv[1]);

    temp = NULL ;
    if((uname=getenv("USER")) == NULL) {
        printf("can't read username!\n\r");
        return;
    }

    init_tty();
    clrscr();

    sprintf(currentboard, "vote//%s", argv[2]);
    sprintf(genbuf, "%s//.MENU", currentboard);
    choice_num = get_num_records(genbuf, sizeof(struct vote_class));
    if(argc > 3 && !strcmp(argv[3],"open") ) {
        FILE *fps;
        sprintf(genbuf,"%s//control",currentboard);
        if((fps=fopen(genbuf,"r")) != NULL) {
            time_t time1;
            fgets(genbuf,80,fps);
            time1 = atof(genbuf);
            fclose(fps);
            clrscr();
            printf("票所使用中! 预计在 %s 结束。",ctime(&time1));
            gotoxy(1,23);
            pause();
            reset_tty();
            return;
        }
        CanOpen = YEA ;
        if(init_OpenVote()==-1)
            exit(0) ;
        NewVote();
        sprintf(genbuf,"%s//.MENU",currentboard);
        choice_num = get_num_records(genbuf, sizeof(struct vote_class));
    } else {
        sprintf(genbuf, "%s//control",currentboard) ;
        if((fps=fopen(genbuf,"r")) == NULL) {
            gotoxy(1,1);
            printf("多重投票器并未开启, 要玩下次吧!");
            gotoxy(1,20);
            printf("\033[0;32m要观看投票结果吗?\033[0m");
            if(mksure("不看不行(Y), 死也不看(N) : [N]"))
            {
                resultreport();
            }
            reset_tty();
            return;
        } else {
            time_t closetime;
            fgets(genbuf, 20, fps);
            fclose(fps);
            closetime = (time_t)atol(genbuf) ;
            if(closetime < time(0) && closetime > 0) {
                multi_close(closetime);
                resultreport();
                reset_tty();
                return 0;
            }
        }
        sprintf(genbuf, "%s//desc",currentboard) ;
        more(genbuf) ;
        sprintf(genbuf, "%s//voted", currentboard);
        if((fps=fopen(genbuf,"r")) != NULL)
            while(fscanf(fps,"%s",genbuf) != EOF)
                if(!strcmp(username,genbuf)) {
                    clrscr();
                    gotoxy(1,1);
                    printf("你已经投完票了!");
                    gotoxy(1,23);
                    pause();
                    reset_tty();
                    return ;
                }
        CanOpen = NA ;
    }

    vote = (struct vote_class*)malloc(choice_num*sizeof(struct vote_class)) ;
    for(i=0;i<choice_num;i++) {
        get_record(genbuf, &vote[i], sizeof(struct vote_class),i+1) ;
    }
    relocate(0, ".MENU");

    DoMenu();
    if(CanOpen == YEA) {
        sprintf(genbuf,"mv %s//prepare %s//control",currentboard, currentboard);
        system(genbuf);
        clrscr();
        gotoxy(1,1);
        printf("投票所开启!");
        gotoxy(1,23);
        pause();
    } else
    {
        for(i=0;i<choice_num;i++)
            if(vote[i].IsAns == YEA) break;
        if(i<choice_num) {
            FILE *fps;
            sprintf(genbuf, "%s//voted",currentboard);
            if((fps=fopen(genbuf,"a+")) == NULL){
                reset_tty() ;
                return;
            }
            fprintf(fps, "%s\n", username);
            fclose(fps);
        }
    }
    gotoxy( 1, 24 );
    reset_tty();
}
