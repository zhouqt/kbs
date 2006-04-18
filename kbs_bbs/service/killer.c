/******************************************************
杀人游戏2003, 作者: bad@smth.org  Qian Wenjie
在水木清华bbs系统上运行

本游戏是自由软件，请随意复制移植
请在修改后的文件头部保留版权信息
******************************************************/

#include "service.h"
#include "bbs.h"

SMTH_API struct user_info uinfo;
SMTH_API int t_lines;
SMTH_API int kicked;
SMTH_API int RMSG;

#define MAX_ROOM 100
#define MAX_PEOPLE 100
#define MAX_MSG 2000

#define ROOM_LOCKED 01
#define ROOM_SECRET 02
#define ROOM_DENYSPEC 04
#define ROOM_SPECBLIND 010

struct room_struct {
    int w;
    int style; /* 0 - chat room 1 - killer room */
    char name[14];
    char title[NAMELEN];
    char creator[IDLEN+2];
    unsigned int level;
    int flag;
    int people;
    int maxpeople;
};

#define PEOPLE_SPECTATOR 01
#define PEOPLE_KILLER 02
#define PEOPLE_ALIVE 04
#define PEOPLE_ROOMOP 010
#define PEOPLE_POLICE 020
#define PEOPLE_TESTED 040
#define PEOPLE_DENYSPEAK 0100

struct people_struct {
    int style;
    char id[IDLEN+2];
    char nick[NAMELEN];
    int flag;
    int pid;
    int vote;
    int vnum;
};

#define INROOM_STOP 1
#define INROOM_NIGHT 2
#define INROOM_DAY 3

struct inroom_struct {
    int w;
    int status;
    int killernum;
    int policenum;
    struct people_struct peoples[MAX_PEOPLE];
    char msgs[MAX_MSG][60];
    int msgpid[MAX_MSG];
    int msgi;
};

struct room_struct * rooms;
struct inroom_struct * inrooms;

struct killer_record {
    int w; //0 - 平民胜利 1 - 杀手胜利
    int t;
    int peoplet;
    char id[MAX_PEOPLE][IDLEN+2];
    int st[MAX_PEOPLE]; // 0 - 活着平民 1 - 死了平民 2 - 活着杀手 3 - 死了杀手 4 - 其他情况
};

int myroom, mypos;


void save_result(int w)
{
    int fd;
    struct flock ldata;
    struct killer_record r;
    int i,j;
    char filename[80]="service/.KILLERRESULT";
    r.t = time(0);
    r.w = w;
    r.peoplet = 0; j = 0;
    for(i=0;i<MAX_PEOPLE;i++)
    if(inrooms[myroom].peoples[i].style!=-1&&!(inrooms[myroom].peoples[i].flag&PEOPLE_SPECTATOR)) {
        strcpy(r.id[j], inrooms[myroom].peoples[i].id);
        r.st[j] = 4;
        if(!(inrooms[myroom].peoples[i].flag&PEOPLE_KILLER)) {
            if(inrooms[myroom].peoples[i].flag&PEOPLE_ALIVE)
                r.st[j] = 0;
            else
                r.st[j] = 1;
        } else {
            if(inrooms[myroom].peoples[i].flag&PEOPLE_ALIVE)
                r.st[j] = 2;
            else
                r.st[j] = 3;
        }
        
        j++;
        r.peoplet++;
    }
    if((fd = open(filename, O_WRONLY|O_CREAT, 0644))!=-1) {
        ldata.l_type=F_WRLCK;
        ldata.l_whence=0;
        ldata.l_len=0;
        ldata.l_start=0;
        if(fcntl(fd, F_SETLKW, &ldata)!=-1){
            lseek(fd, 0, SEEK_END);
            write(fd, &r, sizeof(struct killer_record));
            	
            ldata.l_type = F_UNLCK;
            fcntl(fd, F_SETLKW, &ldata);
        }
        close(fd);
    }
}

/*void load_msgs()
{
    FILE* fp;
    int i;
    char filename[80], buf[80];
    msgst=0;
    sprintf(filename, "home/%c/%s/.INROOMMSG%d", toupper(getCurrentUser()->userid[0]), getCurrentUser()->userid, uinfo.pid);
    fp = fopen(filename, "r");
    if(fp) {
        while(!feof(fp)) {
            if(fgets(buf, 79, fp)==NULL) break;
            if(buf[0]) {
                if(!strncmp(buf, "你被踢了", 8)) kicked=1;
                if(msgst==200) {
                    msgst--;
                    for(i=0;i<msgst;i++)
                        strcpy(msgs[i],msgs[i+1]);
                }
                strcpy(msgs[msgst],buf);
                msgst++;
            }
        }
        fclose(fp);
    }
}*/

void start_change_inroom()
{
    if(inrooms[myroom].w) sleep(0);
    inrooms[myroom].w = 1;
}

void end_change_inroom()
{
    inrooms[myroom].w = 0;
}

struct action {
    char *verb;                 /* 动词 */
    char *part1_msg;            /* 介词 */
    char *part2_msg;            /* 动作 */
};

struct action party_data[] = {
    {"?", "很疑惑的看着", ""},
    {"admire", "对", "的景仰之情犹如滔滔江水连绵不绝"},
    {"agree", "完全同意", "的看法"},
    {"bearhug", "热情的拥抱", ""},
    {"bless", "祝福", "心想事成"},
    {"bow", "毕躬毕敬的向", "鞠躬"},
    {"bye", "看着", "的背影，凄然泪下。身后的收音机传来邓丽君的歌声:\n\"\033[31m何日君再来.....\033[m\""},
    {"caress", "轻轻的抚摸", ""},
    {"cat", "像只小猫般地依偎在", "的怀里撒娇。"},
    {"cringe", "向", "卑躬屈膝，摇尾乞怜"},
    {"cry", "向", "嚎啕大哭"},
    {"comfort", "温言安慰", ""},
    {"clap", "向", "热烈鼓掌"},
    {"dance", "拉着", "的手翩翩起舞"},
    {"dogleg", "对", "狗腿"},
    {"drivel", "对著", "流口水"},
    {"dunno", "瞪大眼睛，天真地问：", "，你说什麽我不懂耶... :("},
    {"faint", "晕倒在", "的怀里"},
    {"fear", "对", "露出怕怕的表情"},
    {"fool", "请大家注意", "这个大白痴....\n真是天上少有....人间仅存的活宝....\n不看太可惜了！"},
    {"forgive", "大度的对", "说：算了，原谅你了"},
    {"giggle", "对著", "傻傻的呆笑"},
    {"grin", "对", "露出邪恶的笑容"},
    {"growl", "对", "咆哮不已"},
    {"hand", "跟", "握手"},
    {"hammer", "举起好大好大的铁锤！！哇！往",
     "头上用力一敲！\n***************\n*  5000000 Pt *\n***************\n      | |      ★☆★☆★☆\n      | |         好多的星星哟\n      |_|"},
    {"heng", "看都不看", "一眼， 哼了一声，高高的把头扬起来了,不屑一顾的样子..."},
    {"hug", "轻轻地拥抱", ""},
    {"idiot", "无情地耻笑", "的痴呆。"},
    {"kick", "把", "踢的死去活来"},
    {"kiss", "轻吻", "的脸颊"},
    {"laugh", "大声嘲笑", ""},
    {"lovelook", "拉着", "的手，温柔地默默对视。目光中孕含着千种柔情，万般蜜意"},
    {"nod", "向", "点头称是"},
    {"nudge", "用手肘顶", "的肥肚子"},
    {"oh", "对", "说：“哦，酱子啊！”"},
    {"pad", "轻拍", "的肩膀"},
    {"papaya", "敲了敲", "的木瓜脑袋"},
    {"pat", "轻轻拍拍", "的头"},
    {"pinch", "用力的把", "拧的黑青"},
    {"puke", "对着", "吐啊吐啊，据说吐多几次就习惯了"},
    {"punch", "狠狠揍了", "一顿"},
    {"pure", "对", "露出纯真的笑容"},
    {"qmarry", "向", "勇敢的跪了下来:\n\"你愿意嫁给我吗？\"\n---真是勇气可嘉啊"},
    {"report", "偷偷地对", "说：“报告我好吗？”"},
    {"shrug", "无奈地向", "耸了耸肩膀"},
    {"sigh", "对", "叹了一口气"},
    {"slap", "啪啪的巴了", "一顿耳光"},
    {"smooch", "拥吻著", ""},
    {"snicker", "嘿嘿嘿..的对", "窃笑"},
    {"sniff", "对", "嗤之以鼻"},
    {"sorry", "痛哭流涕的请求", "原谅"},
    {"spank", "用巴掌打", "的臀部"},
    {"squeeze", "紧紧地拥抱著", ""},
    {"thank", "向", "道谢"},
    {"tickle", "咕叽!咕叽!搔", "的痒"},
    {"waiting", "深情地对", "说：每年每月的每一天，每分每秒我都在这里等着你"},
    {"wake", "努力的摇摇", "，在其耳边大叫：“快醒醒，会着凉的！”"},
    {"wave", "对著", "拼命的摇手"},
    {"welcome", "热烈欢迎", "的到来"},
    {"wink", "对", "神秘的眨眨眼睛"},
    {"xixi", "嘻嘻地对", "笑了几声"},
    {"zap", "对", "疯狂的攻击"},
    {"inn", "双眼饱含着泪水，无辜的望着", ""},
    {"mm", "色眯眯的对", "问好：“美眉好～～～～”。大色狼啊！！！"},
    {"disapp", "这下没搞头啦，为什么", "姑娘对我这个造型完全没反应？没办法！"},
    {"miss", "真诚的望着", "：我想念你我真的想念你我太--想念你了!你相不相信?"},
    {"buypig", "指着", "：“这个猪头给我切一半，谢谢！”"},
    {"rascal", "对", "大叫：“你这个臭流氓！！！！！！！！！！！！！！！！”"},
    {"qifu", "小嘴一扁，对", "哭道：“你欺负我，你欺负我！！！”"},
    {"wa", "对", "大叫一声：“哇哇哇哇哇哇酷弊了耶！！！！！！！！！！！！！！”"},
    {"feibang", "喔－－！熟归熟，", "你这样乱讲话，我一样可以告你毁谤，哈！"},
    {"badman", "指着", "的脑袋, 手舞足蹈的喊:“ 杀~~手~~在~~这~~里~~~”"}, 
    {NULL, NULL, NULL}
};

