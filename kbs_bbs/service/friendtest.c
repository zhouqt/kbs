#include "service.h"
#include "bbs.h"

#define TOTALNUM 20
#define FRIENDTOP ".FRIENDTOP"
#define BADLIST ".FRIENDBADLIST"

struct usertype {
    char userid[IDLEN+2];
    int tried;
    int got;
    int create;
} * users;
int userst=0;

struct questype {
    char topic[80];
    char quest[5][40];
    int value[5];
} * quests;
int questst=0;

typedef char statinfo[35];
statinfo* stats;
int statst=0;

struct userec* lookuser;

void sethome(char* s, char* id)
{
    sprintf(s, "home/%c/%s/.FRIENDTEST", toupper(id[0]), id);
}

void sethometop(char* s, char* id)
{
    sprintf(s, "home/%c/%s/" FRIENDTOP, toupper(id[0]), id);
}

void sethomestat(char* s, char* id)
{
    sprintf(s, "home/%c/%s/.FRIENDSTAT", toupper(id[0]), id);
}

void load_users(char* filename)
{
    int fd;
    struct flock ldata;
    if (userst) free(users);
    userst=0;
    if ((fd = open(filename, O_RDONLY, 0644))!=-1) {
        ldata.l_type=F_RDLCK;
        ldata.l_whence=0;
        ldata.l_len=0;
        ldata.l_start=0;
        if (fcntl(fd, F_SETLKW, &ldata)!=-1) {
            read(fd, &userst, sizeof(userst));
            users=(struct usertype*)malloc(sizeof(struct usertype)*(userst+1));
            read(fd, users, sizeof(struct usertype)*userst);

            ldata.l_type = F_UNLCK;
            fcntl(fd, F_SETLKW, &ldata);
        }
        close(fd);
    }
}

void save_users(char* filename)
{
    int fd;
    struct flock ldata;
    if ((fd = open(filename, O_WRONLY|O_CREAT, 0644))!=-1) {
        ldata.l_type=F_WRLCK;
        ldata.l_whence=0;
        ldata.l_len=0;
        ldata.l_start=0;
        if (fcntl(fd, F_SETLKW, &ldata)!=-1) {
            write(fd, &userst, sizeof(userst));
            write(fd, users, sizeof(struct usertype)*userst);

            ldata.l_type = F_UNLCK;
            fcntl(fd, F_SETLKW, &ldata);
        }
        close(fd);
    }
}

void load_quests(char* filename)
{
    int fd;
    struct flock ldata;
    if (questst) free(quests);
    questst=0;
    if ((fd = open(filename, O_RDONLY, 0644))!=-1) {
        ldata.l_type=F_RDLCK;
        ldata.l_whence=0;
        ldata.l_len=0;
        ldata.l_start=0;
        if (fcntl(fd, F_SETLKW, &ldata)!=-1) {
            read(fd, &questst, sizeof(questst));
            quests=(struct questype*)malloc(sizeof(struct questype)*TOTALNUM);
            read(fd, quests, sizeof(struct questype)*questst);

            ldata.l_type = F_UNLCK;
            fcntl(fd, F_SETLKW, &ldata);
        }
        close(fd);
    }
}

void save_quests(char* filename)
{
    int fd;
    struct flock ldata;
    if ((fd = open(filename, O_WRONLY|O_CREAT, 0644))!=-1) {
        ldata.l_type=F_WRLCK;
        ldata.l_whence=0;
        ldata.l_len=0;
        ldata.l_start=0;
        if (fcntl(fd, F_SETLKW, &ldata)!=-1) {
            write(fd, &questst, sizeof(questst));
            write(fd, quests, sizeof(struct questype)*questst);

            ldata.l_type = F_UNLCK;
            fcntl(fd, F_SETLKW, &ldata);
        }
        close(fd);
    }
}

void check_stats(char* filename)
{
    int fd, i, saveagain=0;
    struct flock ldata;
    struct stat st;
    if ((fd = open(filename, O_RDONLY, 0644))!=-1) {
        ldata.l_type=F_RDLCK;
        ldata.l_whence=0;
        ldata.l_len=0;
        ldata.l_start=0;
        if (fcntl(fd, F_SETLKW, &ldata)!=-1) {
            read(fd, &i, sizeof(i));
            if (i!=0x8080) {
                if (statst) free(stats);
                saveagain = 1;
                lseek(fd, 0, SEEK_SET);
                fstat(fd, &st);
                statst = st.st_size/20;
                stats = malloc(statst*sizeof(statinfo));
                for (i=0;i<statst;i++) {
                    read(fd, stats[i], 20);
                    strcpy(stats[i]+21,"unknown");
                }

                ldata.l_type = F_UNLCK;
                fcntl(fd, F_SETLKW, &ldata);
            }
        }
        close(fd);
    }
    if (saveagain) {
        if ((fd = open(filename, O_WRONLY|O_CREAT, 0644))!=-1) {
            ldata.l_type=F_WRLCK;
            ldata.l_whence=0;
            ldata.l_len=0;
            ldata.l_start=0;
            if (fcntl(fd, F_SETLKW, &ldata)!=-1) {
                i=0x8080;
                write(fd, &i, sizeof(i));
                write(fd, stats, statst*sizeof(statinfo));

                ldata.l_type = F_UNLCK;
                fcntl(fd, F_SETLKW, &ldata);
            }
            close(fd);
        }
    }
}

