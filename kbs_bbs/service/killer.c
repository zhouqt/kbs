/******************************************************
…±»À”Œœ∑2003, ◊˜’ﬂ: bad@smth.org  Qian Wenjie
‘⁄ÀÆƒæ«Âª™bbsœµÕ≥…œ‘À––

±æ”Œœ∑ «◊‘”…»Ìº˛£¨«ÎÀÊ“‚∏¥÷∆“∆÷≤
«Î‘⁄–ﬁ∏ƒ∫ÛµƒŒƒº˛Õ∑≤ø±£¡Ù∞Ê»®–≈œ¢
******************************************************/

#define BBSMAIN
#include "bbs.h"

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
    int w; //0 - ∆Ω√Ò §¿˚ 1 - …± ÷ §¿˚
    time_t t;
    int peoplet;
    char id[MAX_PEOPLE][IDLEN+2];
    int st[MAX_PEOPLE]; // 0 - ªÓ◊≈∆Ω√Ò 1 - À¿¡À∆Ω√Ò 2 - ªÓ◊≈…± ÷ 3 - À¿¡À…± ÷ 4 - ∆‰À˚«Èøˆ
};

int myroom, mypos;

extern int kicked;

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
    sprintf(filename, "home/%c/%s/.INROOMMSG%d", toupper(currentuser->userid[0]), currentuser->userid, uinfo.pid);
    fp = fopen(filename, "r");
    if(fp) {
        while(!feof(fp)) {
            if(fgets(buf, 79, fp)==NULL) break;
            if(buf[0]) {
                if(!strncmp(buf, "ƒ„±ªÃﬂ¡À", 8)) kicked=1;
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
    char *verb;                 /* ∂Ø¥  */
    char *part1_msg;            /* ΩÈ¥  */
    char *part2_msg;            /* ∂Ø◊˜ */
};

struct action party_data[] = {
    {"?", "∫‹“…ªÛµƒø¥◊≈", ""},
    {"admire", "∂‘", "µƒæ∞—ˆ÷Æ«È”Ã»ÁÃœÃœΩ≠ÀÆ¡¨√‡≤ªæ¯"},
    {"agree", "ÕÍ»´Õ¨“‚", "µƒø¥∑®"},
    {"bearhug", "»»«Èµƒ”µ±ß", ""},
    {"bless", "◊£∏£", "–ƒœÎ ¬≥…"},
    {"bow", "±œπ™±œæ¥µƒœÚ", "æœπ™"},
    {"bye", "ø¥◊≈", "µƒ±≥”∞£¨∆‡»ª¿·œ¬°£…Ì∫Ûµƒ ’“Ùª˙¥´¿¥µÀ¿ˆæ˝µƒ∏Ë…˘:\n\"[31m∫Œ»’æ˝‘Ÿ¿¥.....[m\""},
    {"caress", "«·«·µƒ∏ß√˛", ""},
    {"cat", "œÒ÷ª–°√®∞„µÿ“¿ŸÀ‘⁄", "µƒª≥¿Ô»ˆΩø°£"},
    {"cringe", "œÚ", "±∞π™«¸œ•£¨“°Œ≤∆Ú¡Ø"},
    {"cry", "œÚ", "∫øﬂ˚¥Ûøﬁ"},
    {"comfort", "Œ¬—‘∞≤Œø", ""},
    {"clap", "œÚ", "»»¡“πƒ’∆"},
    {"dance", "¿≠◊≈", "µƒ ÷ÙÊÙÊ∆ŒË"},
    {"dogleg", "∂‘", "π∑Õ»"},
    {"drivel", "∂‘÷¯", "¡˜ø⁄ÀÆ"},
    {"dunno", "µ…¥Û—€æ¶£¨ÃÏ’ÊµÿŒ £∫", "£¨ƒ„Àµ ≤˜·Œ“≤ª∂Æ“Æ... :("},
    {"faint", "‘Œµπ‘⁄", "µƒª≥¿Ô"},
    {"fear", "∂‘", "¬∂≥ˆ≈¬≈¬µƒ±Ì«È"},
    {"fool", "«Î¥Ûº“◊¢“‚", "’‚∏ˆ¥Û∞◊≥’....\n’Ê «ÃÏ…œ…Ÿ”–....»Àº‰Ωˆ¥ÊµƒªÓ±¶....\n≤ªø¥Ã´ø…œß¡À£°"},
    {"forgive", "¥Û∂»µƒ∂‘", "Àµ£∫À„¡À£¨‘≠¡¬ƒ„¡À"},
    {"giggle", "∂‘÷¯", "…µ…µµƒ¥Ù–¶"},
    {"grin", "∂‘", "¬∂≥ˆ–∞∂Òµƒ–¶»›"},
    {"growl", "∂‘", "≈ÿœ¯≤ª“—"},
    {"hand", "∏˙", "Œ’ ÷"},
    {"hammer", "æŸ∆∫√¥Û∫√¥ÛµƒÃ˙¥∏£°£°Õ€£°Õ˘",
     "Õ∑…œ”√¡¶“ª«√£°\n***************\n*  5000000 Pt *\n***************\n      | |      °Ô°Ó°Ô°Ó°Ô°Ó\n      | |         ∫√∂‡µƒ–«–«”¥\n      |_|"},
    {"heng", "ø¥∂º≤ªø¥", "“ª—€£¨ ∫ﬂ¡À“ª…˘£¨∏ﬂ∏ﬂµƒ∞—Õ∑—Ô∆¿¥¡À,≤ª–º“ªπÀµƒ—˘◊”..."},
    {"hug", "«·«·µÿ”µ±ß", ""},
    {"idiot", "Œﬁ«Èµÿ≥‹–¶", "µƒ≥’¥Ù°£"},
    {"kick", "∞—", "ÃﬂµƒÀ¿»•ªÓ¿¥"},
    {"kiss", "«·Œ«", "µƒ¡≥º’"},
    {"laugh", "¥Û…˘≥∞–¶", ""},
    {"lovelook", "¿≠◊≈", "µƒ ÷£¨Œ¬»·µÿƒ¨ƒ¨∂‘ ”°£ƒøπ‚÷–‘–∫¨◊≈«ß÷÷»·«È£¨ÕÚ∞„√€“‚"},
    {"nod", "œÚ", "µ„Õ∑≥∆ «"},
    {"nudge", "”√ ÷÷‚∂•", "µƒ∑ ∂«◊”"},
    {"oh", "∂‘", "Àµ£∫°∞≈∂£¨Ω¥◊”∞°£°°±"},
    {"pad", "«·≈ƒ", "µƒºÁ∞Ú"},
    {"papaya", "«√¡À«√", "µƒƒæπœƒ‘¥¸"},
    {"pat", "«·«·≈ƒ≈ƒ", "µƒÕ∑"},
    {"pinch", "”√¡¶µƒ∞—", "≈°µƒ∫⁄«‡"},
    {"puke", "∂‘◊≈", "Õ¬∞°Õ¬∞°£¨æ›ÀµÕ¬∂‡º∏¥ŒæÕœ∞πﬂ¡À"},
    {"punch", "∫›∫›◊·¡À", "“ª∂Ÿ"},
    {"pure", "∂‘", "¬∂≥ˆ¥ø’Êµƒ–¶»›"},
    {"qmarry", "œÚ", "”¬∏“µƒπÚ¡Àœ¬¿¥:\n\"ƒ„‘∏“‚ºﬁ∏¯Œ“¬£ø\"\n---’Ê «”¬∆¯ø…ºŒ∞°"},
    {"report", "ÕµÕµµÿ∂‘", "Àµ£∫°∞±®∏ÊŒ“∫√¬£ø°±"},
    {"shrug", "ŒﬁƒŒµÿœÚ", "À ¡ÀÀ ºÁ∞Ú"},
    {"sigh", "∂‘", "Ãæ¡À“ªø⁄∆¯"},
    {"slap", "≈æ≈æµƒ∞Õ¡À", "“ª∂Ÿ∂˙π‚"},
    {"smooch", "”µŒ«÷¯", ""},
    {"snicker", "∫Ÿ∫Ÿ∫Ÿ..µƒ∂‘", "«‘–¶"},
    {"sniff", "∂‘", "‡Õ÷Æ“‘±«"},
    {"sorry", "Õ¥øﬁ¡˜ÃÈµƒ«Î«Û", "‘≠¡¬"},
    {"spank", "”√∞Õ’∆¥Ú", "µƒÕŒ≤ø"},
    {"squeeze", "ΩÙΩÙµÿ”µ±ß÷¯", ""},
    {"thank", "œÚ", "µ¿–ª"},
    {"tickle", "πæﬂ¥!πæﬂ¥!…¶", "µƒ—˜"},
    {"waiting", "…Ó«Èµÿ∂‘", "Àµ£∫√øƒÍ√ø‘¬µƒ√ø“ªÃÏ£¨√ø∑÷√ø√ÎŒ“∂º‘⁄’‚¿Ôµ»◊≈ƒ„"},
    {"wake", "≈¨¡¶µƒ“°“°", "£¨‘⁄∆‰∂˙±ﬂ¥ÛΩ–£∫°∞øÏ–—–—£¨ª·◊≈¡πµƒ£°°±"},
    {"wave", "∂‘÷¯", "∆¥√¸µƒ“° ÷"},
    {"welcome", "»»¡“ª∂”≠", "µƒµΩ¿¥"},
    {"wink", "∂‘", "…Ò√ÿµƒ’£’£—€æ¶"},
    {"xixi", "Œ˚Œ˚µÿ∂‘", "–¶¡Àº∏…˘"},
    {"zap", "∂‘", "∑ËøÒµƒπ•ª˜"},
    {"inn", "À´—€±•∫¨◊≈¿·ÀÆ£¨ŒﬁπºµƒÕ˚◊≈", ""},
    {"mm", "…´√–√–µƒ∂‘", "Œ ∫√£∫°∞√¿√º∫√°´°´°´°´°±°£¥Û…´¿«∞°£°£°£°"},
    {"disapp", "’‚œ¬√ª∏„Õ∑¿≤£¨Œ™ ≤√¥", "π√ƒÔ∂‘Œ“’‚∏ˆ‘Ï–ÕÕÍ»´√ª∑¥”¶£ø√ª∞Ï∑®£°"},
    {"miss", "’Ê≥œµƒÕ˚◊≈", "£∫Œ“œÎƒÓƒ„Œ“’ÊµƒœÎƒÓƒ„Œ“Ã´--œÎƒÓƒ„¡À!ƒ„œ‡≤ªœ‡–≈?"},
    {"buypig", "÷∏◊≈", "£∫°∞’‚∏ˆ÷ÌÕ∑∏¯Œ“«–“ª∞Î£¨–ª–ª£°°±"},
    {"rascal", "∂‘", "¥ÛΩ–£∫°∞ƒ„’‚∏ˆ≥Ù¡˜√•£°£°£°£°£°£°£°£°£°£°£°£°£°£°£°£°°±"},
    {"qifu", "–°◊Ï“ª±‚£¨∂‘", "øﬁµ¿£∫°∞ƒ„∆€∏∫Œ“£¨ƒ„∆€∏∫Œ“£°£°£°°±"},
    {"wa", "∂‘", "¥ÛΩ–“ª…˘£∫°∞Õ€Õ€Õ€Õ€Õ€Õ€ø·±◊¡À“Æ£°£°£°£°£°£°£°£°£°£°£°£°£°£°°±"},
    {"feibang", "‡∏£≠£≠£° ÏπÈ Ï£¨", "ƒ„’‚—˘¬“Ω≤ª∞£¨Œ““ª—˘ø…“‘∏Êƒ„ªŸ∞˘£¨π˛£°"},
    {"badman", "÷∏◊≈", "µƒƒ‘¥¸,  ÷ŒË◊„µ∏µƒ∫∞:°∞ …±~~ ÷~~‘⁄~~’‚~~¿Ô~~~°±"}, 
    {NULL, NULL, NULL}
};

struct action speak_data[] = {
    {"ask", "—ØŒ ", NULL},
    {"chant", "∏ËÀÃ", NULL},
    {"cheer", "∫»≤…", NULL},
    {"chuckle", "«·–¶", NULL},
    {"curse", "÷‰¬Ó", NULL},
    {"demand", "“™«Û", NULL},
    {"frown", "ıæ√º", NULL},
    {"groan", "…Î“˜", NULL},
    {"grumble", "∑¢¿Œ…ß", NULL},
    {"hum", "‡´‡´◊‘”Ô", NULL},
    {"moan", "±ØÃæ", NULL},
    {"notice", "◊¢“‚", NULL},
    {"order", "√¸¡Ó", NULL},
    {"ponder", "…ÚÀº", NULL},
    {"pout", "‡Ÿ÷¯◊ÏÀµ", NULL},
    {"pray", "∆Ìµª", NULL},
    {"request", "ø“«Û", NULL},
    {"shout", "¥ÛΩ–", NULL},
    {"sing", "≥™∏Ë", NULL},
    {"smile", "Œ¢–¶", NULL},
    {"smirk", "ºŸ–¶", NULL},
    {"swear", "∑¢ ƒ", NULL},
    {"tease", "≥∞–¶", NULL},
    {"whimper", "Œÿ— µƒÀµ", NULL},
    {"yawn", "π˛«∑¡¨ÃÏ", NULL},
    {"yell", "¥Û∫∞", NULL},
    {NULL, NULL, NULL}
};

struct action condition_data[] = {
    {":D", "¿÷µƒ∫œ≤ª¬£◊Ï", NULL},
    {":)", "¿÷µƒ∫œ≤ª¬£◊Ï", NULL},
    {":P", "¿÷µƒ∫œ≤ª¬£◊Ï", NULL},
    {":(", "¿÷µƒ∫œ≤ª¬£◊Ï", NULL},
    {"applaud", "≈æ≈æ≈æ≈æ≈æ≈æ≈æ....", NULL},
    {"blush", "¡≥∂º∫Ï¡À", NULL},
    {"cough", "ø»¡Àº∏…˘", NULL},
    {"faint", "ﬂ€µ±“ª…˘£¨‘Œµπ‘⁄µÿ", NULL},
    {"happy", "µƒ¡≥…œ¬∂≥ˆ¡À–“∏£µƒ±Ì«È£¨≤¢—ß≥‘±•¡Àµƒ÷Ì∫ﬂ∫ﬂ¡À∆¿¥", NULL},
    {"lonely", "“ª∏ˆ»À◊¯‘⁄∑øº‰¿Ô£¨∞ŸŒﬁ¡ƒ¿µ£¨œ£Õ˚À≠¿¥≈„≈„°£°£°£°£", NULL},
    {"luck", "Õ€£°∏£∆¯¿≤£°", NULL},
    {"puke", "’Ê∂Ò–ƒ£¨Œ“Ã˝¡À∂ºœÎÕ¬", NULL},
    {"shake", "“°¡À“°Õ∑", NULL},
    {"sleep", "Zzzzzzzzzz£¨’ÊŒﬁ¡ƒ£¨∂ºøÏÀØ÷¯¡À", NULL},
    {"so", "æÕΩ¥◊”!!", NULL},
    {"strut", "¥Û“°¥Û∞⁄µÿ◊ﬂ", NULL},
    {"tongue", "Õ¬¡ÀÕ¬…‡Õ∑", NULL},
    {"think", "Õ·÷¯Õ∑œÎ¡À“ªœ¬", NULL},
    {"wawl", "æ™ÃÏ∂Øµÿµƒøﬁ", NULL},
    {"goodman", "”√º∞∆‰Œﬁπºµƒ±Ì«Èø¥◊≈¥Ûº“: °∞Œ“’Êµƒ «∫√»À“Æ~~°±", NULL},
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
            sprintf(buf, "%sµÙœﬂ¡À", inrooms[myroom].peoples[i].nick);
            send_msg(-1, buf);
            start_change_inroom();
            inrooms[myroom].peoples[i].style=-1;
            rooms[myroom].people--;
            if(inrooms[myroom].peoples[i].flag&PEOPLE_ROOMOP) {
                for(k=0;k<MAX_PEOPLE;k++) 
                if(inrooms[myroom].peoples[k].style!=-1&&!(inrooms[myroom].peoples[k].flag&PEOPLE_SPECTATOR))
                {
                    inrooms[myroom].peoples[k].flag|=PEOPLE_ROOMOP;
                    sprintf(buf, "%s≥…Œ™–¬∑ø÷˜", inrooms[myroom].peoples[k].nick);
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
        if(!strcmp(rooms[i].creator, currentuser->userid))
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
    if(r->level&currentuser->userlevel!=r->level) return 0;
    if(r->style!=1) return 0;
    if(r->flag&ROOM_SECRET&&!HAS_PERM(currentuser, PERM_SYSOP)) return 0;
    return 1;
}

int can_enter(struct room_struct * r)
{
    if(r->style==-1) return 0;
    if(r->level&currentuser->userlevel!=r->level) return 0;
    if(r->style!=1) return 0;
    if(r->flag&ROOM_LOCKED&&!HAS_PERM(currentuser, PERM_SYSOP)) return 0;
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
    prints("[44;33;1m ∑øº‰:[36m%-12s[33mª∞Ã‚:[36m%-40s[33m◊¥Ã¨:[36m%6s",
        rooms[myroom].name, rooms[myroom].title, (inrooms[myroom].status==INROOM_STOP?"Œ¥ø™ º":(inrooms[myroom].status==INROOM_NIGHT?"∫⁄“π÷–":"¥Û∞◊ÃÏ")));
    clrtoeol();
    resetcolor();
    setfcolor(YELLOW, 1);
    move(1,0);
    prints("®q\x1b[32mÕÊº“\x1b[33m°™°™°™°™°™®r®q\x1b[32m—∂œ¢\x1b[33m°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™®r");
    move(t_lines-2,0);
    prints("®t°™°™°™°™°™°™°™®s®t°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™®s");
    for(i=2;i<=t_lines-3;i++) {
        move(i,0); prints("©¶");
        move(i,16); prints("©¶");
        move(i,18); prints("©¶");
        move(i,78); prints("©¶");
    }
    me=mypos;
    for(i=2;i<=t_lines-3;i++) 
    if(ipage+i-2>=0&&ipage+i-2<rooms[myroom].people) {
        j=getpeople(ipage+i-2);
        if(j==-1) continue;
        if(inrooms[myroom].status!=INROOM_STOP)
        if(inrooms[myroom].peoples[j].flag&PEOPLE_KILLER && (inrooms[myroom].peoples[me].flag&PEOPLE_KILLER ||
            inrooms[myroom].peoples[me].flag&PEOPLE_SPECTATOR&&rooms[myroom].flag&ROOM_SPECBLIND ||
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
            inrooms[myroom].status == INROOM_NIGHT &&
            (inrooms[myroom].peoples[me].flag&PEOPLE_KILLER ||
            inrooms[myroom].peoples[me].flag&PEOPLE_SPECTATOR&&rooms[myroom].flag&ROOM_SPECBLIND))
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
            inrooms[myroom].status == INROOM_NIGHT &&
            (inrooms[myroom].peoples[me].flag&PEOPLE_KILLER ||
            inrooms[myroom].peoples[me].flag&PEOPLE_SPECTATOR)) {
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
        if(!strcmp(ss, "ƒ„±ªÃﬂ¡À")) kicked = 1;
        move(i,20);
        if(ss) prints("%s", ss);
        resetcolor();
    }
}

extern int RMSG;

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
        send_msg(me, "\x1b[31;1m÷¡…Ÿ6»À≤Œº”≤≈ƒ‹ø™ º”Œœ∑\x1b[m");
        end_change_inroom();
        refreshit();
        return;
    }
    if(totalk==0) totalk=((double)total/6+0.5);
    if(totalk>=total/4) totalk=total/4;
    if(totalc>=total/6) totalc=total/6;
    if(totalk>total) {
        send_msg(me, "\x1b[31;1m◊‹»À ˝…Ÿ”⁄“™«Ûµƒªµ»À»À ˝,Œﬁ∑®ø™ º”Œœ∑\x1b[m");
        end_change_inroom();
        refreshit();
        return;
    }
    if(totalc==0)
        sprintf(buf, "\x1b[31;1m”Œœ∑ø™ º¿≤! »À»∫÷–≥ˆœ÷¡À%d∏ˆªµ»À\x1b[m", totalk);
    else
        sprintf(buf, "\x1b[31;1m”Œœ∑ø™ º¿≤! »À»∫÷–≥ˆœ÷¡À%d∏ˆªµ»À, %d∏ˆæØ≤Ï\x1b[m", totalk, totalc);
    send_msg(-1, buf);
    for(i=0;i<totalk;i++) {
        do{
            j=rand()%MAX_PEOPLE;
        }while(inrooms[myroom].peoples[j].style==-1 || inrooms[myroom].peoples[j].flag&PEOPLE_KILLER || inrooms[myroom].peoples[j].flag&PEOPLE_SPECTATOR);
        inrooms[myroom].peoples[j].flag |= PEOPLE_KILLER;
        send_msg(j, "ƒ„◊ˆ¡À“ª∏ˆŒﬁ≥‹µƒªµ»À");
        send_msg(j, "”√ƒ„µƒº‚µ∂(\x1b[31;1mCtrl+S\x1b[m)—°‘Òƒ„“™≤–∫¶µƒ»À∞…...");
    }
    for(i=0;i<totalc;i++) {
        do{
            j=rand()%MAX_PEOPLE;
        }while(inrooms[myroom].peoples[j].style==-1 || inrooms[myroom].peoples[j].flag&PEOPLE_KILLER || inrooms[myroom].peoples[j].flag&PEOPLE_POLICE || inrooms[myroom].peoples[j].flag&PEOPLE_SPECTATOR);
        inrooms[myroom].peoples[j].flag |= PEOPLE_POLICE;
        send_msg(j, "ƒ„◊ˆ¡À“ªŒªπ‚»Ÿµƒ»À√ÒæØ≤Ï");
        send_msg(j, "∞◊ÃÏ«Î”√ƒ„µƒæØ∞Ù(\x1b[31;1mCtrl+T\x1b[m)—°‘Òƒ„ª≥“…µƒ»À...");
    }
    for(i=0;i<MAX_PEOPLE;i++) 
    if(inrooms[myroom].peoples[i].style!=-1)
    if(!(inrooms[myroom].peoples[i].flag&PEOPLE_SPECTATOR))
    {
        inrooms[myroom].peoples[i].flag |= PEOPLE_ALIVE;
        if(!(inrooms[myroom].peoples[i].flag&PEOPLE_KILLER))
            send_msg(i, "œ÷‘⁄ «ÕÌ…œ...");
    }
    inrooms[myroom].status = INROOM_NIGHT;
    end_change_inroom();
    kill_msg(-1);
}

#define menust 10
int do_com_menu()
{
    char menus[menust][15]=
        {"0-∑µªÿ","1-ÕÀ≥ˆ”Œœ∑","2-∏ƒ√˚◊÷", "3-ÕÊº“¡–±Ì", "4-∏ƒª∞Ã‚", "5-…Ë÷√∑øº‰", "6-ÃﬂÕÊº“", "7-∑¢—‘»®", "8-ΩªªªŒª÷√", "9-ø™ º”Œœ∑"};
    int menupos[menust],i,j,k,sel=0,ch,max=0,me,offset=0;
    char buf[80];
    if(inrooms[myroom].status != INROOM_STOP)
        strcpy(menus[9], "9-Ω· ¯”Œœ∑");
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
                        send_msg(me, "ƒ„ªπ‘⁄”Œœ∑,≤ªƒ‹ÕÀ≥ˆ");
                        refreshit();
                        return 0;
                    }
                    move(t_lines-1, 0);
                    resetcolor();
                    clrtoeol();
                    getdata(t_lines-1, 0, "»∑»œÕÀ≥ˆ£ø [y/N] ", buf, 3, 1, 0, 1);
                    if(kicked) return 0;
                    if(toupper(buf[0])!='Y') return 0;
                    return 1;
                case 2:
                    move(t_lines-1, 0);
                    resetcolor();
                    clrtoeol();
                    getdata(t_lines-1, 0, "«Î ‰»Î√˚◊÷:", buf, 13, 1, 0, 1);
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
                            prints(" √˚◊÷≤ª∑˚∫œπÊ∑∂");
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
                            prints(" “—”–»À”√’‚∏ˆ√˚◊÷¡À");
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
                    getdata(t_lines-1, 0, "«Î ‰»Îª∞Ã‚:", buf, 31, 1, 0, 1);
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
                    getdata(t_lines-1, 0, "«Î ‰»Î∑øº‰◊Ó¥Û»À ˝:", buf, 30, 1, 0, 1);
                    if(kicked) return 0;
                    if(buf[0]) {
                        i=atoi(buf);
                        if(i>0&&i<=100) {
                            rooms[myroom].maxpeople = i;
                            sprintf(buf, "Œ›÷˜…Ë÷√∑øº‰◊Ó¥Û»À ˝Œ™%d", i);
                            send_msg(-1, buf);
                        }
                    }
                    move(t_lines-1, 0);
                    clrtoeol();
                    getdata(t_lines-1, 0, "…Ë÷√Œ™“˛≤ÿ∑øº‰? [Y/N]", buf, 30, 1, 0, 1);
                    if(kicked) return 0;
                    buf[0]=toupper(buf[0]);
                    if(buf[0]=='Y'||buf[0]=='N') {
                        if(buf[0]=='Y') rooms[myroom].flag|=ROOM_SECRET;
                        else rooms[myroom].flag&=~ROOM_SECRET;
                        sprintf(buf, "Œ›÷˜…Ë÷√∑øº‰Œ™%s", (buf[0]=='Y')?"“˛≤ÿ":"≤ª“˛≤ÿ");
                        send_msg(-1, buf);
                    }
                    move(t_lines-1, 0);
                    clrtoeol();
                    getdata(t_lines-1, 0, "…Ë÷√Œ™À¯∂®∑øº‰? [Y/N]", buf, 30, 1, 0, 1);
                    if(kicked) return 0;
                    buf[0]=toupper(buf[0]);
                    if(buf[0]=='Y'||buf[0]=='N') {
                        if(buf[0]=='Y') rooms[myroom].flag|=ROOM_LOCKED;
                        else rooms[myroom].flag&=~ROOM_LOCKED;
                        sprintf(buf, "Œ›÷˜…Ë÷√∑øº‰Œ™%s", (buf[0]=='Y')?"À¯∂®":"≤ªÀ¯∂®");
                        send_msg(-1, buf);
                    }
                    move(t_lines-1, 0);
                    clrtoeol();
                    getdata(t_lines-1, 0, "…Ë÷√Œ™æ‹æ¯≈‘π€’ﬂµƒ∑øº‰? [Y/N]", buf, 30, 1, 0, 1);
                    if(kicked) return 0;
                    buf[0]=toupper(buf[0]);
                    if(buf[0]=='Y'||buf[0]=='N') {
                        if(buf[0]=='Y') rooms[myroom].flag|=ROOM_DENYSPEC;
                        else rooms[myroom].flag&=~ROOM_DENYSPEC;
                        sprintf(buf, "Œ›÷˜…Ë÷√∑øº‰Œ™%s", (buf[0]=='Y')?"æ‹æ¯≈‘π€":"≤ªæ‹æ¯≈‘π€");
                        send_msg(-1, buf);
                    }
                    move(t_lines-1, 0);
                    clrtoeol();
                    getdata(t_lines-1, 0, "…Ë÷√Œ™≈‘π€’ﬂŒﬁ∑®ø¥º˚…± ÷æØ≤Ïµƒ∑øº‰? [Y/N]", buf, 30, 1, 0, 1);
                    if(kicked) return 0;
                    buf[0]=toupper(buf[0]);
                    if(buf[0]=='Y'||buf[0]=='N') {
                        if(buf[0]=='Y') rooms[myroom].flag|=ROOM_SPECBLIND;
                        else rooms[myroom].flag&=~ROOM_SPECBLIND;
                        sprintf(buf, "Œ›÷˜…Ë÷√∑øº‰Œ™%s", (buf[0]=='Y')?"≈‘π€Œﬁ∑®ø¥º˚…± ÷æØ≤Ï":"≈‘π€ø…“‘ø¥º˚…± ÷æØ≤Ï");
                        send_msg(-1, buf);
                    }
                    move(t_lines-1, 0);
                    clrtoeol();
                    getdata(t_lines-1, 0, "…Ë÷√ªµ»Àµƒ ˝ƒø:", buf, 30, 1, 0, 1);
                    if(kicked) return 0;
                    if(buf[0]) {
                        i=atoi(buf);
                        if(i>=0&&i<=10) {
                            inrooms[myroom].killernum = i;
                            sprintf(buf, "Œ›÷˜…Ë÷√∑øº‰ªµ»À ˝Œ™%d", i);
                            send_msg(-1, buf);
                        }
                    }
                    move(t_lines-1, 0);
                    clrtoeol();
                    getdata(t_lines-1, 0, "…Ë÷√æØ≤Ïµƒ ˝ƒø:", buf, 30, 1, 0, 1);
                    if(kicked) return 0;
                    if(buf[0]) {
                        i=atoi(buf);
                        if(i>=0&&i<=2) {
                            inrooms[myroom].policenum = i;
                            sprintf(buf, "Œ›÷˜…Ë÷√∑øº‰æØ≤Ï ˝Œ™%d", i);
                            send_msg(-1, buf);
                        }
                    }
                    kill_msg(-1);
                    return 0;
                case 6:
                    move(t_lines-1, 0);
                    resetcolor();
                    clrtoeol();
                    getdata(t_lines-1, 0, "«Î ‰»Î“™Ãﬂµƒ–Ú∫≈( ˝◊÷):", buf, 4, 1, 0, 1);
                    if(kicked) return 0;
                    if(buf[0]) {
                        i = atoi(buf)-1;
                        if(i>=0&&i<MAX_PEOPLE&&inrooms[myroom].peoples[i].style!=-1&&inrooms[myroom].peoples[i].pid!=uinfo.pid) {
                            inrooms[myroom].peoples[i].flag&=~PEOPLE_ALIVE;
                            send_msg(i, "ƒ„±ªÃﬂ¡À");
                            kill_msg(i);
                            return 2;
                        }
                    }
                    return 0;
                case 7:
                    move(t_lines-1, 0);
                    resetcolor();
                    clrtoeol();
                    getdata(t_lines-1, 0, "«Î ‰»Î“™Ω˚÷π/ª÷∏¥∑¢—‘»®µƒ–Ú∫≈( ˝◊÷):", buf, 4, 1, 0, 1);
                    if(kicked) return 0;
                    if(buf[0]) {
                        i = atoi(buf)-1;
                        if(i>=0&&i<MAX_PEOPLE&&inrooms[myroom].peoples[i].style!=-1&&inrooms[myroom].peoples[i].pid!=uinfo.pid) {
                            if(inrooms[myroom].peoples[i].flag&PEOPLE_DENYSPEAK)
                                inrooms[myroom].peoples[i].flag&=~PEOPLE_DENYSPEAK;
                            else
                                inrooms[myroom].peoples[i].flag|=PEOPLE_DENYSPEAK;
                            sprintf(buf, "%d %s±ª%s¡À∑¢—‘»®", i+1, inrooms[myroom].peoples[i].nick, (inrooms[myroom].peoples[i].flag&PEOPLE_DENYSPEAK)?"Ω˚÷π":"ª÷∏¥");
                            send_msg(-1, buf);
                            kill_msg(-1);
                            return 0;
                        }
                    }
                    return 0;
                case 9:
                    start_change_inroom();
                    if(inrooms[myroom].status == INROOM_STOP)
                        start_game();
                    else {
                        inrooms[myroom].status = INROOM_STOP;
                        send_msg(-1, "”Œœ∑±ªŒ›÷˜«ø÷∆Ω· ¯");
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
    int i,j,k,killer,me;
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
    strcpy(inrooms[myroom].peoples[i].id, currentuser->userid);
    strcpy(inrooms[myroom].peoples[i].nick, currentuser->userid);
    inrooms[myroom].peoples[i].pid = uinfo.pid;
    if(rooms[myroom].people==0 && !strcmp(rooms[myroom].creator, currentuser->userid))
        inrooms[myroom].peoples[i].flag = PEOPLE_ROOMOP;
    if(spec) inrooms[myroom].peoples[i].flag|=PEOPLE_SPECTATOR;
    rooms[myroom].people++;
    end_change_inroom();

    kill_msg(-1);
/*    sprintf(buf, "%sΩ¯»Î∑øº‰", currentuser->userid);
    for(i=0;i<myroom->people;i++) {
        send_msg(inrooms.peoples+i, buf);
        kill(inrooms.peoples[i].pid, SIGUSR1);
    }*/

    room_refresh(0);
    while(1){
        do{
            int ch;
            ch=-getdata(t_lines-1, 0, " ‰»Î:", buf, 70, 1, NULL, 1);
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
                    send_msg(me, "\x1b[31;1m±æ¬÷ƒ„“—æ≠’Ï≤Èπ˝¡À\x1b[m");
                    refreshit();
                    continue;
                }
                if(inrooms[myroom].peoples[sel].flag&PEOPLE_SPECTATOR)
                    send_msg(me, "\x1b[31;1m¥À»À «≈‘π€’ﬂ\x1b[m");
                else if(!(inrooms[myroom].peoples[sel].flag&PEOPLE_ALIVE))
                    send_msg(me, "\x1b[31;1m¥À»À“—À¿\x1b[m");
                else if((inrooms[myroom].peoples[sel].flag&PEOPLE_KILLER)) {
                    inrooms[myroom].peoples[me].flag|=PEOPLE_TESTED;
                    send_msg(me, "\x1b[31;1mæ≠π˝ƒ„µƒ’Ï≤‚, ∑¢œ÷¥À»À «ªµ»À!!!\x1b[m");
                }
                else {
                    inrooms[myroom].peoples[me].flag|=PEOPLE_TESTED;
                    send_msg(me, "\x1b[31;1mæ≠π˝ƒ„µƒ’Ï≤‚, ∑¢œ÷¥À»À «∫√»À\x1b[m");
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
                if(inrooms[myroom].peoples[me].flag&PEOPLE_ALIVE&&
                    (inrooms[myroom].peoples[me].flag&PEOPLE_KILLER&&inrooms[myroom].status==INROOM_NIGHT ||
                    inrooms[myroom].status==INROOM_DAY)) {
                    if(inrooms[myroom].peoples[sel].flag&PEOPLE_ALIVE && 
                        !(inrooms[myroom].peoples[sel].flag&PEOPLE_SPECTATOR) &&
                        sel!=me) {
                        int i,j,t1,t2,t3;
                        sprintf(buf, "\x1b[32;1m%d %sÕ∂¡À%d %s“ª∆±\x1b[m", me+1, inrooms[myroom].peoples[me].nick,
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
                            int max=0, ok=0, maxi, maxpid;
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
                            sprintf(buf, "Õ∂∆±Ω·π˚:");
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
                                sprintf(buf, "%sµƒÕ∂∆± ˝: %d ∆±", 
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
                                sprintf(buf, "◊Ó∏ﬂ∆± ˝œ‡Õ¨,«Î÷ÿ–¬–≠…ÃΩ·π˚...");
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
                                    sprintf(buf, "ƒ„±ª¥Ûº“¥¶æˆ¡À!");
                                else
                                    sprintf(buf, "ƒ„±ª–◊ ÷…±µÙ¡À!");
                                send_msg(maxi, buf);
                                if(inrooms[myroom].status == INROOM_DAY) {
                                    if(inrooms[myroom].peoples[maxi].flag&PEOPLE_KILLER)
                                        sprintf(buf, "ªµ»À%s±ª¥¶æˆ¡À!",
                                            inrooms[myroom].peoples[maxi].nick);
                                    else
                                        sprintf(buf, "∫√»À%s±ª¥¶æˆ¡À!",
                                            inrooms[myroom].peoples[maxi].nick);
                                }
                                else
                                    sprintf(buf, "%s±ª…±µÙ¡À!",
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
                                    send_msg(-1, "ªµ»ÀªÒµ√¡À §¿˚...");
                                    save_result(1);
                                    for(j=0;j<MAX_PEOPLE;j++)
                                    if(inrooms[myroom].peoples[j].style!=-1)
                                    if(inrooms[myroom].peoples[j].flag&PEOPLE_KILLER &&
                                        inrooms[myroom].peoples[j].flag&PEOPLE_ALIVE) {
                                        sprintf(buf, "‘≠¿¥%s «ªµ»À!",
                                            inrooms[myroom].peoples[j].nick);
                                        send_msg(-1, buf);
                                    }
                                }
                                else if(a==0) {
                                    inrooms[myroom].status = INROOM_STOP;
                                    send_msg(-1, "À˘”–ªµ»À∂º±ª¥¶æˆ¡À£¨∫√»ÀªÒµ√¡À §¿˚...");
                                    save_result(0);
                                }
                                else if(inrooms[myroom].status == INROOM_DAY) {
                                    inrooms[myroom].status = INROOM_NIGHT;
                                    send_msg(-1, "ø÷≤¿µƒ“π…´”÷Ωµ¡Ÿ¡À...");
                                    for(i=0;i<MAX_PEOPLE;i++) 
                                        if(inrooms[myroom].peoples[i].style!=-1)
                                        if(inrooms[myroom].peoples[i].flag&PEOPLE_KILLER&&inrooms[myroom].peoples[i].flag&PEOPLE_ALIVE)
                                            send_msg(i, "«Î◊•ΩÙƒ„µƒ±¶πÛ ±º‰”√\x1b[31;1mCtrl+S\x1b[m…±»À...");
                                }
                                else {
                                    for(i=0;i<MAX_PEOPLE;i++)
                                        inrooms[myroom].peoples[i].flag&=(~PEOPLE_TESTED);
                                    inrooms[myroom].status = INROOM_DAY;
                                    send_msg(-1, "ÃÏ¡¡¡À...");
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
                            send_msg(me, "\x1b[31;1mƒ„≤ªƒ‹—°‘Ò◊‘…±\x1b[m");
                        else if(!(inrooms[myroom].peoples[sel].flag&PEOPLE_ALIVE))
                            send_msg(me, "\x1b[31;1m¥À»À“—À¿\x1b[m");
                        else
                            send_msg(me, "\x1b[31;1m¥À»À «≈‘π€’ﬂ\x1b[m");
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
                    sprintf(buf, "%s \x1b[1m%s\x1b[m %s", party_data[i].part1_msg, buf2[0]?buf2:"¥Ûº“", party_data[i].part2_msg);
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
            send_msg(i, "ƒ„œ÷‘⁄√ª”–∑¢—‘»®");
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
            send_msg(i, "ƒ„±ªÃﬂ¡À");
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
        sprintf(buf, "…± ÷%s«±Ã”¡À", buf2);
    else
        sprintf(buf, "%s¿Îø™∑øº‰", buf2);
    for(i=0;i<myroom->people;i++) {
        send_msg(inrooms.peoples+i, buf);
        kill(inrooms.peoples[i].pid, SIGUSR1);
    }*/
quitgame2:
    kicked=0;
    getdata(t_lines-1, 0, "ºƒªÿ±æ¥Œ»´≤ø–≈œ¢¬?[y/N]", buf3, 3, 1, 0, 1);
    if(toupper(buf3[0])=='Y') {
        sprintf(buf, "tmp/%d.msg", rand());
        save_msgs(buf);
        sprintf(buf2, "\"%s\"µƒ…±»Àº«¬º", roomname);
        mail_file(currentuser->userid, buf, currentuser->userid, buf2, BBSPOST_MOVE, NULL);
    }
    signal(SIGUSR1, talk_request);
}

static int room_list_refresh(struct _select_def *conf)
{
    clear();
    docmdtitle("[”Œœ∑ “—°µ•]",
              "  ÕÀ≥ˆ[\x1b[1;32m°˚\x1b[0;37m,\x1b[1;32me\x1b[0;37m] Ω¯»Î[\x1b[1;32mEnter\x1b[0;37m] —°‘Ò[\x1b[1;32m°¸\x1b[0;37m,\x1b[1;32m°˝\x1b[0;37m] ÃÌº”[\x1b[1;32ma\x1b[0;37m] º”»Î[\x1b[1;32mJ\x1b[0;37m] \x1b[m      ◊˜’ﬂ: \x1b[31;1mbad@smth.org\x1b[m");
    move(2, 0);
    prints("[0;1;37;44m    %4s %-14s %-12s %4s %4s %6s %-20s", "±‡∫≈", "”Œœ∑ “√˚≥∆", "¥¥Ω®’ﬂ", "»À ˝", "◊Ó∂‡", "À¯“˛≈‘", "ª∞Ã‚");
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
        prints("  %3d  %-14s %-12s %3d  %3d  %2s%2s%2s %-36s", i, r->name, r->creator, r->people, r->maxpeople, (r->flag&ROOM_LOCKED)?"°Ã":"", (r->flag&ROOM_SECRET)?"°Ã":"", (!(r->flag&ROOM_DENYSPEC))?"°Ã":"", r->title);
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
        prints(" ∏√∑øº‰“—±ªÀ¯∂®!");
        refresh(); sleep(1);
        return SHOW_REFRESH;
    }
    r2 = rooms+j;
    if(r2->people>=r2->maxpeople&&!HAS_PERM(currentuser, PERM_SYSOP)) {
        move(0, 0);
        clrtoeol();
        prints(" ∏√∑øº‰»À ˝“—¬˙");
        refresh(); sleep(1);
        return SHOW_REFRESH;
    }
    getdata(0, 0, " «∑Ò“‘≈‘π€’ﬂ…Ì∑›Ω¯»Î? [y/N]", ans, 3, 1, NULL, 1);
    if(toupper(ans[0])=='Y'&&r2->flag&ROOM_DENYSPEC&&!HAS_PERM(currentuser, PERM_SYSOP)) {
        move(0, 0);
        clrtoeol();
        prints(" ∏√∑øº‰æ‹æ¯≈‘π€’ﬂ");
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
        strcpy(r.creator, currentuser->userid);
        getdata(0, 0, "∑øº‰√˚:", name, 13, 1, NULL, 1);
        if(!name[0]) return SHOW_REFRESH;
        if(name[0]==' '||name[strlen(name)-1]==' ') {
            move(0, 0);
            clrtoeol();
            prints(" ∑øº‰√˚ø™Õ∑Ω·Œ≤≤ªƒ‹Œ™ø’∏Ò");
            refresh(); sleep(1);
            return SHOW_CONTINUE;
        }
        strcpy(r.name, name);
        r.style = 1;
        r.flag = 0;
        r.people = 0;
        r.maxpeople = 100;
        strcpy(r.title, "Œ“…±Œ“…±Œ“…±…±…±");
        if(add_room(&r)==-1) {
            move(0, 0);
            clrtoeol();
            prints(" ”–“ª—˘√˚◊÷µƒ∑øº‰¿≤!");
            refresh(); sleep(1);
            return SHOW_REFRESH;
        }
        join_room(find_room(r.name), 0);
        return SHOW_DIRCHANGE;
    case 'J':
        getdata(0, 0, "∑øº‰√˚:", name, 12, 1, NULL, 1);
        if(!name[0]) return SHOW_REFRESH;
        if((i=find_room(name))==-1) {
            move(0, 0);
            clrtoeol();
            prints(" √ª”–’“µΩ∏√∑øº‰!");
            refresh(); sleep(1);
            return SHOW_REFRESH;
        }
        r2 = rooms+i;
        if(r2->people>=r2->maxpeople&&!HAS_PERM(currentuser, PERM_SYSOP)) {
            move(0, 0);
            clrtoeol();
            prints(" ∏√∑øº‰»À ˝“—¬˙");
            refresh(); sleep(1);
            return SHOW_REFRESH;
        }
        getdata(0, 0, " «∑Ò“‘≈‘π€’ﬂ…Ì∑›Ω¯»Î? [y/N]", ans, 3, 1, NULL, 1);
        if(toupper(ans[0])=='Y'&&r2->flag&ROOM_DENYSPEC&&!HAS_PERM(currentuser, PERM_SYSOP)) {
            move(0, 0);
            clrtoeol();
            prints(" ∏√∑øº‰æ‹æ¯≈‘π€’ﬂ");
            refresh(); sleep(1);
            return SHOW_REFRESH;
        }
        join_room(find_room(name), toupper(ans[0])=='Y');
        return SHOW_DIRCHANGE;
    case 'K':
        if(!HAS_PERM(currentuser, PERM_SYSOP)) return SHOW_CONTINUE;
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
    grouplist_conf.prompt = "°Ù";
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
}