struct action speak_data[] = {
    {"ask", "询问", NULL},
    {"chant", "歌颂", NULL},
    {"cheer", "喝采", NULL},
    {"chuckle", "轻笑", NULL},
    {"curse", "咒骂", NULL},
    {"demand", "要求", NULL},
    {"frown", "蹙眉", NULL},
    {"groan", "呻吟", NULL},
    {"grumble", "发牢骚", NULL},
    {"hum", "喃喃自语", NULL},
    {"moan", "悲叹", NULL},
    {"notice", "注意", NULL},
    {"order", "命令", NULL},
    {"ponder", "沈思", NULL},
    {"pout", "噘著嘴说", NULL},
    {"pray", "祈祷", NULL},
    {"request", "恳求", NULL},
    {"shout", "大叫", NULL},
    {"sing", "唱歌", NULL},
    {"smile", "微笑", NULL},
    {"smirk", "假笑", NULL},
    {"swear", "发誓", NULL},
    {"tease", "嘲笑", NULL},
    {"whimper", "呜咽的说", NULL},
    {"yawn", "哈欠连天", NULL},
    {"yell", "大喊", NULL},
    {NULL, NULL, NULL}
};

struct action condition_data[] = {
    {":D", "乐的合不拢嘴", NULL},
    {":)", "乐的合不拢嘴", NULL},
    {":P", "乐的合不拢嘴", NULL},
    {":(", "乐的合不拢嘴", NULL},
    {"applaud", "啪啪啪啪啪啪啪....", NULL},
    {"blush", "脸都红了", NULL},
    {"cough", "咳了几声", NULL},
    {"faint", "咣当一声，晕倒在地", NULL},
    {"happy", "的脸上露出了幸福的表情，并学吃饱了的猪哼哼了起来", NULL},
    {"lonely", "一个人坐在房间里，百无聊赖，希望谁来陪陪。。。。", NULL},
    {"luck", "哇！福气啦！", NULL},
    {"puke", "真恶心，我听了都想吐", NULL},
    {"shake", "摇了摇头", NULL},
    {"sleep", "Zzzzzzzzzz，真无聊，都快睡著了", NULL},
    {"so", "就酱子!!", NULL},
    {"strut", "大摇大摆地走", NULL},
    {"tongue", "吐了吐舌头", NULL},
    {"think", "歪著头想了一下", NULL},
    {"wawl", "惊天动地的哭", NULL},
    {"goodman", "用及其无辜的表情看着大家: “我真的是好人耶~~”", NULL},
    {NULL, NULL, NULL}
};

void send_msg(int u, char* msg)
{
    int i, j, k, f;
    char buf[200], buf2[200], buf3[80];

    strcpy(buf, msg);

    for(i=0;i<=6;i++) {
        buf3[0]='%'; buf3[1]=i+48; buf3[2]=0;
        while(strstr(buf, buf3)!=NULL) {
            strcpy(buf2, buf);
            k=strstr(buf, buf3)-buf;
            buf2[k]=0; k+=2;
            sprintf(buf, "%s\x1b[3%dm%s", buf2, (i>0)?i:7, buf2+k);
        }
    }
    
    while(strchr(buf, '\n')!=NULL) {
        i = strchr(buf, '\n')-buf;
        buf[i]=0;
        send_msg(u, buf);
        strcpy(buf2, buf+i+1);
        strcpy(buf, buf2);
    }
    while(strlen(buf)>56) {
        int maxi=0;
        k=0; j = 0; f = 0;
        for(i=0;i<strlen(buf);i++) {
            if(buf[i]=='\x1b') f = 1;
            if(f) {
                if(isalpha(buf[i])) f=0;
                continue;
            }
            if(k==0&&i<=56) {
                if(i>maxi)
                    maxi = i;
            }
            j++;
            if(k) k=0;
            else if(buf[i]<0) k=1;
        }
        if(maxi<strlen(buf)&&maxi!=0) {
            strcpy(buf2, buf);
            buf[maxi]=0;
            send_msg(u, buf);
            strcpy(buf, buf2+maxi);
        }
        else break;
    }
    j=MAX_MSG;
    if(inrooms[myroom].msgs[(MAX_MSG-1+inrooms[myroom].msgi)%MAX_MSG][0]==0)
    for(i=0;i<MAX_MSG;i++)
        if(inrooms[myroom].msgs[(i+inrooms[myroom].msgi)%MAX_MSG][0]==0) {
            j=(i+inrooms[myroom].msgi)%MAX_MSG;
            break;
        }
    if(j==MAX_MSG) {
        strcpy(inrooms[myroom].msgs[inrooms[myroom].msgi], buf);
        if(u==-1)
            inrooms[myroom].msgpid[inrooms[myroom].msgi] = -1;
        else
            inrooms[myroom].msgpid[inrooms[myroom].msgi] = inrooms[myroom].peoples[u].pid;
        inrooms[myroom].msgi = (inrooms[myroom].msgi+1)%MAX_MSG;
    }
    else {
        strcpy(inrooms[myroom].msgs[j], buf);
        if(u==-1)
            inrooms[myroom].msgpid[j] = u;
        else
            inrooms[myroom].msgpid[j] = inrooms[myroom].peoples[u].pid;
    }
}

void kill_msg(int u)
{
    int i,j,k;
    char buf[80];
    for(i=0;i<MAX_PEOPLE;i++)
    if(inrooms[myroom].peoples[i].style!=-1)
    if(u==-1||i==u) {
        j=kill(inrooms[myroom].peoples[i].pid, SIGUSR1);
        if(j==-1) {
            sprintf(buf, "%s掉线了", inrooms[myroom].peoples[i].nick);
            send_msg(-1, buf);
            start_change_inroom();
            inrooms[myroom].peoples[i].style=-1;
            rooms[myroom].people--;
            if(inrooms[myroom].peoples[i].flag&PEOPLE_ROOMOP) {
                for(k=0;k<MAX_PEOPLE;k++) 
                if(inrooms[myroom].peoples[k].style!=-1&&!(inrooms[myroom].peoples[k].flag&PEOPLE_SPECTATOR))
                {
                    inrooms[myroom].peoples[k].flag|=PEOPLE_ROOMOP;
                    sprintf(buf, "%s成为新房主", inrooms[myroom].peoples[k].nick);
                    send_msg(-1, buf);
                    break;
                }
            }
            end_change_inroom();
            i=-1;
        }
    }
}

int add_room(struct room_struct * r)
{
    int i,j;
    for(i=0;i<MAX_ROOM;i++) 
    if(rooms[i].style==1) {
        if(!strcmp(rooms[i].name, r->name))
            return -1;
        if(!strcmp(rooms[i].creator, getCurrentUser()->userid))
            return -1;
    }
    for(i=0;i<MAX_ROOM;i++)
    if(rooms[i].style==-1) {
        memcpy(rooms+i, r, sizeof(struct room_struct));
        inrooms[i].status = INROOM_STOP;
        inrooms[i].killernum = 0;
        inrooms[i].msgi = 0;
        inrooms[i].policenum = 0;
        inrooms[i].w = 0;
        for(j=0;j<MAX_MSG;j++)
            inrooms[i].msgs[j][0]=0;
        for(j=0;j<MAX_PEOPLE;j++)
            inrooms[i].peoples[j].style = -1;
        return 0;
    }
    return -1;
}