void save_stats(char* filename, statinfo a)
{
    int fd, i;
    struct flock ldata;
    check_stats(filename);
    if ((fd = open(filename, O_WRONLY|O_CREAT, 0644))!=-1) {
        ldata.l_type=F_WRLCK;
        ldata.l_whence=0;
        ldata.l_len=0;
        ldata.l_start=0;
        if (fcntl(fd, F_SETLKW, &ldata)!=-1) {
            lseek(fd, 0, SEEK_SET);
            i=0x8080;
            write(fd, &i, sizeof(i));
            lseek(fd, 0, SEEK_END);
            write(fd, a, sizeof(statinfo));

            ldata.l_type = F_UNLCK;
            fcntl(fd, F_SETLKW, &ldata);
        }
        close(fd);
    }
}

void load_stats(char* filename)
{
    int fd;
    struct flock ldata;
    struct stat st;
    int i;
    check_stats(filename);
    if (statst) free(stats);
    statst=0;
    if ((fd = open(filename, O_RDONLY, 0644))!=-1) {
        ldata.l_type=F_RDLCK;
        ldata.l_whence=0;
        ldata.l_len=0;
        ldata.l_start=0;
        if (fcntl(fd, F_SETLKW, &ldata)!=-1) {
            fstat(fd, &st);
            statst = (st.st_size-4)/sizeof(statinfo);
            stats = malloc(statst*sizeof(statinfo));
            read(fd, &i, sizeof(i));
            read(fd, stats, statst*sizeof(statinfo));

            ldata.l_type = F_UNLCK;
            fcntl(fd, F_SETLKW, &ldata);
        }
        close(fd);
    }
}

void sort_users()
{
    int i,j;
    struct usertype tmp;
    for (i=0;i<userst;i++)
        for (j=i+1;j<userst;j++)
            if (users[i].tried<users[j].tried) {
                memcpy(&tmp,&users[i],sizeof(tmp));
                memcpy(&users[i],&users[j],sizeof(tmp));
                memcpy(&users[j],&tmp,sizeof(tmp));
            }
}

void init()
{
}

void done()
{
    if (userst) free(users);
    if (questst) free(quests);
    if (statst) free(stats);
}

int usermenu()
{
    int i,tuid;
    char buf[IDLEN+2], direct[PATHLEN];
    struct stat st;
    clear();
    load_users(FRIENDTOP);
    sort_users();
    move(2,20);
    setfcolor(WHITE,0);
    prints("欢迎来到友谊测试中心(");
    setfcolor(RED,1);
    prints("FRIENDTEST");
    setfcolor(WHITE,0);
    prints(")1.0   (题库:%d)", userst);
    move(3,40);
    prints("作者: ");
    setfcolor(RED,0);
    prints("bad@smth.org");
    move(5,13);
    setfcolor(WHITE,1);
    prints("===人气最旺指数排行===");
    move(6,15);
    setfcolor(RED,1);
    prints("╭───────╮");
    for (i=0;i<10;i++) {
        move(7+i, 15);
        setfcolor(RED,1);
        prints("│");
        if (i>=userst) {
            move(7+i, 20);
            setfcolor(GREEN,1);
            prints("--空--");
        } else {
            move(7+i, 23-strlen(users[i].userid)/2);
            setfcolor(GREEN,1);
            prints(users[i].userid);
        }
        move(7+i, 31);
        setfcolor(RED,1);
        prints("│");
    }
    move(17,15);
    setfcolor(RED,1);
    prints("╰───────╯");

    load_users(FRIENDTOP);
    move(5,53-7);
    setfcolor(WHITE,1);
    prints("===最新加入用户列表===");
    move(6,55-7);
    setfcolor(RED,1);
    prints("╭───────╮");
    for (i=0;i<10;i++) {
        move(7+i, 55-7);
        setfcolor(RED,1);
        prints("│");
        if (userst-i-1<0) {
            move(7+i, 60-7);
            setfcolor(GREEN,1);
            prints("--空--");
        } else {
            move(7+i, 63-strlen(users[userst-i-1].userid)/2-7);
            setfcolor(GREEN,1);
            prints(users[userst-i-1].userid);
        }
        move(7+i, 71-7);
        setfcolor(RED,1);
        prints("│");
    }
    move(17,55-7);
    setfcolor(RED,1);
    prints("╰───────╯");
    setfcolor(WHITE,0);

    move(19,22);
    prints("创建["); setfcolor(GREEN,1); prints("a"); setfcolor(WHITE,0); prints("] ");
    prints("统计结果["); setfcolor(GREEN,1); prints("s"); setfcolor(WHITE,0); prints("] ");
    prints("管理题["); setfcolor(GREEN,1); prints("m"); setfcolor(WHITE,0); prints("] ");
    prints("注册列表["); setfcolor(GREEN,1); prints("l"); setfcolor(WHITE,0); prints("] ");
    if (HAS_PERM(getCurrentUser(), PERM_SYSOP)) {
        prints("SYSOP["); setfcolor(GREEN,1); prints("d"); setfcolor(WHITE,0); prints("] ");
    }
    move(20,22);
    prints("要做别人的题,请输入他的id");
//    setfcolor(WHITE,1);
    while (1) {
        move(22,20); clrtoeol();
        getdata(22,26,"请选择:",buf,IDLEN+1,1,NULL,true);
        if (toupper(buf[0])=='A'&&!buf[1])
            return 1;
        if (toupper(buf[0])=='S'&&!buf[1])
            return 3;
        if (toupper(buf[0])=='M'&&!buf[1])
            return 4;
        if (toupper(buf[0])=='L'&&!buf[1])
            return 5;
        if (HAS_PERM(getCurrentUser(), PERM_SYSOP)&&toupper(buf[0])=='D'&&!buf[1]) {
            return 6;
        } else if (buf[0]) {
            tuid = getuser(buf, &lookuser);
            if (tuid) {
                sethome(direct, lookuser->userid);
                if (stat(direct, &st) != -1)
                    return 2;
                else {
                    move(23,20);
                    clrtoeol();
                    prints("使用者%s尚未创建题目!", lookuser->userid);
                }
            } else {
                move(23,20);
                clrtoeol();
                prints("错误的使用者id!");
            }
        } else return 0;
    }
}

int possible_high()
{
    int i,j,k=0,l;
    for (i=0;i<questst;i++) {
        l=-10;
        for (j=0;j<5;j++) {
            if (!quests[i].quest[j][0]) break;
            if (quests[i].value[j]>l)
                l=quests[i].value[j];
        }
        k+=l;
    }
    return k;
}

void new_friendtest_reset()
{
    resetcolor();
    clear();
    move(0,0);
    setfcolor(YELLOW,1);
    setbcolor(BLUE);
    prints("   FRIENDTEST      新建用户 %s                                                  ", getCurrentUser()->userid);
    resetcolor();
    prints("\n");
}

void new_friendtest()
{
    char direct[PATHLEN];
    int i, j, k;
    struct stat st;
    char ans[3], buf[122], buf2[10];
    int empty=1;
    new_friendtest_reset();

    load_users(BADLIST);
    for (i=0;i<userst;i++)
        if (!strcmp(users[i].userid,getCurrentUser()->userid)&&users[i].tried&1) {
            prints("\n你已经被封禁了出题权限\n");
            pressreturn();
            done();
            return;
        }

    sethome(direct, getCurrentUser()->userid);
    if (stat(direct, &st) != -1) {
        empty=0;
        getdata(1,0,"已经存在你的账号，是否继续? [y/N] ", ans, 2, 1, NULL, true);
        if (toupper(ans[0])!='Y') return;
    }
    getdata(1,0,"请输入题数[1--20]", buf, 3, true, NULL, true);
    questst=atoi(buf);
    if (questst<1||questst>TOTALNUM) {
        questst=0;
        return;
    }
    quests=(struct questype*)malloc(sizeof(struct questype)*questst);
    for (i=0;i<questst;i++) {
again:
        new_friendtest_reset();
        prints("第%d题(共%d题)\n", i+1, questst);
        getdata(2,0,"请输入题目(40个汉字内): ", quests[i].topic, 80, true, NULL, true);
        if (!quests[i].topic[0]) {
            new_friendtest_reset();
            getdata(1,0,"退出建立账号? [Y-退出/N-继续/E-不再输入新题]: ", ans, 2, true, NULL, true);
            if (toupper(ans[0])=='Y') return;
            if (toupper(ans[0])=='E'&&i!=0) {
                questst=i;
                break;
            }
            goto again;
        }
        for (j=0;j<5;j++) quests[i].quest[j][0]=0;
        do {
            move(3,0); clrtoeol();
            getdata(3,0,"请输入选项个数[1--5]: ", buf, 2, true, NULL, true);
            j=atoi(buf);
        } while (j<1||j>5);
        for (k=0;k<j;k++) {
            do {
                move(4+k*2,0); clrtoeol();
                sprintf(buf, "请输入第%d个选择内容:", k+1);
                getdata(4+k*2,0,buf,quests[i].quest[k],40,true,NULL,true);
            } while (!quests[i].quest[k][0]);
            do {
                move(5+k*2,0); clrtoeol();
                sprintf(buf, "请输入第%d个选择分值[-10--10]:", k+1);
                getdata(5+k*2,0,buf,buf2,4,true,NULL,true);
                quests[i].value[k]=atoi(buf2);
            } while (quests[i].value[k]<-10||quests[i].value[k]>10);
        }
    }
    new_friendtest_reset();
    move(1,0);
    prints("你的测试共有%d项，你的测试可能得到的最高分是%d", questst, possible_high());
    getdata(2,0,"以上输入是否正确，选择否取消，选择是建立[Y/n]", ans, 2, true, NULL, true);
    if (toupper(ans[0])=='N') return;
    save_quests(direct);
    load_users(FRIENDTOP);
    j=1;
    for (i=0;i<userst;i++)
        if (!strcmp(users[i].userid, getCurrentUser()->userid)) {
            users[i].tried = 0;
            users[i].got = 0;
            users[i].create = time(0);
            j=0;
            break;
        }
    if (j) {
        if (!userst)
            users=(struct usertype*)malloc(sizeof(struct usertype)*(userst+1));
        strcpy(users[userst].userid, getCurrentUser()->userid);
        users[userst].tried = 0;
        users[userst].got = 0;
        users[userst].create = time(0);
        userst++;
    }
    save_users(FRIENDTOP);
    sethometop(direct, getCurrentUser()->userid);
    unlink(direct);
    sethomestat(direct, getCurrentUser()->userid);
    unlink(direct);
    move(3,0);
    prints("恭喜你，你已经创建了账号，赶快邀请你的好友来参加吧");
    pressreturn();
}