/*
int del_room(struct room_struct * r)
{
    int i, j;
    for(i=0;i<*roomst;i++)
    if(!strcmp(rooms[i].name, r->name)) {
        rooms[i].name[0]=0;
        break;
    }
    return 0;
}
*/

void clear_room()
{
    int i;
    for(i=0;i<MAX_ROOM;i++)
        if((rooms[i].style!=-1) && (rooms[i].people==0))
            rooms[i].style=-1;
}

int can_see(struct room_struct * r)
{
    if(r->style==-1) return 0;
    if((r->level&getCurrentUser()->userlevel)!=r->level) return 0;
    if(r->style!=1) return 0;
    if(r->flag&ROOM_SECRET&&!HAS_PERM(getCurrentUser(), PERM_SYSOP)) return 0;
    return 1;
}

int can_enter(struct room_struct * r)
{
    if(r->style==-1) return 0;
    if((r->level&getCurrentUser()->userlevel)!=r->level) return 0;
    if(r->style!=1) return 0;
    if(r->flag&ROOM_LOCKED&&!HAS_PERM(getCurrentUser(), PERM_SYSOP)) return 0;
    return 1;
}

int room_count()
{
    int i,j=0;
    for(i=0;i<MAX_ROOM;i++)
        if(can_see(rooms+i)) j++;
    return j;
}

int room_get(int w)
{
    int i,j=0;
    for(i=0;i<MAX_ROOM;i++) {
        if(can_see(rooms+i)) {
            if(w==j) return i;
            j++;
        }
    }
    return -1;
}

int find_room(char * s)
{
    int i;
    struct room_struct * r2;
    for(i=0;i<MAX_ROOM;i++) {
        r2 = rooms+i;
        if(!can_enter(r2)) continue;
        if(!strcmp(r2->name, s))
            return i;
    }
    return -1;
}

int selected = 0, ipage=0, jpage=0;

int getpeople(int i)
{
    int j, k=0;
    for(j=0;j<MAX_PEOPLE;j++) {
        if(inrooms[myroom].peoples[j].style==-1) continue;
        if(i==k) return j;
        k++;
    }
    return -1;
}

int get_msgt()
{
    int i,j=0,k;
    for(i=0;i<MAX_MSG;i++) {
        if(inrooms[myroom].msgs[(i+inrooms[myroom].msgi)%MAX_MSG][0]==0) break;
        k=inrooms[myroom].msgpid[(i+inrooms[myroom].msgi)%MAX_MSG];
        if(k==-1||k==uinfo.pid) j++;
    }
    return j;
}

char * get_msgs(int s)
{
    int i,j=0,k;
    char * ss;
    for(i=0;i<MAX_MSG;i++) {
        if(inrooms[myroom].msgs[(i+inrooms[myroom].msgi)%MAX_MSG][0]==0) break;
        k=inrooms[myroom].msgpid[(i+inrooms[myroom].msgi)%MAX_MSG];
        if(k==-1||k==uinfo.pid) {
            if(j==s) {
                ss = inrooms[myroom].msgs[(i+inrooms[myroom].msgi)%MAX_MSG];
                return ss;
            }
            j++;
        }
    }
    return NULL;
}

void save_msgs(char * s)
{
    FILE* fp;
    int i;
    fp=fopen(s, "w");
    if(fp==NULL) return;
    for(i=0;i<get_msgt();i++)
        fprintf(fp, "%s\n", get_msgs(i));
    fclose(fp);
}

void refreshit()
{
    int i,j,me,msgst,k,i0;
    char buf[30],buf2[30],buf3[30];
    for(i=0;i<t_lines-1;i++) {
        move(i, 0);
        clrtoeol();
    }
    move(0,0);
    prints("\033[44;33;1m 房间:\033[36m%-12s\033[33m话题:\033[36m%-40s\033[33m状态:\033[36m%6s",
        rooms[myroom].name, rooms[myroom].title, (inrooms[myroom].status==INROOM_STOP?"未开始":(inrooms[myroom].status==INROOM_NIGHT?"黑夜中":"大白天")));
    clrtoeol();
    resetcolor();
    setfcolor(YELLOW, 1);
    move(1,0);
    prints("╭\x1b[32m玩家\x1b[33m—————╮╭\x1b[32m讯息\x1b[33m———————————————————————————╮");
    move(t_lines-2,0);
    prints("╰———————╯╰—————————————————————————————╯");
    for(i=2;i<=t_lines-3;i++) {
        move(i,0); prints("│");
        move(i,16); prints("│");
        move(i,18); prints("│");
        move(i,78); prints("│");
    }
    me=mypos;
    for(i=2;i<=t_lines-3;i++) 
    if(ipage+i-2>=0&&ipage+i-2<rooms[myroom].people) {
        j=getpeople(ipage+i-2);
        if(j==-1) continue;
        if(inrooms[myroom].status!=INROOM_STOP)
        if((inrooms[myroom].peoples[j].flag&PEOPLE_KILLER) && ((inrooms[myroom].peoples[me].flag&PEOPLE_KILLER) ||
            ((inrooms[myroom].peoples[me].flag&PEOPLE_SPECTATOR)&&(rooms[myroom].flag&ROOM_SPECBLIND)) ||
            !(inrooms[myroom].peoples[j].flag&PEOPLE_ALIVE))) {
            resetcolor();
            move(i,2);
            setfcolor(RED, 1);
            prints("*");
        }
        if(inrooms[myroom].status!=INROOM_STOP&&!(inrooms[myroom].peoples[j].flag&PEOPLE_ALIVE)&&
            !(inrooms[myroom].peoples[j].flag&PEOPLE_SPECTATOR)) {
            resetcolor();
            move(i,3);
            setfcolor(BLUE, 1);
            prints("X");
        }
        else if((inrooms[myroom].peoples[j].flag&PEOPLE_SPECTATOR)) {
            resetcolor();
            move(i,3);
            setfcolor(GREEN, 0);
            prints("O");
        }
        else if(inrooms[myroom].status == INROOM_DAY ||
            (inrooms[myroom].status == INROOM_NIGHT &&
            ((inrooms[myroom].peoples[me].flag&PEOPLE_KILLER) ||
            ((inrooms[myroom].peoples[me].flag&PEOPLE_SPECTATOR)&&(rooms[myroom].flag&ROOM_SPECBLIND)))))
            if((inrooms[myroom].peoples[j].flag&PEOPLE_ALIVE)&&
            (inrooms[myroom].peoples[j].vote != 0)) {
            resetcolor();
            move(i,3);
            setfcolor(YELLOW, 0);
            prints("v");
        }
        resetcolor();
        move(i,4);
        if(ipage+i-2==selected) {
            setfcolor(RED, 1);
        }
        sprintf(buf, "%d %s", j+1, inrooms[myroom].peoples[j].nick);
        buf[12]=0;
        if(inrooms[myroom].status == INROOM_DAY ||
            (inrooms[myroom].status == INROOM_NIGHT &&
            ((inrooms[myroom].peoples[me].flag&PEOPLE_KILLER) ||
            (inrooms[myroom].peoples[me].flag&PEOPLE_SPECTATOR)))) {
            k=0;
            for(i0=0;i0<MAX_PEOPLE;i0++)
                if(inrooms[myroom].peoples[i0].style!=-1 && inrooms[myroom].peoples[i0].vote==
                    inrooms[myroom].peoples[j].pid)
                    k++;
            if(k>0) {
                int j0=0;
                if(k>=strlen(buf)) k=strlen(buf);
                for(i0=0;i0<k;i0++) {
                    if(j0) j0=0;
                    else if(buf[i0]<0) j0=1;
                }
                if(j0&&k<strlen(buf)) k++;
                strcpy(buf2, buf);
                buf2[k]=0;
                strcpy(buf3, buf+k);
                sprintf(buf, "\x1b[4m%s\x1b[m%s%s", buf2, (ipage+i-2==selected)?"\x1b[31;1m":"", buf3);
            }
        }
        prints("%s", buf);
    }
    resetcolor();
    msgst=get_msgt();
    for(i=2;i<=t_lines-3;i++) 
    if(msgst-1-(t_lines-3-i)-jpage>=0)
    {
        char * ss=get_msgs(msgst-1-(t_lines-3-i)-jpage);
        if(!strcmp(ss, "你被踢了")) kicked = 1;
        move(i,20);
        if(ss) prints("%s", ss);
        resetcolor();
    }
}


void room_refresh(int signo)
{
    int y,x;
    signal(SIGUSR1, room_refresh);

    if(RMSG) return;
    if(rooms[myroom].style!=1) kicked = 1;
    
    getyx(&y, &x);
    refreshit();
    move(y, x);
    refresh();
}

void start_game()
{
    int i,j,totalk=0,total=0,totalc=0, me;
    char buf[80];
    me=mypos;
    for(i=0;i<MAX_PEOPLE;i++) 
    if(inrooms[myroom].peoples[i].style!=-1)
    {
        inrooms[myroom].peoples[i].flag &= ~PEOPLE_KILLER;
        inrooms[myroom].peoples[i].flag &= ~PEOPLE_POLICE;
        inrooms[myroom].peoples[i].vote = 0;
    }
    totalk=inrooms[myroom].killernum;
    totalc=inrooms[myroom].policenum;
    for(i=0;i<MAX_PEOPLE;i++)
        if(inrooms[myroom].peoples[i].style!=-1)
        if(!(inrooms[myroom].peoples[i].flag&PEOPLE_SPECTATOR)) 
            total++;
    if(total<6) {
        send_msg(me, "\x1b[31;1m至少6人参加才能开始游戏\x1b[m");
        end_change_inroom();
        refreshit();
        return;
    }
    if(totalk==0) totalk=((double)total/6+0.5);
    if(totalk>=total/4) totalk=total/4;
    if(totalc>=total/6) totalc=total/6;
    if(totalk>total) {
        send_msg(me, "\x1b[31;1m总人数少于要求的坏人人数,无法开始游戏\x1b[m");
        end_change_inroom();
        refreshit();
        return;
    }
    if(totalc==0)
        sprintf(buf, "\x1b[31;1m游戏开始啦! 人群中出现了%d个坏人\x1b[m", totalk);
    else
        sprintf(buf, "\x1b[31;1m游戏开始啦! 人群中出现了%d个坏人, %d个警察\x1b[m", totalk, totalc);
    send_msg(-1, buf);
    for(i=0;i<totalk;i++) {
        do{
            j=rand()%MAX_PEOPLE;
        }while(inrooms[myroom].peoples[j].style==-1 || inrooms[myroom].peoples[j].flag&PEOPLE_KILLER || inrooms[myroom].peoples[j].flag&PEOPLE_SPECTATOR);
        inrooms[myroom].peoples[j].flag |= PEOPLE_KILLER;
        send_msg(j, "你做了一个无耻的坏人");
        send_msg(j, "用你的尖刀(\x1b[31;1mCtrl+S\x1b[m)选择你要残害的人吧...");
    }
    for(i=0;i<totalc;i++) {
        do{
            j=rand()%MAX_PEOPLE;
        }while(inrooms[myroom].peoples[j].style==-1 || inrooms[myroom].peoples[j].flag&PEOPLE_KILLER || inrooms[myroom].peoples[j].flag&PEOPLE_POLICE || inrooms[myroom].peoples[j].flag&PEOPLE_SPECTATOR);
        inrooms[myroom].peoples[j].flag |= PEOPLE_POLICE;
        send_msg(j, "你做了一位光荣的人民警察");
        send_msg(j, "白天请用你的警棒(\x1b[31;1mCtrl+T\x1b[m)选择你怀疑的人...");
    }
    for(i=0;i<MAX_PEOPLE;i++) 
    if(inrooms[myroom].peoples[i].style!=-1)
    if(!(inrooms[myroom].peoples[i].flag&PEOPLE_SPECTATOR))
    {
        inrooms[myroom].peoples[i].flag |= PEOPLE_ALIVE;
        if(!(inrooms[myroom].peoples[i].flag&PEOPLE_KILLER))
            send_msg(i, "现在是晚上...");
    }
    inrooms[myroom].status = INROOM_NIGHT;
    end_change_inroom();
    kill_msg(-1);
}