void do_test_reset()
{
    resetcolor();
    clear();
    move(0,0);
    setfcolor(YELLOW,1);
    setbcolor(BLUE);
    prints("   FRIENDTEST      友谊竞赛 %s <--> %s                                           ", getCurrentUser()->userid, lookuser->userid);
    resetcolor();
    prints("\n");
}

void do_test()
{
    char direct[PATHLEN];
    struct stat st;
    int i, j, k, l;
    char ans[3], buf[122];
    statinfo answer;
    time_t span;

    do_test_reset();

    sethome(direct, lookuser->userid);
    if (stat(direct, &st) == -1) {
        move(1,0);
        prints("该账号不存在!");
        pressreturn();
        return;
    }
    load_quests(direct);
    sethometop(direct, lookuser->userid);
    load_users(direct);
    sort_users();
    move(2,0);
    prints("欢迎进入 %s 的友谊测试\n", lookuser->userid);
    prints("本测试最高可能得分为%d\n", possible_high());
    if (userst) {
        prints("已经有 %d 人参加过测试\n", userst);
        prints("其中最高分 %d ,  最低分 %d\n", users[0].tried, users[userst-1].tried);
        prints("\n=======TOP 10========\n");
        for (i=0;i<10;i++)
            if (i<userst)
                prints(" %-12s %-3d\n", users[i].userid, users[i].tried);
        getdata(22,0,"按d显示详细结果: ", ans, 2, true, NULL, true);
        if (toupper(ans[0])=='D') {
            do_test_reset();
            move(1,0);
            prints("详细测试结果\n");
            move(3,0);
            prints(" 账号         分数      测试次数");
            j=4;
            for (i=0;i<userst;i++) {
                move(j,0);
                prints(" %-12s %-3d        %-3d", users[i].userid, users[i].tried, users[i].got);
                if (j>=20&&i<userst-1) {
                    getdata(22,0,"按\033[32;1mq\033[m退出:",ans,2,true,NULL,true);
                    if (toupper(ans[0])=='Q') break;
                    do_test_reset();
                    move(1,0);
                    prints("详细测试结果\n");
                    move(3,0);
                    prints(" 账号         分数      测试次数");
                    j=3;
                }
                j++;
            }
            pressreturn();
            do_test_reset();
        }
    } else {
        pressreturn();
        do_test_reset();
    }
    for (i=0;i<userst;i++)
        if (!strcmp(users[i].userid, getCurrentUser()->userid)) {
            span = time(0)-users[i].create;
            if (span<3600) {
                move(2,0);
                prints("你在一小时内刚做过 %s 的友谊测试\n", lookuser->userid);
                prints("请稍微休息一会儿再接着做\n");
                pressreturn();
                return;
            }
        }

    k=0;
    for (i=0;i<questst;i++) {
doitagain:
        do_test_reset();
        move(1,0);
        prints("第%d题(共%d题)\n\n", i+1, questst);
        prints("%s\n", quests[i].topic);
        for (j=0;j<5;j++) {
            if (!quests[i].quest[j][0]) break;
            prints("%d) %s\n", j+1, quests[i].quest[j]);
        }
        sprintf(buf, "选择[1--%d]:", j);
        do {
            move(5+j,0);
            clrtoeol();
            getdata(5+j, 0, buf, ans, 2, true, NULL, true);
            l=atoi(ans);
            if (!l) {
                do_test_reset();
                getdata(1,0,"是否退出测试?[y/N]", ans, 2, true, NULL, true);
                if (toupper(ans[0])=='Y') return;
                goto doitagain;
            }
        } while (l<1||l>j);
        answer[i]=l;
        k+=quests[i].value[l-1];
    }
    load_users(direct);
    j=1;
    for (i=0;i<userst;i++)
        if (!strcmp(users[i].userid, getCurrentUser()->userid)) {
            if (k>users[i].tried)
                users[i].tried = k;
            users[i].got++;
            users[i].create = time(0);
            j=0;
            break;
        }
    if (j) {
        if (!userst)
            users=(struct usertype*)malloc(sizeof(struct usertype)*(userst+1));
        strcpy(users[userst].userid, getCurrentUser()->userid);
        users[userst].tried = k;
        users[userst].got = 1;
        users[userst].create = time(0);
        userst++;
    }
    save_users(direct);

    if (j) {
        load_users(FRIENDTOP);
        for (i=0;i<userst;i++)
            if (!strcmp(users[i].userid, lookuser->userid)) {
                users[i].tried++;
                break;
            }
        save_users(FRIENDTOP);
    }
    sethomestat(direct, lookuser->userid);
    strcpy(answer+21, getCurrentUser()->userid);
    save_stats(direct, answer);

    do_test_reset();
    move(2,0);
    prints("你已经结束了 %s 的友谊测试\n", lookuser->userid);
    prints("本测试最高可能得分为%d\n", possible_high());
    prints("你的得分为%d\n", k);
    pressreturn();
}