#define menust 10
int do_com_menu()
{
    char menus[menust][15]=
        {"0-返回","1-退出游戏","2-改名字", "3-玩家列表", "4-改话题", "5-设置房间", "6-踢玩家", "7-发言权", "8-交换位置", "9-开始游戏"};
    int menupos[menust],i,j,k,sel=0,ch,max=0,me,offset=0;
    char buf[80];
    if(inrooms[myroom].status != INROOM_STOP)
        strcpy(menus[9], "9-结束游戏");
    menupos[0]=0;
    for(i=1;i<menust;i++)
        menupos[i]=menupos[i-1]+strlen(menus[i-1])+1;
    do{
        resetcolor();
        move(t_lines-1,0);
        clrtoeol();
        offset = 0;
        while(menupos[sel]-menupos[offset]+strlen(menus[sel])>=scr_cols) offset++;
        j=mypos;
        for(i=0;i<menust;i++) 
        if(inrooms[myroom].peoples[j].flag&PEOPLE_ROOMOP||i<=3)
        if(menupos[i]-menupos[offset]>=0)
        {
            resetcolor();
            move(t_lines-1, menupos[i]-menupos[offset]);
            if(i==sel) {
                setfcolor(RED,1);
            }
            if(i>=max-1) max=i+1;
            prints("%s", menus[i]);
        }
        ch=igetkey();
        if(kicked) return 0;
        switch(ch){
        case KEY_LEFT:
        case KEY_UP:
            sel--;
            if(sel<0) sel=max-1;
            break;
        case KEY_RIGHT:
        case KEY_DOWN:
            sel++;
            if(sel>=max) sel=0;
            break;
        case '\n':
        case '\r':
            switch(sel) {
                case 0:
                    return 0;
                case 1:
                    me=mypos;
                    if(inrooms[myroom].peoples[me].flag&PEOPLE_ALIVE&&!(inrooms[myroom].peoples[me].flag&PEOPLE_ROOMOP)&&inrooms[myroom].status!=INROOM_STOP) {
                        send_msg(me, "你还在游戏,不能退出");
                        refreshit();
                        return 0;
                    }
                    move(t_lines-1, 0);
                    resetcolor();
                    clrtoeol();
                    getdata(t_lines-1, 0, "确认退出？ [y/N] ", buf, 3, 1, 0, 1);
                    if(kicked) return 0;
                    if(toupper(buf[0])!='Y') return 0;
                    return 1;
                case 2:
                    move(t_lines-1, 0);
                    resetcolor();
                    clrtoeol();
                    getdata(t_lines-1, 0, "请输入名字:", buf, 13, 1, 0, 1);
                    if(kicked) return 0;
                    if(buf[0]) {
                        k=1;
                        for(j=0;j<strlen(buf);j++)
                            k=k&&(isprint2(buf[j]));
                        k=k&&(buf[0]!=' ');
                        k=k&&(buf[strlen(buf)-1]!=' ');
                        if(!k) {
                            move(t_lines-1,0);
                            resetcolor();
                            clrtoeol();
                            prints(" 名字不符合规范");
                            refresh();
                            sleep(1);
                            return 0;
                        }
                        me=mypos;
                        j=0;
                        for(i=0;i<MAX_PEOPLE;i++)
                            if(inrooms[myroom].peoples[i].style!=-1)
                            if(i!=me)
                            if(!strcmp(buf,inrooms[myroom].peoples[i].id) || !strcmp(buf,inrooms[myroom].peoples[i].nick)) j=1;
                        if(j) {
                            move(t_lines-1,0);
                            resetcolor();
                            clrtoeol();
                            prints(" 已有人用这个名字了");
                            refresh();
                            sleep(1);
                            return 0;
                        }
                        start_change_inroom();
                        me=mypos;
                        strcpy(inrooms[myroom].peoples[me].nick, buf);
                        end_change_inroom();
                        kill_msg(-1);
                    }
                    return 0;
                case 3:
                    me=mypos;
                    for(i=0;i<MAX_PEOPLE;i++) 
                    if(inrooms[myroom].peoples[i].style!=-1)
                    {
                        sprintf(buf, "%-12s  %s", inrooms[myroom].peoples[i].id, inrooms[myroom].peoples[i].nick);
                        send_msg(me, buf);
                    }
                    refreshit();
                    return 0;
                case 4:
                    move(t_lines-1, 0);
                    resetcolor();
                    clrtoeol();
                    getdata(t_lines-1, 0, "请输入话题:", buf, 31, 1, 0, 1);
                    if(kicked) return 0;
                    if(buf[0]) {
                        start_change_inroom();
                        strcpy(rooms[myroom].title, buf);
                        end_change_inroom();
                        kill_msg(-1);
                    }
                    return 0;
                case 5:
                    move(t_lines-1, 0);
                    resetcolor();
                    clrtoeol();
                    getdata(t_lines-1, 0, "请输入房间最大人数:", buf, 30, 1, 0, 1);
                    if(kicked) return 0;
                    if(buf[0]) {
                        i=atoi(buf);
                        if(i>0&&i<=100) {
                            rooms[myroom].maxpeople = i;
                            sprintf(buf, "屋主设置房间最大人数为%d", i);
                            send_msg(-1, buf);
                        }
                    }
                    move(t_lines-1, 0);
                    clrtoeol();
                    getdata(t_lines-1, 0, "设置为隐藏房间? [Y/N]", buf, 30, 1, 0, 1);
                    if(kicked) return 0;
                    buf[0]=toupper(buf[0]);
                    if(buf[0]=='Y'||buf[0]=='N') {
                        if(buf[0]=='Y') rooms[myroom].flag|=ROOM_SECRET;
                        else rooms[myroom].flag&=~ROOM_SECRET;
                        sprintf(buf, "屋主设置房间为%s", (buf[0]=='Y')?"隐藏":"不隐藏");
                        send_msg(-1, buf);
                    }
                    move(t_lines-1, 0);
                    clrtoeol();
                    getdata(t_lines-1, 0, "设置为锁定房间? [Y/N]", buf, 30, 1, 0, 1);
                    if(kicked) return 0;
                    buf[0]=toupper(buf[0]);
                    if(buf[0]=='Y'||buf[0]=='N') {
                        if(buf[0]=='Y') rooms[myroom].flag|=ROOM_LOCKED;
                        else rooms[myroom].flag&=~ROOM_LOCKED;
                        sprintf(buf, "屋主设置房间为%s", (buf[0]=='Y')?"锁定":"不锁定");
                        send_msg(-1, buf);
                    }
                    move(t_lines-1, 0);
                    clrtoeol();
                    getdata(t_lines-1, 0, "设置为拒绝旁观者的房间? [Y/N]", buf, 30, 1, 0, 1);
                    if(kicked) return 0;
                    buf[0]=toupper(buf[0]);
                    if(buf[0]=='Y'||buf[0]=='N') {
                        if(buf[0]=='Y') rooms[myroom].flag|=ROOM_DENYSPEC;
                        else rooms[myroom].flag&=~ROOM_DENYSPEC;
                        sprintf(buf, "屋主设置房间为%s", (buf[0]=='Y')?"拒绝旁观":"不拒绝旁观");
                        send_msg(-1, buf);
                    }
                    move(t_lines-1, 0);
                    clrtoeol();
                    getdata(t_lines-1, 0, "设置为旁观者无法看见杀手警察的房间? [Y/N]", buf, 30, 1, 0, 1);
                    if(kicked) return 0;
                    buf[0]=toupper(buf[0]);
                    if(buf[0]=='Y'||buf[0]=='N') {
                        if(buf[0]=='Y') rooms[myroom].flag|=ROOM_SPECBLIND;
                        else rooms[myroom].flag&=~ROOM_SPECBLIND;
                        sprintf(buf, "屋主设置房间为%s", (buf[0]=='Y')?"旁观无法看见杀手警察":"旁观可以看见杀手警察");
                        send_msg(-1, buf);
                    }
                    move(t_lines-1, 0);
                    clrtoeol();
                    getdata(t_lines-1, 0, "设置坏人的数目:", buf, 30, 1, 0, 1);
                    if(kicked) return 0;
                    if(buf[0]) {
                        i=atoi(buf);
                        if(i>=0&&i<=10) {
                            inrooms[myroom].killernum = i;
                            sprintf(buf, "屋主设置房间坏人数为%d", i);
                            send_msg(-1, buf);
                        }
                    }
                    move(t_lines-1, 0);
                    clrtoeol();
                    getdata(t_lines-1, 0, "设置警察的数目:", buf, 30, 1, 0, 1);
                    if(kicked) return 0;
                    if(buf[0]) {
                        i=atoi(buf);
                        if(i>=0&&i<=2) {
                            inrooms[myroom].policenum = i;
                            sprintf(buf, "屋主设置房间警察数为%d", i);
                            send_msg(-1, buf);
                        }
                    }
                    kill_msg(-1);
                    return 0;
                case 6:
                    move(t_lines-1, 0);
                    resetcolor();
                    clrtoeol();
                    getdata(t_lines-1, 0, "请输入要踢的序号(数字):", buf, 4, 1, 0, 1);
                    if(kicked) return 0;
                    if(buf[0]) {
                        i = atoi(buf)-1;
                        if(i>=0&&i<MAX_PEOPLE&&inrooms[myroom].peoples[i].style!=-1&&inrooms[myroom].peoples[i].pid!=uinfo.pid) {
                            inrooms[myroom].peoples[i].flag&=~PEOPLE_ALIVE;
                            inrooms[myroom].peoples[i].flag|=PEOPLE_SPECTATOR;
                            send_msg(i, "你被踢了");
                            kill_msg(i);
                            return 2;
                        }
                    }
                    return 0;
                case 7:
                    move(t_lines-1, 0);
                    resetcolor();
                    clrtoeol();
                    getdata(t_lines-1, 0, "请输入要禁止/恢复发言权的序号(数字):", buf, 4, 1, 0, 1);
                    if(kicked) return 0;
                    if(buf[0]) {
                        i = atoi(buf)-1;
                        if(i>=0&&i<MAX_PEOPLE&&inrooms[myroom].peoples[i].style!=-1&&inrooms[myroom].peoples[i].pid!=uinfo.pid) {
                            if(inrooms[myroom].peoples[i].flag&PEOPLE_DENYSPEAK)
                                inrooms[myroom].peoples[i].flag&=~PEOPLE_DENYSPEAK;
                            else
                                inrooms[myroom].peoples[i].flag|=PEOPLE_DENYSPEAK;
                            sprintf(buf, "%d %s被%s了发言权", i+1, inrooms[myroom].peoples[i].nick, (inrooms[myroom].peoples[i].flag&PEOPLE_DENYSPEAK)?"禁止":"恢复");
                            send_msg(-1, buf);
                            kill_msg(-1);
                            return 0;
                        }
                    }
                    return 0;
                case 8:
                    return 0;
                case 9:
                    start_change_inroom();
                    if(inrooms[myroom].status == INROOM_STOP)
                        start_game();
                    else {
                        inrooms[myroom].status = INROOM_STOP;
                        send_msg(-1, "游戏被屋主强制结束");
                        end_change_inroom();
                        kill_msg(-1);
                    }
                    return 0;
            }
            break;
        default:
            for(i=0;i<max;i++)
                if(ch==menus[i][0]) sel=i;
            break;
        }
    }while(1);
}

void join_room(int w, int spec)
{
    char buf[200],buf2[200],buf3[200],msg[200],roomname[80];
    int i,j,k,me;
    clear();
    myroom = w;
    start_change_inroom();
    if(rooms[myroom].style!=1) {
        end_change_inroom();
        return;
    }
    strcpy(roomname, rooms[myroom].name);
    signal(SIGUSR1, room_refresh);
    i=0;
    while(inrooms[myroom].peoples[i].style!=-1) i++;
    mypos = i;
    inrooms[myroom].peoples[i].style = 0;
    inrooms[myroom].peoples[i].flag = 0;
    strcpy(inrooms[myroom].peoples[i].id, getCurrentUser()->userid);
    strcpy(inrooms[myroom].peoples[i].nick, getCurrentUser()->userid);
    inrooms[myroom].peoples[i].pid = uinfo.pid;
    if(rooms[myroom].people==0 && !strcmp(rooms[myroom].creator, getCurrentUser()->userid))
        inrooms[myroom].peoples[i].flag = PEOPLE_ROOMOP;
    if(spec) inrooms[myroom].peoples[i].flag|=PEOPLE_SPECTATOR;
    rooms[myroom].people++;
    end_change_inroom();

    kill_msg(-1);
/*    sprintf(buf, "%s进入房间", getCurrentUser()->userid);
    for(i=0;i<myroom->people;i++) {
        send_msg(inrooms.peoples+i, buf);
        kill(inrooms.peoples[i].pid, SIGUSR1);
    }*/

    room_refresh(0);
    while(1){
        do{
            int ch;
            ch=-getdata(t_lines-1, 0, "输入:", buf, 70, 1, NULL, 1);
            if(rooms[myroom].style!=1) kicked = 1;
            if(kicked) goto quitgame;
            if(ch==KEY_UP) {
                selected--;
                if(selected<0) selected = rooms[myroom].people-1;
                if(ipage>selected) ipage=selected;
                if(selected>ipage+t_lines-5) ipage=selected-(t_lines-5);
                refreshit();
            }
            else if(ch==KEY_DOWN) {
                selected++;
                if(selected>=rooms[myroom].people) selected=0;
                if(ipage>selected) ipage=selected;
                if(selected>ipage+t_lines-5) ipage=selected-(t_lines-5);
                refreshit();
            }
            else if(ch==KEY_PGUP) {
                jpage+=t_lines/2;
                refreshit();
            }
            else if(ch==KEY_PGDN) {
                jpage-=t_lines/2;
                if(jpage<=0) jpage=0;
                refreshit();
            }
            else if(ch==Ctrl('T')&&inrooms[myroom].status == INROOM_DAY) {
                int pid;
                int sel;
                sel = getpeople(selected);
                if(sel==-1) continue;
                me = mypos;
                pid = inrooms[myroom].peoples[me].pid;
                if(!(inrooms[myroom].peoples[me].flag&PEOPLE_POLICE))
                    continue;
                if(!(inrooms[myroom].peoples[me].flag&PEOPLE_ALIVE))
                    continue;
                if(inrooms[myroom].peoples[me].flag&PEOPLE_TESTED) {
                    send_msg(me, "\x1b[31;1m本轮你已经侦查过了\x1b[m");
                    refreshit();
                    continue;
                }
                if(inrooms[myroom].peoples[sel].flag&PEOPLE_SPECTATOR)
                    send_msg(me, "\x1b[31;1m此人是旁观者\x1b[m");
                else if(!(inrooms[myroom].peoples[sel].flag&PEOPLE_ALIVE))
                    send_msg(me, "\x1b[31;1m此人已死\x1b[m");
                else if((inrooms[myroom].peoples[sel].flag&PEOPLE_KILLER)) {
                    inrooms[myroom].peoples[me].flag|=PEOPLE_TESTED;
                    send_msg(me, "\x1b[31;1m经过你的侦测, 发现此人是坏人!!!\x1b[m");
                }
                else {
                    inrooms[myroom].peoples[me].flag|=PEOPLE_TESTED;
                    send_msg(me, "\x1b[31;1m经过你的侦测, 发现此人是好人\x1b[m");
                }
                refreshit();
            }
            else if(ch==Ctrl('S')) {
                int pid;
                int sel;
                sel = getpeople(selected);
                if(sel==-1) continue;
                me=mypos;
                pid=inrooms[myroom].peoples[sel].pid;
                if(inrooms[myroom].peoples[me].vote==0)
                if((inrooms[myroom].peoples[me].flag&PEOPLE_ALIVE)&&
                    (((inrooms[myroom].peoples[me].flag&PEOPLE_KILLER)&&inrooms[myroom].status==INROOM_NIGHT) ||
                    inrooms[myroom].status==INROOM_DAY)) {
                    if(inrooms[myroom].peoples[sel].flag&PEOPLE_ALIVE && 
                        !(inrooms[myroom].peoples[sel].flag&PEOPLE_SPECTATOR) &&
                        sel!=me) {
                        int i,j,t1,t2,t3;
                        sprintf(buf, "\x1b[32;1m%d %s投了%d %s一票\x1b[m", me+1, inrooms[myroom].peoples[me].nick,
                            sel+1, inrooms[myroom].peoples[sel].nick);
                        start_change_inroom();
                        inrooms[myroom].peoples[me].vote = pid;
                        end_change_inroom();
                        if(inrooms[myroom].status==INROOM_NIGHT) {
                            for(i=0;i<MAX_PEOPLE;i++)
                                if(inrooms[myroom].peoples[i].style!=-1)
                                if(inrooms[myroom].peoples[i].flag&PEOPLE_KILLER||
                                    inrooms[myroom].peoples[i].flag&PEOPLE_SPECTATOR)
                                    send_msg(i, buf);
                        }
                        else {
                            send_msg(-1, buf);
                        }
checkvote:
                        t1=0; t2=0; t3=0;
                        for(i=0;i<MAX_PEOPLE;i++)
                            inrooms[myroom].peoples[i].vnum = 0;
                        for(i=0;i<MAX_PEOPLE;i++)
                        if(inrooms[myroom].peoples[i].style!=-1)
                        if(!(inrooms[myroom].peoples[i].flag&PEOPLE_SPECTATOR) &&
                            inrooms[myroom].peoples[i].flag&PEOPLE_ALIVE &&
                            (inrooms[myroom].peoples[i].flag&PEOPLE_KILLER||inrooms[myroom].status==INROOM_DAY)) {
                            for(j=0;j<MAX_PEOPLE;j++)
                                if(inrooms[myroom].peoples[j].style!=-1)
                                if(inrooms[myroom].peoples[j].pid == inrooms[myroom].peoples[i].vote)
                                    inrooms[myroom].peoples[j].vnum++;
                        }
                        for(i=0;i<MAX_PEOPLE;i++)
                        if(inrooms[myroom].peoples[i].style!=-1)
                        if(!(inrooms[myroom].peoples[i].flag&PEOPLE_SPECTATOR) &&
                            inrooms[myroom].peoples[i].flag&PEOPLE_ALIVE) {
                            if(inrooms[myroom].peoples[i].vnum>=t1) {
                                t2=t1; t1=inrooms[myroom].peoples[i].vnum;
                            }
                            else if(inrooms[myroom].peoples[i].vnum>=t2) {
                                t2=inrooms[myroom].peoples[i].vnum;
                            }
                        }
                        j=1;
                        for(i=0;i<MAX_PEOPLE;i++)
                        if(inrooms[myroom].peoples[i].style!=-1)
                        if(!(inrooms[myroom].peoples[i].flag&PEOPLE_SPECTATOR) &&
                            inrooms[myroom].peoples[i].flag&PEOPLE_ALIVE &&
                            (inrooms[myroom].peoples[i].flag&PEOPLE_KILLER||inrooms[myroom].status==INROOM_DAY))
                            if(inrooms[myroom].peoples[i].vote == 0) {
                                j=0;
                                t3++;
                            }
                        if(j || t1-t2>t3) {
                            int max=0, ok=0, maxi=0, maxpid=0;
                            for(i=0;i<MAX_PEOPLE;i++)
                                inrooms[myroom].peoples[i].vnum = 0;
                            for(i=0;i<MAX_PEOPLE;i++)
                            if(inrooms[myroom].peoples[i].style!=-1)
                            if(!(inrooms[myroom].peoples[i].flag&PEOPLE_SPECTATOR) &&
                                inrooms[myroom].peoples[i].flag&PEOPLE_ALIVE &&
                                (inrooms[myroom].peoples[i].flag&PEOPLE_KILLER||inrooms[myroom].status==INROOM_DAY)) {
                                for(j=0;j<MAX_PEOPLE;j++)
                                if(inrooms[myroom].peoples[j].style!=-1)
                                    if(inrooms[myroom].peoples[j].pid == inrooms[myroom].peoples[i].vote)
                                        inrooms[myroom].peoples[j].vnum++;
                            }
                            sprintf(buf, "投票结果:");
                            if(inrooms[myroom].status==INROOM_NIGHT) {
                                for(j=0;j<MAX_PEOPLE;j++)
                                    if(inrooms[myroom].peoples[j].style!=-1)
                                    if(inrooms[myroom].peoples[j].flag&PEOPLE_KILLER||
                                        inrooms[myroom].peoples[j].flag&PEOPLE_SPECTATOR)
                                        send_msg(j, buf);
                            }
                            else {
                                send_msg(-1, buf);
                            }
                            for(i=0;i<MAX_PEOPLE;i++)
                            if(inrooms[myroom].peoples[i].style!=-1)
                            if(!(inrooms[myroom].peoples[i].flag&PEOPLE_SPECTATOR) &&
                                inrooms[myroom].peoples[i].flag&PEOPLE_ALIVE) {
                                sprintf(buf, "%s的投票数: %d 票", 
                                    inrooms[myroom].peoples[i].nick,
                                    inrooms[myroom].peoples[i].vnum);
                                if(inrooms[myroom].peoples[i].vnum==max)
                                    ok=0;
                                if(inrooms[myroom].peoples[i].vnum>max) {
                                    max=inrooms[myroom].peoples[i].vnum;
                                    ok=1;
                                    maxi=i;
                                    maxpid=inrooms[myroom].peoples[i].pid;
                                }
                                if(inrooms[myroom].status==INROOM_NIGHT) {
                                    for(j=0;j<MAX_PEOPLE;j++)
                                        if(inrooms[myroom].peoples[j].style!=-1)
                                        if(inrooms[myroom].peoples[j].flag&PEOPLE_KILLER||
                                            inrooms[myroom].peoples[j].flag&PEOPLE_SPECTATOR)
                                            send_msg(j, buf);
                                }
                                else {
                                    send_msg(-1, buf);
                                }
                            }
                            if(!ok) {
                                sprintf(buf, "最高票数相同,请重新协商结果...");
                                if(inrooms[myroom].status==INROOM_NIGHT) {
                                    for(j=0;j<MAX_PEOPLE;j++)
                                        if(inrooms[myroom].peoples[j].style!=-1)
                                        if(inrooms[myroom].peoples[j].flag&PEOPLE_KILLER||
                                            inrooms[myroom].peoples[j].flag&PEOPLE_SPECTATOR)
                                            send_msg(j, buf);
                                }
                                else
                                    send_msg(-1, buf);
                                start_change_inroom();
                                for(j=0;j<MAX_PEOPLE;j++)
                                    inrooms[myroom].peoples[j].vote=0;
                                end_change_inroom();
                            }
                            else {
                                int a=0,b=0;
                                if(inrooms[myroom].status == INROOM_DAY)
                                    sprintf(buf, "你被大家处决了!");
                                else
                                    sprintf(buf, "你被凶手杀掉了!");
                                send_msg(maxi, buf);
                                if(inrooms[myroom].status == INROOM_DAY) {
                                    if(inrooms[myroom].peoples[maxi].flag&PEOPLE_KILLER)
                                        sprintf(buf, "坏人%s被处决了!",
                                            inrooms[myroom].peoples[maxi].nick);
                                    else
                                        sprintf(buf, "好人%s被处决了!",
                                            inrooms[myroom].peoples[maxi].nick);
                                }
                                else
                                    sprintf(buf, "%s被杀掉了!",
                                        inrooms[myroom].peoples[maxi].nick);
                                for(j=0;j<MAX_PEOPLE;j++)
                                    if(inrooms[myroom].peoples[j].style!=-1)
                                    if(j!=maxi)
                                        send_msg(j, buf);
                                start_change_inroom();
                                for(i=0;i<MAX_PEOPLE;i++)
                                    if(inrooms[myroom].peoples[i].style!=-1)
                                    if(inrooms[myroom].peoples[i].pid == maxpid)
                                        inrooms[myroom].peoples[i].flag &= ~PEOPLE_ALIVE;
                                for(i=0;i<MAX_PEOPLE;i++)
                                    if(inrooms[myroom].peoples[i].style!=-1)
                                    if(inrooms[myroom].peoples[i].flag&PEOPLE_ALIVE) {
                                        if(inrooms[myroom].peoples[i].flag&PEOPLE_KILLER) a++;
                                        else b++;
                                    }
                                if(a>0&&a>=b-1&&inrooms[myroom].status==INROOM_DAY) {
                                    inrooms[myroom].status = INROOM_STOP;
                                    send_msg(-1, "坏人获得了胜利...");
                                    save_result(1);
                                    for(j=0;j<MAX_PEOPLE;j++)
                                    if(inrooms[myroom].peoples[j].style!=-1)
                                    if(inrooms[myroom].peoples[j].flag&PEOPLE_KILLER &&
                                        inrooms[myroom].peoples[j].flag&PEOPLE_ALIVE) {
                                        sprintf(buf, "原来%s是坏人!",
                                            inrooms[myroom].peoples[j].nick);
                                        send_msg(-1, buf);
                                    }
                                }
                                else if(a==0) {
                                    inrooms[myroom].status = INROOM_STOP;
                                    send_msg(-1, "所有坏人都被处决了，好人获得了胜利...");
                                    save_result(0);
                                }
                                else if(inrooms[myroom].status == INROOM_DAY) {
                                    inrooms[myroom].status = INROOM_NIGHT;
                                    send_msg(-1, "恐怖的夜色又降临了...");
                                    for(i=0;i<MAX_PEOPLE;i++) 
                                        if(inrooms[myroom].peoples[i].style!=-1)
                                        if(inrooms[myroom].peoples[i].flag&PEOPLE_KILLER&&inrooms[myroom].peoples[i].flag&PEOPLE_ALIVE)
                                            send_msg(i, "请抓紧你的宝贵时间用\x1b[31;1mCtrl+S\x1b[m杀人...");
                                }
                                else {
                                    for(i=0;i<MAX_PEOPLE;i++)
                                        inrooms[myroom].peoples[i].flag&=(~PEOPLE_TESTED);
                                    inrooms[myroom].status = INROOM_DAY;
                                    send_msg(-1, "天亮了...");
                                }
                                for(i=0;i<MAX_PEOPLE;i++)
                                    inrooms[myroom].peoples[i].vote = 0;
                                end_change_inroom();
                            }
                        }
                        kill_msg(-1);
                    }
                    else {
                        if(sel==me)
                            send_msg(me, "\x1b[31;1m你不能选择自杀\x1b[m");
                        else if(!(inrooms[myroom].peoples[sel].flag&PEOPLE_ALIVE))
                            send_msg(me, "\x1b[31;1m此人已死\x1b[m");
                        else
                            send_msg(me, "\x1b[31;1m此人是旁观者\x1b[m");
                        refreshit();
                    }
                }
            }
            else if(ch<=0&&!buf[0]) {
                j=do_com_menu();
                if(kicked) goto quitgame;
                if(j==1) goto quitgame;
                if(j==2) if(inrooms[myroom].status!=INROOM_STOP) goto checkvote;
            }
            else if(ch<=0){
                break;
            }
        }while(1);
        start_change_inroom();
        me=mypos;
        strcpy(msg, buf);
        if(msg[0]=='/'&&msg[1]=='/') {
            i=2;
            while(msg[i]!=' '&&i<strlen(msg)) i++;
            strcpy(buf, msg+2);
            buf[i-2]=0;
            while(msg[i]==' '&&i<strlen(msg)) i++;
            buf2[0]=0; buf3[0]=0;
            if(msg[i-1]==' '&&i<strlen(msg)) {
                k=i;
                while(msg[k]!=' '&&k<strlen(msg)) k++;
                strcpy(buf2, msg+i);
                buf2[k-i]=0;
                i=k;
                while(msg[i]==' '&&i<strlen(msg)) i++;
                if(msg[i-1]==' '&&i<strlen(msg)) {
                    k=i;
                    while(msg[k]!=' '&&k<strlen(msg)) k++;
                    strcpy(buf3, msg+i);
                    buf3[k-i]=0;
                }
            }
            k=1;
            for(i=0;;i++) {
                if(!party_data[i].verb) break;
                if(!strcasecmp(party_data[i].verb, buf)) {
                    k=0;
                    sprintf(buf, "%s \x1b[1m%s\x1b[m %s", party_data[i].part1_msg, buf2[0]?buf2:"大家", party_data[i].part2_msg);
                    break;
                }
            }
            if(k)
            for(i=0;;i++) {
                if(!speak_data[i].verb) break;
                if(!strcasecmp(speak_data[i].verb, buf)) {
                    k=0;
                    sprintf(buf, "%s: %s", speak_data[i].part1_msg, buf2);
                    break;
                }
            }
            if(k)
            for(i=0;;i++) {
                if(!condition_data[i].verb) break;
                if(!strcasecmp(condition_data[i].verb, buf)) {
                    k=0;
                    sprintf(buf, "%s", condition_data[i].part1_msg);
                    break;
                }
            }

            if(k) continue;
            strcpy(buf2, buf);
            sprintf(buf, "\x1b[1m%d %s\x1b[m %s", me+1, inrooms[myroom].peoples[me].nick, buf2);
        }
        else {
            strcpy(buf2, buf);
            sprintf(buf, "%d %s: %s", me+1, inrooms[myroom].peoples[me].nick, buf2);
        }
        if(inrooms[myroom].peoples[me].flag&PEOPLE_DENYSPEAK) {
            send_msg(i, "你现在没有发言权");
        }
        else
        if(inrooms[myroom].status==INROOM_NIGHT) {
            if(inrooms[myroom].peoples[me].flag&PEOPLE_KILLER)
            for(i=0;i<MAX_PEOPLE;i++) 
            if(inrooms[myroom].peoples[i].style!=-1)
            {
                if(inrooms[myroom].peoples[i].flag&PEOPLE_KILLER||
                    inrooms[myroom].peoples[i].flag&PEOPLE_SPECTATOR) {
                    send_msg(i, buf);
                }
            }
        }
        else {
            if(!(inrooms[myroom].peoples[me].flag&PEOPLE_SPECTATOR))
            send_msg(-1, buf);
        }
        end_change_inroom();
        kill_msg(-1);
    }

quitgame:
    start_change_inroom();
    me=mypos;
    if(inrooms[myroom].peoples[me].flag&PEOPLE_ROOMOP) {
        for(i=0;i<MAX_PEOPLE;i++)
        if(inrooms[myroom].peoples[i].style!=-1)
        if(i!=me) {
            send_msg(i, "你被踢了");
        }
        rooms[myroom].style = -1;
        end_change_inroom();
        for(i=0;i<MAX_PEOPLE;i++)
            if(inrooms[myroom].peoples[i].style!=-1)
            if(i!=me)
                kill_msg(i);
        goto quitgame2;
    }
    inrooms[myroom].peoples[me].style=-1;
    rooms[myroom].people--;
    end_change_inroom();

/*    if(killer)
        sprintf(buf, "杀手%s潜逃了", buf2);
    else
        sprintf(buf, "%s离开房间", buf2);
    for(i=0;i<myroom->people;i++) {
        send_msg(inrooms.peoples+i, buf);
        kill(inrooms.peoples[i].pid, SIGUSR1);
    }*/
quitgame2:
    kicked=0;
    getdata(t_lines-1, 0, "寄回本次全部信息吗?[y/N]", buf3, 3, 1, 0, 1);
    if(toupper(buf3[0])=='Y') {
        sprintf(buf, "tmp/%d.msg", rand());
        save_msgs(buf);
        sprintf(buf2, "\"%s\"的杀人记录", roomname);
        mail_file(getCurrentUser()->userid, buf, getCurrentUser()->userid, buf2, BBSPOST_MOVE, NULL);
    }
    signal(SIGUSR1, talk_request);
}