void show_stat_reset()
{
    resetcolor();
    clear();
    move(0,0);
    setfcolor(YELLOW,1);
    setbcolor(BLUE);
    prints("   FRIENDTEST      统计结果 %s                                                 ", getCurrentUser()->userid);
    resetcolor();
    prints("\n");
}

void show_stat()
{
    char direct[PATHLEN];
    struct stat st;
    int i, j, k, l, m;
    char ans[3];

    do_test_reset();

    sethome(direct, getCurrentUser()->userid);
    if (stat(direct, &st) == -1) {
        move(1,0);
        prints("你还没有建立账号!");
        pressreturn();
        return;
    }
    load_quests(direct);
    sethometop(direct, getCurrentUser()->userid);
    load_users(direct);
    sort_users();
    sethomestat(direct, getCurrentUser()->userid);
    load_stats(direct);

    move(2,0);
    j=0;
    for (i=0;i<userst;i++)
        j+=users[i].tried;
    if (userst) j/=userst;
    prints("统计信息:\n共 %d 人做过你的测试\n平均分: %d\n\n按回车键开始详细资料",
           userst, j);
    pressreturn();

    for (i=0;i<questst;i++) {
        do_test_reset();
        move(1,0);
        prints("第%d题(共%d题)\n\n", i+1, questst);
        prints("%s\n", quests[i].topic);
        for (j=0;j<5;j++)
            if (!quests[i].quest[j][0]) break;
        for (k=0;k<j;k++) {
            m=0;
            for (l=0;l<statst;l++)
                if (stats[l][i]==k+1) m++;
            prints("%d) %s (分值:%d  选择人次:%d)\n", k+1, quests[i].quest[k], quests[i].value[k], m);
        }
        getdata(10,0,"按q退出:",ans,2,true,NULL,true);
        if (toupper(ans[0])=='Q') break;
    }
}

void admin_reset()
{
    resetcolor();
    clear();
    move(0,0);
    setfcolor(YELLOW,1);
    setbcolor(BLUE);
    prints("   FRIENDTEST      管理选单 %s                                                 ", getCurrentUser()->userid);
    resetcolor();
    prints("\n");
}

void admin_stat()
{
    char direct[PATHLEN];
    struct stat st;
    int i, j, k, l, m;
    char ans[3], buf[122], buf2[10];

    sethome(direct, getCurrentUser()->userid);
    if (stat(direct, &st) == -1) {
        move(1,0);
        prints("你还没有建立账号!");
        pressreturn();
        return;
    }
    load_quests(direct);
    sethometop(direct, getCurrentUser()->userid);
    load_users(direct);
    sort_users();

    while (1) {
        admin_reset();

        move(2,0);
        j=0;
        for (i=0;i<userst;i++)
            j+=users[i].tried;
        if (userst) j/=userst;
        prints("统计信息:\n共 %d 人做过你的测试\n平均分: %d",
               userst, j);

        getdata(7,0,"选择[q-退出,d-删除,e-修改,a-添加,r-排行重置,s-察看,l-列表] ",ans,2,true,NULL,true);
        switch (toupper(ans[0])) {
            case 'Q': return;
            case 'L':
                sethomestat(direct, getCurrentUser()->userid);
                load_stats(direct);
                admin_reset();
                move(1,0);
                prints("用户            答案列表\n");
                prints("答案            ");
                for (j=0;j<questst;j++) {
                    l=0;
                    for (k=0;k<5;k++) {
                        if (!quests[j].quest[k][0]) break;
                        if (quests[j].value[k]>quests[j].value[l])
                            l=k;
                    }
                    prints("%d ", l+1);
                }
                k=3;
                for (i=0;i<statst;i++) {
                    move(k,0);
                    prints("%-16s", stats[i]+21);
                    for (j=0;j<questst;j++)
                        prints("%d ", stats[i][j]);
                    k++;
                    if (k>20&&i<statst-1) {
                        getdata(22,0,"按\033[32;1mq\033[m退出:",ans,2,true,NULL,true);
                        if (toupper(ans[0])=='Q') break;
                        admin_reset();
                        move(1,0);
                        prints("用户            答案列表\n");
                        prints("答案            ");
                        for (j=0;j<questst;j++) {
                            l=0;
                            for (k=0;k<5;k++) {
                                if (!quests[j].quest[k][0]) break;
                                if (quests[j].value[k]>quests[j].value[l])
                                    l=k;
                            }
                            prints("%d ", l+1);
                        }
                        k=3;
                    }
                }
                pressreturn();
                break;
            case 'D':
                sprintf(buf,"输入删除项(1--%d, all全删):",questst);
                do {
                    move(8,0); clrtoeol();
                    getdata(8,0,buf,buf2,4,true,NULL,true);
                    i=atoi(buf2);
                } while ((i<1||i>questst)&&strcasecmp(buf2,"all"));
                if (!strcasecmp(buf2,"all")) {
                    free(quests);
                    questst=0;
                } else {
                    i--;
                    for (j=i;j<questst-1;j++)
                        memcpy(&quests[j],&quests[j+1],sizeof(quests[j]));
                    questst--;
                    if (!questst) free(quests);
                }
                sethome(direct, getCurrentUser()->userid);
                if (!questst) {
                    unlink(direct);
                    return;
                } else save_quests(direct);
                break;
            case 'A':
                i=questst;
                if (i>=TOTALNUM) {
                    move(8,0);
                    prints("题目不能超过20个!\n");
                    pressreturn();
                    break;
                }
againa:
                new_friendtest_reset();
                prints("第%d题(共%d题)\n", i+1, questst+1);
                getdata(2,0,"请输入题目(40个汉字内): ", quests[i].topic, 80, true, NULL, true);
                if (!quests[i].topic[0]) {
                    new_friendtest_reset();
                    getdata(1,0,"取消? [Y-退出/N-继续]: ", ans, 2, true, NULL, true);
                    if (toupper(ans[0])=='Y') break;
                    goto againa;
                }
                for (j=0;j<5;j++) quests[i].quest[j][0]=0;
                do {
                    move(3,0); clrtoeol();
                    getdata(3,0,"请输入选项个数[1--5]: ", buf, 2, true, NULL, true);
                    j=atoi(buf);
                } while (j<1||j>5);
                for (k=0;k<j;k++) {
                    do {
                        move(4+k*2,0); clrtoeol();
                        sprintf(buf, "请输入第%d个选择内容:", k+1);
                        getdata(4+k*2,0,buf,quests[i].quest[k],40,true,NULL,true);
                    } while (!quests[i].quest[k][0]);
                    do {
                        move(5+k*2,0); clrtoeol();
                        sprintf(buf, "请输入第%d个选择分值[-10--10]:", k+1);
                        getdata(5+k*2,0,buf,buf2,4,true,NULL,true);
                        quests[i].value[k]=atoi(buf2);
                    } while (quests[i].value[k]<-10||quests[i].value[k]>10);
                }
                questst++;
                sethome(direct, getCurrentUser()->userid);
                save_quests(direct);
                break;

            case 'E':
                sprintf(buf,"输入编辑项(1--%d):",questst);
                do {
                    move(8,0); clrtoeol();
                    getdata(8,0,buf,buf2,4,true,NULL,true);
                    i=atoi(buf2);
                } while ((i<1||i>questst));
                i--;
againb:
                new_friendtest_reset();
                prints("第%d题(共%d题)\n", i+1, questst);
                getdata(2,0,"请输入题目(40个汉字内): ", quests[i].topic, 80, true, NULL, false);
                if (!quests[i].topic[0]) {
                    new_friendtest_reset();
                    getdata(1,0,"取消? [Y-退出/N-继续]: ", ans, 2, true, NULL, true);
                    if (toupper(ans[0])=='Y') break;
                    goto againb;
                }
                for (k=0;k<5;k++) if (!quests[i].quest[k][0]) break;
                do {
                    move(3,0); clrtoeol();
                    sprintf(buf, "%d", k);
                    getdata(3,0,"请输入选项个数[1--5]: ", buf, 2, true, NULL, false);
                    j=atoi(buf);
                } while (j<1||j>5);
                if (j<5) quests[i].quest[j][0]=0;
                for (k=0;k<j;k++) {
                    do {
                        move(4+k*2,0); clrtoeol();
                        sprintf(buf, "请输入第%d个选择内容:", k+1);
                        getdata(4+k*2,0,buf,quests[i].quest[k],40,true,NULL,false);
                    } while (!quests[i].quest[k][0]);
                    do {
                        move(5+k*2,0); clrtoeol();
                        sprintf(buf, "请输入第%d个选择分值[-10--10]:", k+1);
                        sprintf(buf2,"%d",quests[i].value[k]);
                        getdata(5+k*2,0,buf,buf2,4,true,NULL,false);
                        quests[i].value[k]=atoi(buf2);
                    } while (quests[i].value[k]<-10||quests[i].value[k]>10);
                }
                sethome(direct, getCurrentUser()->userid);
                save_quests(direct);
                break;
            case 'R':
                getdata(8,0,"一旦重置之后，排行榜将被抹去，确认吗(y/N)",ans,2,true,NULL,true);
                if (toupper(ans[0])!='Y') break;
                load_users(FRIENDTOP);
                j=1;
                for (i=0;i<userst;i++)
                    if (!strcmp(users[i].userid, getCurrentUser()->userid)) {
                        users[i].tried = 0;
                        users[i].got = 0;
                        users[i].create = time(0);
                        j=0;
                        break;
                    }
                if (j) {
                    if (!userst)
                        users=(struct usertype*)malloc(sizeof(struct usertype)*(userst+1));
                    strcpy(users[userst].userid, getCurrentUser()->userid);
                    users[userst].tried = 0;
                    users[userst].got = 0;
                    users[userst].create = time(0);
                    userst++;
                }
                save_users(FRIENDTOP);
                sethometop(direct, getCurrentUser()->userid);
                unlink(direct);
                sethomestat(direct, getCurrentUser()->userid);
                unlink(direct);
                move(10,0);
                prints("题目已经被重置!\n");
                pressreturn();
                break;
            case 'S':
                for (i=0;i<questst;i++) {
                    do_test_reset();
                    move(1,0);
                    prints("第%d题(共%d题)\n\n", i+1, questst);
                    prints("%s\n", quests[i].topic);
                    for (j=0;j<5;j++)
                        if (!quests[i].quest[j][0]) break;
                    for (k=0;k<j;k++) {
                        m=0;
                        for (l=0;l<statst;l++)
                            if (stats[l][i]==k+1) m++;
                        prints("%d) %s (分值:%d  选择人次:%d)\n", k+1, quests[i].quest[k], quests[i].value[k], m);
                    }
                    getdata(10,0,"按q退出:",ans,2,true,NULL,true);
                    if (toupper(ans[0])=='Q') break;
                }
                break;
        }
    }
}