static int room_list_refresh(struct _select_def *conf)
{
    clear();
    docmdtitle("[游戏室选单]",
              "  退出[\x1b[1;32m←\x1b[0;37m,\x1b[1;32me\x1b[0;37m] 进入[\x1b[1;32mEnter\x1b[0;37m] 选择[\x1b[1;32m↑\x1b[0;37m,\x1b[1;32m↓\x1b[0;37m] 添加[\x1b[1;32ma\x1b[0;37m] 加入[\x1b[1;32mJ\x1b[0;37m] \x1b[m      作者: \x1b[31;1mbad@smth.org\x1b[m");
    move(2, 0);
    prints("\033[0;1;37;44m    %4s %-14s %-12s %4s %4s %6s %-20s", "编号", "游戏室名称", "创建者", "人数", "最多", "锁隐旁", "话题");
    clrtoeol();
    resetcolor();
    update_endline();
    return SHOW_CONTINUE;
}

static int room_list_show(struct _select_def *conf, int i)
{
    struct room_struct * r;
    int j = room_get(i-1);
    if(j!=-1) {
        r=rooms+j;
        prints("  %3d  %-14s %-12s %3d  %3d  %2s%2s%2s %-36s", i, r->name, r->creator, r->people, r->maxpeople, (r->flag&ROOM_LOCKED)?"√":"", (r->flag&ROOM_SECRET)?"√":"", (!(r->flag&ROOM_DENYSPEC))?"√":"", r->title);
    }
    return SHOW_CONTINUE;
}

static int room_list_select(struct _select_def *conf)
{
    struct room_struct * r, * r2;
    int i=room_get(conf->pos-1), j;
    char ans[4];
    if(i==-1) return SHOW_CONTINUE;
    r = rooms+i;
    j=find_room(r->name);
    if(j==-1) {
        move(0, 0);
        clrtoeol();
        prints(" 该房间已被锁定!");
        refresh(); sleep(1);
        return SHOW_REFRESH;
    }
    r2 = rooms+j;
    if(r2->people>=r2->maxpeople&&!HAS_PERM(getCurrentUser(), PERM_SYSOP)) {
        move(0, 0);
        clrtoeol();
        prints(" 该房间人数已满");
        refresh(); sleep(1);
        return SHOW_REFRESH;
    }
    getdata(0, 0, "是否以旁观者身份进入? [y/N]", ans, 3, 1, NULL, 1);
    if(toupper(ans[0])=='Y'&&r2->flag&ROOM_DENYSPEC&&!HAS_PERM(getCurrentUser(), PERM_SYSOP)) {
        move(0, 0);
        clrtoeol();
        prints(" 该房间拒绝旁观者");
        refresh(); sleep(1);
        return SHOW_REFRESH;
    }
    join_room(find_room(r2->name), toupper(ans[0])=='Y');
    return SHOW_DIRCHANGE;
}

static int room_list_getdata(struct _select_def *conf, int pos, int len)
{
    clear_room();
    conf->item_count = room_count();
    return SHOW_CONTINUE;
}

static int room_list_prekey(struct _select_def *conf, int *key)
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

static int room_list_key(struct _select_def *conf, int key)
{
    struct room_struct r, *r2;
    int i,j;
    char name[40], ans[4];
    switch(key) {
    case 'a':
        strcpy(r.creator, getCurrentUser()->userid);
        getdata(0, 0, "房间名:", name, 13, 1, NULL, 1);
        if(!name[0]) return SHOW_REFRESH;
        if(name[0]==' '||name[strlen(name)-1]==' ') {
            move(0, 0);
            clrtoeol();
            prints(" 房间名开头结尾不能为空格");
            refresh(); sleep(1);
            return SHOW_CONTINUE;
        }
        strcpy(r.name, name);
        r.style = 1;
        r.flag = 0;
        r.people = 0;
        r.maxpeople = 100;
        strcpy(r.title, "我杀我杀我杀杀杀");
        if(add_room(&r)==-1) {
            move(0, 0);
            clrtoeol();
            prints(" 有一样名字的房间啦!");
            refresh(); sleep(1);
            return SHOW_REFRESH;
        }
        join_room(find_room(r.name), 0);
        return SHOW_DIRCHANGE;
    case 'J':
        getdata(0, 0, "房间名:", name, 12, 1, NULL, 1);
        if(!name[0]) return SHOW_REFRESH;
        if((i=find_room(name))==-1) {
            move(0, 0);
            clrtoeol();
            prints(" 没有找到该房间!");
            refresh(); sleep(1);
            return SHOW_REFRESH;
        }
        r2 = rooms+i;
        if(r2->people>=r2->maxpeople&&!HAS_PERM(getCurrentUser(), PERM_SYSOP)) {
            move(0, 0);
            clrtoeol();
            prints(" 该房间人数已满");
            refresh(); sleep(1);
            return SHOW_REFRESH;
        }
        getdata(0, 0, "是否以旁观者身份进入? [y/N]", ans, 3, 1, NULL, 1);
        if(toupper(ans[0])=='Y'&&r2->flag&ROOM_DENYSPEC&&!HAS_PERM(getCurrentUser(), PERM_SYSOP)) {
            move(0, 0);
            clrtoeol();
            prints(" 该房间拒绝旁观者");
            refresh(); sleep(1);
            return SHOW_REFRESH;
        }
        join_room(find_room(name), toupper(ans[0])=='Y');
        return SHOW_DIRCHANGE;
    case 'K':
        if(!HAS_PERM(getCurrentUser(), PERM_SYSOP)) return SHOW_CONTINUE;
        i = room_get(conf->pos-1);
        if(i!=-1) {
            r2 = rooms+i;
            r2->style = -1;
            for(j=0;j<MAX_PEOPLE;j++)
            if(inrooms[i].peoples[j].style!=-1)
                kill(inrooms[i].peoples[j].pid, SIGUSR1);
        }
        return SHOW_DIRCHANGE;
    }
    return SHOW_CONTINUE;
}

void show_top_board()
{
    FILE* fp;
    char buf[80];
    int i,j,x,y;
    clear();
    for(i=1;i<=6;i++) {
        sprintf(buf, "service/killer.%d", i);
        fp=fopen(buf, "r");
        if(!fp) return;
        for(j=0;j<7;j++) {
            if(feof(fp)) break;
            y=(i-1)%3*8+j; x=(i-1)/3*40;
            if(fgets(buf, 80, fp)==NULL) break;
            move(y, x);
            resetcolor();
            if(j==2) setfcolor(RED, 1);
            prints("%s", buf);
        }
        fclose(fp);
    }
    pressanykey();
}

int choose_room()
{
    struct _select_def grouplist_conf;
    int i;
    POINT *pts;

    clear_room();
    bzero(&grouplist_conf, sizeof(struct _select_def));
    grouplist_conf.item_count = room_count();
    if (grouplist_conf.item_count == 0) {
        grouplist_conf.item_count = 1;
    }
    pts = (POINT *) malloc(sizeof(POINT) * BBS_PAGESIZE);
    for (i = 0; i < BBS_PAGESIZE; i++) {
        pts[i].x = 2;
        pts[i].y = i + 3;
    }
    grouplist_conf.item_per_page = BBS_PAGESIZE;
    grouplist_conf.flag = LF_VSCROLL | LF_BELL | LF_LOOP | LF_MULTIPAGE;
    grouplist_conf.prompt = "◆";
    grouplist_conf.item_pos = pts;
    grouplist_conf.arg = NULL;
    grouplist_conf.title_pos.x = 0;
    grouplist_conf.title_pos.y = 0;
    grouplist_conf.pos = 1;     /* initialize cursor on the first mailgroup */
    grouplist_conf.page_pos = 1;        /* initialize page to the first one */

    grouplist_conf.on_select = room_list_select;
    grouplist_conf.show_data = room_list_show;
    grouplist_conf.pre_key_command = room_list_prekey;
    grouplist_conf.key_command = room_list_key;
    grouplist_conf.show_title = room_list_refresh;
    grouplist_conf.get_data = room_list_getdata;
    show_top_board();
    list_select_loop(&grouplist_conf);
    free(pts);
    return 0;
}

int killer_main()
{
    int i,oldmode;
    void * shm, * shm2;
    shm=attach_shm("GAMEROOM_SHMKEY", 3451, sizeof(struct room_struct)*MAX_ROOM, &i);
    rooms = shm;
    if(i) {
        for(i=0;i<MAX_ROOM;i++) {
            rooms[i].style=-1;
            rooms[i].w = 0;
            rooms[i].level = 0;
        }
    }
    shm2=attach_shm("KILLER_SHMKEY", 9578, sizeof(struct inroom_struct)*MAX_ROOM, &i);
    inrooms = shm2;
    if(i) {
        for(i=0;i<MAX_ROOM;i++)
            inrooms[i].w = 0;
    }
    oldmode = uinfo.mode;
    modify_user_mode(KILLER);
    choose_room();
    modify_user_mode(oldmode);
    shmdt(shm);
    shmdt(shm2);
    return 0;
}