void sys_reset()
{
    resetcolor();
    clear();
    move(0,0);
    setfcolor(YELLOW,1);
    setbcolor(BLUE);
    prints("   FRIENDTEST      用户列表 %s                                                 ", getCurrentUser()->userid);
    resetcolor();
    prints("\n");
}

void sys_list()
{
    char ans[3];
    int i,k;
    sys_reset();
    load_users(FRIENDTOP);
    move(1,0);
    prints("用户            做题人数\n");
    k=2;
    for (i=0;i<userst;i++) {
        move(k,0);
        prints("%-16s %d", users[i].userid, users[i].tried);
        k++;
        if (k>20&&i<userst-1) {
            getdata(22,0,"按\033[32;1mq\033[m退出:",ans,2,true,NULL,true);
            if (toupper(ans[0])=='Q') break;
            sys_reset();
            move(1,0);
            prints("用户            做题人数\n");
            k=2;
        }
    }
    pressreturn();
}

int admin_menu()
{
    char ans[3];
    int i,j,k,tuid;
    char direct[PATHLEN], buf[IDLEN+2];

    if (!HAS_PERM(getCurrentUser(), PERM_SYSOP)) return -1;
    resetcolor();
    clear();
    move(0,0);
    setfcolor(YELLOW,1);
    setbcolor(BLUE);
    prints("   FRIENDTEST      管理模式 %s                                                 ", getCurrentUser()->userid);
    resetcolor();
    prints("\n");

    getdata(3,0,"选择(0-删题,1-禁止开题,2-允许开题,3-禁止做题,4-允许做题,5-列表,其他-离开) ", ans, 2, true, NULL, true);
    if (ans[0]>='0'&&ans[0]<='4') {
        getdata(5,0,"请输入id:",buf,IDLEN+1,true,NULL,true);
        tuid = getuser(buf, &lookuser);
        if (!tuid) {
            move(7,0);
            clrtoeol();
            prints("错误的使用者id!");
            return -1;
        }
    }
    {
        switch (ans[0]) {
            case '0':
                sethome(direct, lookuser->userid);
                unlink(direct);
                sethometop(direct, lookuser->userid);
                unlink(direct);
                sethomestat(direct, lookuser->userid);
                unlink(direct);
                move(7,0);
                prints("删除成功");
                break;
            case '1':
                load_users(BADLIST);
                j=1;
                for (i=0;i<userst;i++) {
                    if (!strcmp(users[i].userid, lookuser->userid)) {
                        j=0;
                        if (users[i].tried&1) j=2;
                        users[i].tried |= 1;
                        break;
                    }
                }
                if (j==1) {
                    if (!userst)
                        users=(struct usertype*)malloc(sizeof(struct usertype)*(userst+1));
                    strcpy(users[userst].userid, lookuser->userid);
                    users[userst].tried = 1;
                    users[userst].got = 0;
                    users[userst].create = time(0);
                    userst++;
                }
                save_users(BADLIST);
                move(7,0);
                if (j==2)
                    prints("该用户没有开题权限");
                else
                    prints("封禁成功");
                break;
            case '2':
                load_users(BADLIST);
                j=1;
                for (i=0;i<userst;i++) {
                    if (!strcmp(users[i].userid, lookuser->userid)) {
                        j=0;
                        if (!(users[i].tried&1)) j=1;
                        users[i].tried &= ~1;
                        if (users[i].tried==0) {
                            for (k=i;k<userst-1;k++)
                                memcpy(&users[k], &users[k+1], sizeof(users[k]));
                            userst--;
                        }
                        break;
                    }
                }
                save_users(BADLIST);
                move(7,0);
                if (j==1)
                    prints("该用户未被封禁");
                else
                    prints("解除成功");
                break;
            case '3':
                load_users(BADLIST);
                j=1;
                for (i=0;i<userst;i++) {
                    if (!strcmp(users[i].userid, lookuser->userid)) {
                        j=0;
                        if (users[i].tried&2) j=2;
                        users[i].tried |= 2;
                        break;
                    }
                }
                if (j==1) {
                    if (!userst)
                        users=(struct usertype*)malloc(sizeof(struct usertype)*(userst+1));
                    strcpy(users[userst].userid, lookuser->userid);
                    users[userst].tried = 2;
                    users[userst].got = 0;
                    users[userst].create = time(0);
                    userst++;
                }
                save_users(BADLIST);
                move(7,0);
                if (j==2)
                    prints("该用户没有做题权限");
                else
                    prints("封禁成功");
                break;
            case '4':
                load_users(BADLIST);
                j=1;
                for (i=0;i<userst;i++) {
                    if (!strcmp(users[i].userid, lookuser->userid)) {
                        j=0;
                        if (!(users[i].tried&2)) j=1;
                        users[i].tried &= ~2;
                        if (users[i].tried==0) {
                            for (k=i;k<userst-1;k++)
                                memcpy(&users[k], &users[k+1], sizeof(users[k]));
                            userst--;
                        }
                        break;
                    }
                }
                save_users(BADLIST);
                move(7,0);
                if (j==1)
                    prints("该用户未被封禁");
                else
                    prints("解除成功");
                break;
            case '5':
                load_users(BADLIST);
                resetcolor();
                clear();
                move(0,0);
                setfcolor(YELLOW,1);
                setbcolor(BLUE);
                prints("   FRIENDTEST      管理模式 %s                                                 ", getCurrentUser()->userid);
                resetcolor();
                move(1,0);
                prints("用户            出题  做题\n");
                k=2;
                for (i=0;i<userst;i++) {
                    move(k,0);
                    prints("%-16s %s     %s", users[i].userid, users[i].tried&1?"Y":"N", users[i].tried&2?"Y":"N");
                    k++;
                    if (k>20&&i<userst-1) {
                        getdata(22,0,"按\033[32;1mq\033[m退出:",ans,2,true,NULL,true);
                        if (toupper(ans[0])=='Q') break;
                        resetcolor();
                        clear();
                        move(0,0);
                        setfcolor(YELLOW,1);
                        setbcolor(BLUE);
                        prints("   FRIENDTEST      管理模式 %s                                                 ", getCurrentUser()->userid);
                        resetcolor();
                        prints("\n");
                        move(1,0);
                        prints("用户            出题  做题\n");
                        k=2;
                    }
                }
                break;
        }
    }
    pressreturn();
    return 0;
}

int friend_main()
{
    int i;
    init();
    load_users(BADLIST);
    for (i=0;i<userst;i++)
        if (!strcmp(users[i].userid,getCurrentUser()->userid)&&users[i].tried&2) {
            prints("\n你已经被封禁了该游戏权限\n");
            pressreturn();
            done();
            return -1;
        }

    modify_user_mode(FRIENDTEST);
    while ((i=usermenu())!=0) {
        switch (i) {
            case 1:
                new_friendtest();
                break;
            case 2:
                do_test();
                break;
            case 3:
                show_stat();
                break;
            case 4:
                admin_stat();
                break;
            case 5:
                sys_list();
                break;
            case 6:
                admin_menu();
                break;
        }
    }

    done();
    return 0;
}
