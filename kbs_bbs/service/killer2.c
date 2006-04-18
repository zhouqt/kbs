/******************************************************
杀人游戏2003, 作者: bad@smth.org  Qian Wenjie
修改者: znight@smth.org
在水木清华bbs系统上运行

本游戏是自由软件，请随意复制移植
请在修改后的文件头部保留版权信息
******************************************************/
/*****************************************************

Modified by znight@smth, 01/14/2004
改动：
(1) 分开了玩家和旁观者，玩家序号1~30,旁观者31~100，同时取消旁观者标志；
(2) 修正了关于管理员的几个问题，管理员退出不会结束游戏，可以转交管理员权限。同时取消管理员标志；
(3) 禁止快速说话（间隔不小于1秒）；
(4) 说话颜色加了#1~#6；
(5) 只能选择玩家（选项不能移动到旁观者）；
(6) 游戏结果可自动转贴；
(7) 游戏结束后自动贴出警察和杀手名单；
(8) 游戏结束后自动贴出警察查人顺序；

南开规则方面：
(1) 投票加了确认(ctrl+s选择，ctrl+t确认)；
(2) 辩护和指证时，杀手可以ctrl+s崩溃，再按一次取消；
(3) 指证时可更改指证对象，结果在ctrl+t时才会显示；
(4) 显示是否投了票；
(5) 任何时刻，最后的杀手崩溃立即结束游戏；
******************************************************/

#include "service.h"
#include "bbs.h"

SMTH_API struct user_info uinfo;
SMTH_API int t_lines;

// 是否南开BBS
#undef NANKAI
#undef QNGY

#ifdef QNGY
// 是否自动转贴
#define AUTOPOST
#endif

#ifdef AUTOPOST
#define KILLERBOARD "Killer"
#endif // AUTOPOST

#ifndef NANKAI

#define K_HAS_PERM HAS_PERM
#define CURRENTUSER getCurrentUser()
SMTH_API int kicked;

#define DEFAULT_SIGNAL talk_request

#define k_getdata getdata

#define k_attach_shm attach_shm

#define k_mail_file mail_file

int strlen2(char *s)
{
	int p;
	int inansi;
	p=0;
	inansi=0;
	while (*s)
	{
		if (*s=='\x1b')
		{
			inansi=1;
		}
		else
		if (inansi && *s=='m')
		{
			inansi=0;
		}
		else
		if (!inansi)
		{
			p++;
		}
		s++;
	}
	return p;
}

#else // NANKAI

#ifndef BBSPOST_MOVE
#define BBSPOST_MOVE -1
#endif // POSTFILE_MOVE

#define DEFAULT_SIGNAL SIG_DFL

#define k_attach_shm attach_shm2

#define K_HAS_PERM(x,y) HAS_PERM(y)
#define CURRENTUSER (&currentuser)
extern unsigned char scr_cols;

#define BLACK 0
#define RED 1
#define GREEN 2
#define YELLOW 3
#define BLUE 4
#define PINK 5
#define CYAN 6
#define WHITE 7

int kicked;

int k_getdata(int line,int col,char * prompt,char * buf,int len,int echo,void *nouse,int clearlabel)
{
	buf[0] = '\0';
	getdata(line, col, prompt, buf, len, echo, clearlabel);
}

int k_mail_file(char *fromid, char *tmpfile, char *userid, char *title, int unlinkmode, struct fileheader *fh)
{
	mail_file(tmpfile, userid, title);
	unlink(tmpfile);
}

#endif // NANKAI

#define k_setfcolor(x,y) kf_setfcolor(disp,x,y)

void kf_setfcolor(char * disp, int i,int j)
{
	char str[20];
	if (j)
		sprintf(str,"\x1b[%d;%dm",i+30,j);
	else
		sprintf(str,"\x1b[%dm",i+30);
	strcat(disp,str);
}

#define k_resetcolor() strcat(disp,"\x1b[m")

#define MAX_ROOM 100
#define MAX_PEOPLE 100
#define MAX_PLAYER 30
#define MAX_MSG 2000

#define MAX_KILLER 10
#define MAX_POLICE 10

#define ROOM_LOCKED 01
#define ROOM_SECRET 02
#define ROOM_DENYSPEC 04
#define ROOM_SPECBLIND 010

struct room_struct {
	int w;
	int style;				  /* 0 - chat room 1 - killer room */
	char name[14];
	char title[NAMELEN];
	int op;
	unsigned int level;
	int flag;
	int numplayer;
	int maxplayer;
	int numspectator;
};

#define PEOPLE_KILLER 0x1
#define PEOPLE_POLICE 0x2

#define PEOPLE_ALIVE 0x100
#define PEOPLE_DENYSPEAK 0x200
#define PEOPLE_VOTED  0x400
#define PEOPLE_HIDEKILLER 0x800
#define PEOPLE_SURRENDER 0x1000

#define PEOPLE_TESTED 0x8000

#define SIGN_KILLER "*"
#define SIGN_POLICE "@"

struct people_struct {
	int style;
	char id[IDLEN + 2];
	char nick[NAMELEN];
	int flag;
	int pid;
	int vote;
	int vnum;
};

// 2晚上->3死者遗言->4指证->5辩护->6投票->7被关留言(被关者是杀手无留言)
#define INROOM_STOP 1
//晚上
#define INROOM_NIGHT 2
//白天
#define INROOM_DAY 3
//指证阶段
#define INROOM_CHECK 4
//辩护阶段
#define INROOM_DEFENCE 5
//投票阶段
#define INROOM_VOTE 6
//死者留遗言
#define INROOM_DARK 7

struct inroom_struct {
	int w;
	int status;
	
	int killernum;
	int policenum;
	
	// add for nk
	int turn; // 轮到某人发言
	int round; // round of vote
	int victim; // be killed
	int informant; // be inspected
	
	int nokill;
	char voted[MAX_PLAYER];
	int numvoted;
	// end for nk
	struct people_struct peoples[MAX_PEOPLE];
	char msgs[MAX_MSG][100];
	int msgpid[MAX_MSG];
	int msgi;
	
	char killers[MAX_PLAYER];
	char polices[MAX_PLAYER];
	
	char seq_detect[MAX_PLAYER];
	
	char gamename[80];
};

struct room_struct *rooms;
struct inroom_struct *inrooms;

struct killer_record {	
	int w;					  //0 - 平民胜利 1 - 杀手胜利
	time_t t;
	int peoplet;
	char id[MAX_PLAYER][IDLEN + 2];
	int st[MAX_PLAYER];		 // 0 - 活着平民 1 - 死了平民 2 - 活着杀手 3 - 死了杀手 4 - 其他情况
};

int myroom, mypos;

#define RINFO inrooms[myroom]
#define PINFO(x) RINFO.peoples[x]

time_t lasttalktime;
int denytalk;

void player_drop(int d);
void kill_msg(int u);

#ifdef AUTOPOST

char * ap_tmpname()
{
	static char fn[80];
	sprintf(fn,"tmp/killer%d.tmp",myroom);
	return fn;
}

#endif // AUTOPOST

void safe_substr(char * d, const char * s ,int n)
{
	int i;
	for(i=0;*s && i<n ;i++,d++,s++)
	{
		*d=*s;
		if (*s&0x80)
		{
			d++;
			s++;
			i++;
			*d=*s;
		}
	}
	if (i>n)
		*(d-2)=0;
	else
		*d=0;
}

void save_result(int w)
{
	int fd;
	struct flock ldata;
	struct killer_record r;
	int i, j;
	char filename[80] = "service/.KILLERRESULT";

#ifdef AUTOPOST
	post_file(CURRENTUSER,"",ap_tmpname(),KILLERBOARD,RINFO.gamename,0,1,getSession());
	unlink(ap_tmpname());
#endif // AUTOPOST

	memset(&r,0,sizeof(r));
	r.t = time(0);
	r.w = w;
	r.peoplet = 0;
	j = 0;
	for (i = 0; i < MAX_PLAYER; i++)
		if (PINFO(i).style != -1)
		{
			strcpy(r.id[j], PINFO(i).id);
			r.st[j] = 4;
			if (!(PINFO(i).flag & PEOPLE_KILLER))
			{
				if (PINFO(i).flag & PEOPLE_ALIVE)
					r.st[j] = 0;
				else
					r.st[j] = 1;
			}
			else
			{
				if (PINFO(i).flag & PEOPLE_ALIVE)
					r.st[j] = 2;
				else
					r.st[j] = 3;
			}

			j++;
			r.peoplet++;
		}
	if ((fd = open(filename, O_WRONLY | O_CREAT, 0644)) != -1)
	{
		ldata.l_type = F_WRLCK;
		ldata.l_whence = 0;
		ldata.l_len = 0;
		ldata.l_start = 0;
		if (fcntl(fd, F_SETLKW, &ldata) != -1)
		{
			lseek(fd, 0, SEEK_END);
			write(fd, &r, sizeof(struct killer_record));
			ldata.l_type = F_UNLCK;
			fcntl(fd, F_SETLKW, &ldata);
		}
		close(fd);
	}
}

/*
void load_msgs()
{
	FILE* fp;
	int i;
	char filename[80], buf[80];
	msgst=0;
	sprintf(filename, "home/%c/%s/.INROOMMSG%d", toupper(CURRENTUSER->userid[0]), CURRENTUSER->userid, uinfo.pid);
	fp = fopen(filename, "r");
	if(fp) {
		while(!feof(fp))
		{
			if(fgets(buf, 79, fp)==NULL)
				break;
			if(buf[0])
			{
				if(!strncmp(buf, "你被踢了", 8)) kicked=1;
				if(msgst==200)
				{
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
}
*/

void start_change_inroom()
{
	if (RINFO.w)
		sleep(0);
	RINFO.w = 1;
}

void end_change_inroom()
{
	RINFO.w = 0;
}

struct action {
	char *verb;				 /* 动词 */
	char *part1_msg;			/* 介词 */
	char *part2_msg;			/* 动作 */
};

struct action party_data[] = {
	{"?", "很疑惑的看着", ""},
	{"admire", "对", "的景仰之情犹如滔滔江水连绵不绝"},
	{"agree", "完全同意", "的看法"},
	{"bearhug", "热情的拥抱", ""},
	{"bless", "祝福", "心想事成"},
	{"bow", "毕躬毕敬的向", "鞠躬"},
	{"bye", "看着", "的背影，凄然泪下。身后的收音机传来邓丽君的歌声:\n\"\x1b[31m何日君再来.....\x1b[m\""},
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
	 "头上用力一敲！\n***************\n*  5000000 Pt *\n***************\n	  | |	  ★☆★☆★☆\n	  | |		 好多的星星哟\n	  |_|"},
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

void send_msg(int u, char *msg)
{
	int i, j, k, f;
	int maxi;
	char buf[200], buf2[200], buf3[80];

	if (strlen(msg)>199)
		msg[199]=0;

	strcpy(buf, msg);

	for (i = 0; i <= 6; i++)
	{
		buf3[0] = '%';
		buf3[1] = i + 48;
		buf3[2] = 0;
		while (strstr(buf, buf3) != NULL)
		{
			strcpy(buf2, buf);
			k = strstr(buf, buf3) - buf;
			buf2[k] = 0;
			k += 2;
			sprintf(buf, "%s\x1b[3%dm%s", buf2, (i > 0) ? i : 7, buf2 + k);
		}
	}
	for (i = 0; i <= 6; i++)
	{
		buf3[0] = '#';
		buf3[1] = i + 48;
		buf3[2] = 0;
		while (strstr(buf, buf3) != NULL)
		{
			strcpy(buf2, buf);
			k = strstr(buf, buf3) - buf;
			buf2[k] = 0;
			k += 2;
			sprintf(buf, "%s\x1b[3%d;1m%s", buf2, (i > 0) ? i : 7, buf2 + k);
		}
	}

	while (strchr(buf, '\n') != NULL)
	{
		i = strchr(buf, '\n') - buf;
		buf[i] = 0;
		send_msg(u, buf);
		strcpy(buf2, buf + i + 1);
		strcpy(buf, buf2);
	}
 
	j = 0;
	f = 0;
	for (i = 0; i < strlen(buf); i++)
	{
		if (buf[i] == '\x1b')
			f = 1;
		if (f)
		{
			if (isalpha(buf[i]))
				f = 0;
			continue;
		}
		if (j >= 54 || i>=96)
			break;
		j++;
		if (buf[i]&0x80)
		{
			i++;
			j++;
		}
	}
	maxi=i;
	if (maxi < strlen(buf) && maxi != 0)
	{
		buf2[0]=' ';
		strcpy(buf2+1,buf+maxi);
		buf[maxi] = 0;
	}
	else
		buf2[0]=0;
		   
	j = MAX_MSG;
	if (RINFO.msgs[(MAX_MSG - 1 + RINFO.msgi) % MAX_MSG][0] == 0)
		for (i = 0; i < MAX_MSG; i++)
			if (RINFO.msgs[(i + RINFO.msgi) % MAX_MSG][0] == 0)
			{
				j = (i + RINFO.msgi) % MAX_MSG;
				break;
			}
	if (j == MAX_MSG)
	{
		strcpy(RINFO.msgs[RINFO.msgi], buf);
		if (u == -1)
			RINFO.msgpid[RINFO.msgi] = -1;
		else
			RINFO.msgpid[RINFO.msgi] = PINFO(u).pid;
		RINFO.msgi = (RINFO.msgi + 1) % MAX_MSG;
	}
	else
	{
		strcpy(RINFO.msgs[j], buf);
		if (u == -1)
			RINFO.msgpid[j] = u;
		else
			RINFO.msgpid[j] = PINFO(u).pid;
	}
	
#ifdef AUTOPOST
	if (u==-1)
	{
		struct flock ldata;
		int fd;
		fd = open(ap_tmpname(), O_WRONLY | O_CREAT | O_APPEND, 0644);
		if (fd>=0)
		{
			ldata.l_type = F_WRLCK;
			ldata.l_whence = 0;
			ldata.l_len = 0;
			ldata.l_start = 0;
			if (fcntl(fd, F_SETLKW, &ldata) != -1)
			{
				lseek(fd, 0, SEEK_END);
				write(fd, buf, strlen(buf));
				write(fd,"\n",1);
				ldata.l_type = F_UNLCK;
				fcntl(fd, F_SETLKW, &ldata);
			}
			close(fd);			
		}
	}
#endif	// AUTOPOST

	if (buf2[0])
		send_msg(u,buf2);
}

void check_users(int u)
{
	int i,j,k;
	char buf[80];
	start_change_inroom();
	for (i = 0; i < MAX_PEOPLE; i++)
		if (PINFO(i).style != -1)
		{
			if (u==-1)
				j=kill(PINFO(i).pid,SIGUSR1);
			else
				j=kill(PINFO(i).pid,0);
			if (j == -1)
			{
				sprintf(buf, "\x1b[34;1m%d %s掉线了\x1b[m",i+1,PINFO(i).nick);
				send_msg(-1,buf);
				player_drop (i); 
		 
				k = -1;
			}
		}
	if (rooms[myroom].op>=0 && PINFO(rooms[myroom].op).style==-1)
	{
		for (k = 0; k < MAX_PEOPLE; k++)
			if (PINFO(k).style != -1)
			{
				rooms[myroom].op=k;
				sprintf(buf, "\x1b[34;1m%d %s成为新房主\x1b[m", k+1,PINFO(k).nick);
				send_msg(-1, buf);
				kill_msg(-1);
				break;
			}
		if (k>=MAX_PEOPLE)
		{
			rooms[myroom].op=-1;
			rooms[myroom].style = -1;
			send_msg(-1, "房间解散了");
			kill_msg(-1);
		}
	}

	end_change_inroom();
}

void kill_msg(int u)
{
	check_users(u);
	if (u!=-1)
	{
		if (PINFO(u).style!=-1)
			kill(PINFO(u).pid,SIGUSR1);
	}
}

int add_room(struct room_struct *r)
{
	int i, j;

	for (i = 0; i < MAX_ROOM; i++)
		if (rooms[i].style == 1)
		{
			if (!strcmp(rooms[i].name, r->name))
				return -1;
			if (rooms[i].op!=-1 
				&& !strcmp(inrooms[i].peoples[rooms[i].op].id, CURRENTUSER->userid))
				return -1;
		}
	for (i = 0; i < MAX_ROOM; i++)
		if (rooms[i].style == -1)
		{
			memcpy(rooms + i, r, sizeof(struct room_struct));
			inrooms[i].status = INROOM_STOP;
			inrooms[i].killernum = 0;
			inrooms[i].msgi = 0;
			inrooms[i].policenum = 0;
			inrooms[i].w = 0;
			for (j = 0; j < MAX_MSG; j++)
				inrooms[i].msgs[j][0] = 0;
			for (j = 0; j < MAX_PEOPLE; j++)
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
	if(!strcmp(rooms[i].name, r->name))
	{
		rooms[i].name[0]=0;
		break;
	}
	return 0;
}
*/

void clear_room()
{
	int i;

	for (i = 0; i < MAX_ROOM; i++)
		if ((rooms[i].style != -1) && (rooms[i].numplayer + rooms[i].numspectator == 0))
			rooms[i].style = -1;
}

int can_see(struct room_struct *r)
{
	if (r->style == -1)
		return 0;
	if ((r->level & CURRENTUSER->userlevel) != r->level)
		return 0;
	if (r->style != 1)
		return 0;
	if (r->flag & ROOM_SECRET && !K_HAS_PERM(CURRENTUSER, PERM_SYSOP))
		return 0;
	return 1;
}

int can_enter(struct room_struct *r)
{
	if (r->style == -1)
		return 0;
	if ((r->level & CURRENTUSER->userlevel) != r->level)
		return 0;
	if (r->style != 1)
		return 0;
	if ((r->flag & ROOM_LOCKED) && !K_HAS_PERM(CURRENTUSER, PERM_SYSOP))
		return 0;
	return 1;
}

int room_count()
{
	int i, j = 0;

	for (i = 0; i < MAX_ROOM; i++)
		if (can_see(rooms + i))
			j++;
	return j;
}

int room_get(int w)
{
	int i, j = 0;

	for (i = 0; i < MAX_ROOM; i++) {
		if (can_see(rooms + i))
		{
			if (w == j)
				return i;
			j++;
		}
	}
	return -1;
}

int find_room(char *s)
{
	int i;
	struct room_struct *r2;

	for (i = 0; i < MAX_ROOM; i++)
	{
		r2 = rooms + i;
		if (!can_enter(r2))
			continue;
		if (!strcmp(r2->name, s))
			return i;
	}
	return -1;
}

int selected = 0, ipage = 0, jpage = 0;

int getpeople(int i)
{
	int j, k = 0;

	for (j = 0; j < MAX_PEOPLE; j++)
	{
		if (PINFO(j).style == -1)
			continue;
		if (i == k)
			return j;
		k++;
	}
	return -1;
}

int get_msgt()
{
	int i, j = 0, k;

	for (i = 0; i < MAX_MSG; i++)
	{
		if (RINFO.msgs[(i + RINFO.msgi) % MAX_MSG][0] == 0)
			break;
		k = RINFO.msgpid[(i + RINFO.msgi) % MAX_MSG];
		if (k == -1 || k == uinfo.pid)
			j++;
	}
	return j;
}

char *get_msgs(int s)
{
	int i, j = 0, k;
	char *ss;

	for (i = 0; i < MAX_MSG; i++) 
	{
		if (RINFO.msgs[(i + RINFO.msgi) % MAX_MSG][0] == 0)
			break;
		k = RINFO.msgpid[(i + RINFO.msgi) % MAX_MSG];
		if (k == -1 || k == uinfo.pid)
		{
			if (j == s)
			{
				ss = RINFO.msgs[(i + RINFO.msgi) % MAX_MSG];
				return ss;
			}
			j++;
		}
	}
	return NULL;
}

void save_msgs(char *s)
{
	FILE *fp;
	int i;

	fp = fopen(s, "w");
	if (fp == NULL)
		return;
	for (i = 0; i < get_msgt(); i++)
		fprintf(fp, "%s\n", get_msgs(i));
	fclose(fp);
}

void refreshit()
{
	int i, j, me, msgst,col;
	char buf[30], buf3[30], *ss, disp[500];
	static int inrefresh=0;
	
	if (inrefresh)
		return;
		
	inrefresh=1;
 
	for (i = 0; i < t_lines - 1; i++)
	{
		move(i, 0);
		clrtoeol();
	}
	switch(RINFO.status)
	{
	case INROOM_STOP:
		ss="未";
		break;
	case INROOM_NIGHT:
		ss="夜";
		break;
	case INROOM_DAY:
		ss="晨";
		break;
	case INROOM_CHECK:
		ss="指";
		break;
	case INROOM_DEFENCE:
		ss="辨";
		break;
	case INROOM_VOTE:
		ss="投";
		break;
	case INROOM_DARK:
		ss="昏";
		break;
    default:
        ss="";
        break;
	}
	move(0, 0);
	prints("\x1b[44;33;1m 房间:\x1b[36m%-12s\x1b[33m话题:\x1b[36m%-40s\x1b[33m状态:\x1b[36m%2s \x1b[33mOP:\x1b[36m%2d",
		   rooms[myroom].name,
		   rooms[myroom].title,
		   ss,
		   rooms[myroom].op+1);
	clrtoeol();
	move(1, 0);
	prints("\x1b[m\x1b[33;1m╭\x1b[32m玩家\x1b[33m—————╮╭\x1b[32m讯息\x1b[33m———————————————————————————╮");
	move(t_lines - 2, 0);
	prints("\x1b[33;1m╰———————╯╰—————————————————————————————╯");
/*	for (i = 2; i <= t_lines - 3; i++)
	{
		move(i, 0);
		prints("│");
		move(i, 16);
		prints("│");
		move(i, 18);
		prints("│");
		move(i, 78);
		prints("│");
	}
*/
	msgst = get_msgt();
	me = mypos;
	for (i = 2; i <= t_lines - 3; i++)
	{
		col=2;
		strcpy(disp,"\x1b[m\x1b[33;1m│");
		if (ipage + i - 2 >= 0 && ipage + i - 2 < rooms[myroom].numplayer+rooms[myroom].numspectator)
		{
			j = getpeople(ipage + i - 2);
			if (j == -1)
			{
				while (col<16)
				{
		  			strcat(disp," ");
		  			col++;
		  		}
			}
			else
			if (j>=MAX_PLAYER)
			{
				k_resetcolor();
				move(i, 2);
				k_setfcolor(GREEN, 0);
				strcat(disp,"O");
				col++;
			}
			else
			if (RINFO.status != INROOM_STOP)
			{
				if ((PINFO(j).flag & PEOPLE_KILLER)
					&& ((me<MAX_PLAYER && (PINFO(me).flag & PEOPLE_KILLER)) // 我是玩家，并且也是杀手
						|| (me>=MAX_PLAYER && !(rooms[myroom].flag & ROOM_SPECBLIND)) // 我是旁观者，不禁止旁观看见杀手警察
						|| !(PINFO(j).flag & PEOPLE_ALIVE))) // 死了的杀手
				{
					if (!(PINFO(j).flag & PEOPLE_HIDEKILLER)
						|| (me<MAX_PLAYER && (PINFO(me).flag & PEOPLE_KILLER)))
					{
						k_setfcolor(RED, 1);
						strcat(disp,SIGN_KILLER);
						col++;
					}
				}
				if ((PINFO(j).flag & PEOPLE_POLICE)
					&& ((me<MAX_PLAYER && (PINFO(me).flag & PEOPLE_POLICE))  // 我是玩家，并且也是警察
					|| (me>=MAX_PLAYER && !(rooms[myroom].flag & ROOM_SPECBLIND)))) // 我是旁观者，不禁止旁观看见杀手警察
				{
					k_setfcolor(CYAN, 1);
					strcat(disp,SIGN_POLICE);
					col++;
				}
				while (col<3)
				{
					strcat(disp," ");
					col++;
				}
				if (!(PINFO(j).flag & PEOPLE_ALIVE))
				{
					k_setfcolor(BLUE, 1);
					strcat(disp,"X");
					col++;
				}
				if ((RINFO.status==INROOM_VOTE)
					&& (PINFO(j).flag & PEOPLE_ALIVE)
					&& (PINFO(j).vote != -1))
				{
					k_setfcolor(YELLOW, 0);
					strcat(disp,"v");
					col++;
				}
			}
			while (col<4)
			{
				strcat(disp," ");
				col++;
			}
			if (ipage + i - 2 == selected)
				k_setfcolor(RED, 1);
			else
				k_resetcolor();

			sprintf(buf,"%2d",(j+1)%100);
			strcat(disp,buf);
			col+=2;

			if ((RINFO.status == INROOM_CHECK || RINFO.status == INROOM_DEFENCE || RINFO.status == INROOM_VOTE)
				&& j<MAX_PLAYER
				&& (PINFO(j).flag & PEOPLE_ALIVE)
				&& (PINFO(j).flag & PEOPLE_VOTED))
			{
				k_setfcolor(CYAN, 1);
				strcat(disp,"$");
				col++;
			}
			else
			{
				strcat(disp," ");
				col++;
			}
			
			if (ipage + i - 2 == selected)
				k_setfcolor(RED, 1);
			else
				k_resetcolor();
			safe_substr(buf3,PINFO(j).nick,9);
			sprintf(buf, "%-9.9s",buf3);
			strcat(disp,buf);
			col+=9;
		}
		while (col<16)
		{
			strcat(disp," ");
			col++;
		}
		strcat(disp, "\x1b[1;33m││\x1b[m");
		col+=4;
		if (msgst - 1 - (t_lines - 3 - i) - jpage >= 0)
		{
			char *ss = get_msgs(msgst - 1 - (t_lines - 3 - i) - jpage);

			if (ss)
			{
				if (!strcmp(ss, "你被踢了"))
					kicked = 1;
				strcat(disp, ss);
				col+=strlen2(ss);
			}
		}
		while (col<78)
		{
			strcat(disp," ");
			col++;
		}
		strcat(disp, "\x1b[m\x1b[1;33m│\x1b[m");
		move(i, 0);
		prints("%s",disp);
	}
	inrefresh=0;
}

SMTH_API int RMSG;

void room_refresh(int signo)
{
	int y, x;

	signal(SIGUSR1, room_refresh);

	if (RMSG)
		return;
	if (rooms[myroom].style != 1)
		kicked = 1;

	getyx(&y, &x);
	refreshit();
	move(y, x);
	refresh();
}

void show_killerpolice()
{
	char buf[200],buf2[20];
	int i;
	strcpy(buf,"\x1b[34;1m杀手：\x1b[m");
	for(i=0;i<MAX_PLAYER;i++)
	{
		if (RINFO.killers[i]==-1)
			break;
		if (i!=0)
			strcat(buf,", ");
		sprintf(buf2,"%d",RINFO.killers[i]+1);
		strcat(buf,buf2);
	}
	send_msg(-1,buf);
	
	if (RINFO.policenum>0)
	{
		strcpy(buf,"\x1b[34;1m警察：\x1b[m");
		for(i=0;i<MAX_PLAYER;i++)
		{
			if (RINFO.polices[i]==-1)
				break;
			if (i!=0)
				strcat(buf,", ");
			sprintf(buf2,"%d",RINFO.polices[i]+1);
			strcat(buf,buf2);
		}
		send_msg(-1,buf);
		
		strcpy(buf,"\x1b[34;1m调查顺序：\x1b[m");
		for(i=0;i<MAX_PLAYER;i++)
		{
			if (RINFO.seq_detect[i]==-1)
				break;
			if (i!=0)
				strcat(buf,", ");
			sprintf(buf2,"%d",RINFO.seq_detect[i]+1);
			strcat(buf,buf2);
		}
		send_msg(-1,buf);
	}
}

int check_win()
{
	int tc,tk,tp;
	char buf[200];
	int i;
	tc=0;
	tk=0;
	tp=0;
	for(i=0;i<MAX_PLAYER;i++)
		if (PINFO(i).style!=-1
			&& (PINFO(i).flag & PEOPLE_ALIVE))
		{
			if (PINFO(i).flag & PEOPLE_KILLER)
			{
				if (!(PINFO(i).flag & PEOPLE_SURRENDER))
					tk++;
			}
			else
				if (PINFO(i).flag & PEOPLE_POLICE)
					tp++;
				else
					tc++;
	}
	if (tk==0)
	{
		// 好人胜利
		RINFO.status=INROOM_STOP;
		send_msg (-1, "\33[31;1m所有杀手都被处决了，好人获得了胜利...\33[m");
		show_killerpolice();
		save_result (0);
		return 1;
	}
	else if (tp==0 || tc==0 || tk>=tp+tc)
	{
		// 坏人胜利
		RINFO.status=INROOM_STOP;
		if (tp==0)
			send_msg (-1, "\33[31;1m英勇的警察全部牺牲，坏人获得了胜利...\33[m");
		else
		if (tc==0)
			send_msg (-1, "\33[31;1m无辜的平民全部死亡，坏人获得了胜利...\33[m");
		else
			send_msg (-1, "\33[31;1m好人没有坏人多，坏人获得了胜利...\33[m");
		for (i = 0; i < MAX_PLAYER; ++i)
			if (PINFO(i).style != -1
				&& (PINFO(i).flag & PEOPLE_KILLER)
				&& (PINFO(i).flag & PEOPLE_ALIVE))
			{
				sprintf (buf, "原来\33[32;1m %d %s \33[m是杀手!", i + 1, PINFO(i).nick);
				send_msg (-1, buf);
		}
		show_killerpolice();
		save_result (1);
		return 2;
	}
	return 0;
}

void goto_night()
{
	int i;
	if (!check_win())
	{
		send_msg(-1,"\x1b[31;1m夜幕降临了...\x1b[m");
		for(i=0;i<MAX_PLAYER;i++)
			if (PINFO(i).style!=-1
				&& (PINFO(i).flag & PEOPLE_ALIVE))
			{
				if (PINFO(i).flag & PEOPLE_KILLER)
					if (RINFO.nokill)
						send_msg(i,"用你的尖刀(\x1b[31;1mCtrl+S\x1b[m)选择要吓唬的人,\x1b[31;1mCtrl+T\x1b[m确认");
					else
						send_msg(i,"用你的尖刀(\x1b[31;1mCtrl+S\x1b[m)选择要残害的人,\x1b[31;1mCtrl+T\x1b[m确认");
				else
					if (PINFO(i).flag & PEOPLE_POLICE)
						send_msg(i,"用你的警棒(\x1b[31;1mCtrl+S\x1b[m)查看你怀疑的人,\x1b[31;1mCtrl+T\x1b[m确认");
			}
 
		RINFO.victim=-1;
		RINFO.turn=-1;
		RINFO.round=-1;
		RINFO.informant=-1;
		for(i=0;i<MAX_PLAYER;i++)
		{
			PINFO(i).vote=-1;
			PINFO(i).flag &=~PEOPLE_VOTED;
			RINFO.voted[i]=-1;
		}
		RINFO.status=INROOM_NIGHT; 
	}
}

void goto_check()
{
	char buf[200];
	int k;
	k=0;
	if (!check_win())
	{
		if (RINFO.nokill)
		{
			RINFO.nokill=0;
			RINFO.turn=RINFO.victim;
		}
		else
		{
			RINFO.turn=RINFO.victim+1;
		}
		while (PINFO(RINFO.turn).style==-1 
			|| !(PINFO(RINFO.turn).flag & PEOPLE_ALIVE) )
		{
			RINFO.turn++;
			if (RINFO.turn>=MAX_PLAYER)
				RINFO.turn=0;
			if (RINFO.turn==RINFO.victim)
			{
				k=1;
				break;
			}
		}
		if (!k)
		{
			send_msg (-1, "\33[31;1m现在开始指证\33[m");
			sprintf (buf,"先请\33[32;1m %d %s \33[m发言", RINFO.turn + 1, PINFO(RINFO.turn).nick);
			send_msg (-1, buf);
			send_msg (RINFO.turn, "请\x1b[31;1m上下移动光标\x1b[m，在您怀疑的对象上");
			send_msg (RINFO.turn, "按\33[31;1mCtrl+S\33[m指证");
			send_msg (RINFO.turn, "指证完后，用\33[31;1mCtrl+T\33[m结束发言");
			if (PINFO(RINFO.turn).flag & PEOPLE_KILLER)
				send_msg (RINFO.turn, "\33[31;1m把光标移到自己位置上，用Ctrl+S可自杀\33[m");
			RINFO.round=0;
			RINFO.status=INROOM_CHECK;
		}
		else
		{
			send_msg(-1,"\33[31;1m没有指证的人，取消指证、辩护和投票\33[m");
			goto_night();
		}
	}
}
	
void goto_dark()
{
	char buf[200];
	if (RINFO.victim>=MAX_PLAYER || RINFO.victim<0)
	{
		sprintf(buf,"\33[31;1m没有判决,没有遗言\33[m");
		send_msg(-1,buf);
		goto_night();
	}
	else if (PINFO(RINFO.victim).flag & PEOPLE_KILLER)
	{
		sprintf(buf,"\33[31;1m%d %s 被法官处决了，没有遗言\33[m",RINFO.victim+1,PINFO(RINFO.victim).nick);
		PINFO(RINFO.victim).flag&=~PEOPLE_ALIVE;
		send_msg(-1,buf);
		goto_night();
	}
	else
	{
		int i,tk,tp,tc;
		tk=tp=tc=0;
		for(i=0;i<MAX_PLAYER;i++)
			if (i!=RINFO.victim && PINFO(i).style!=-1 && (PINFO(i).flag & PEOPLE_ALIVE))
			{
				if (PINFO(i).flag & PEOPLE_KILLER)
					tk++;
				else
				if (PINFO(i).flag & PEOPLE_POLICE)
					tp++;
				else
					tc++;
			}
		if (tk==0 || tp==0 || tc==0 || tp+tc<=tk)
		{
			sprintf(buf,"\33[31;1m%d %s 被法官处决了\33[m",RINFO.victim+1,PINFO(RINFO.victim).nick);
			PINFO(RINFO.victim).flag&=~PEOPLE_ALIVE;
			goto_night();
		}
		else
		{
			sprintf(buf,"\33[31;1m%d %s 被法官处决了，请留遗言\33[m",RINFO.victim+1,PINFO(RINFO.victim).nick);
			send_msg(-1,buf);
			RINFO.status=INROOM_DARK;
			send_msg(RINFO.victim,"按\33[31;1mCtrl+T\33[m结束遗言");
		}
	}
}

void goto_defence()
{
	char buf[200];
	int k;
	int i,j=0;
	
	RINFO.round++;

	if (RINFO.victim<0 || RINFO.victim>=MAX_PLAYER)
		RINFO.victim=0;
	
	k=0;
	for(i=0;i<MAX_PLAYER;i++)
		if (PINFO(i).style!=-1
			&& (PINFO(i).flag & PEOPLE_ALIVE)
			&& (PINFO(i).flag & PEOPLE_KILLER)
			&& (PINFO(i).flag & PEOPLE_SURRENDER))
		{
			PINFO(i).flag &=~PEOPLE_ALIVE;
			k++;
		}
	if (k>0)
	{
		send_msg(-1,"\33[31;1m杀手崩溃了，取消辩护和投票\33[m");
		goto_night();
		return;
	}

	k=0;	
	for(i=0;i<MAX_PLAYER;i++)
		if (PINFO(i).style!=-1
			&& (PINFO(i).flag & PEOPLE_ALIVE)
			&& (PINFO(i).flag & PEOPLE_VOTED))
			{
				k++;
				j=i;
			}
	if (k==0)
	{
		send_msg (-1, "\33[31;1m没有被指证的人，取消辩护和投票\33[m");
		goto_night();
		return;
	}
	else if (k==1)
	{
		send_msg(-1,"\33[31;1m只有一个被指证的人，取消辩护和投票\33[m");
		RINFO.victim=j;
		goto_dark();
		return;
	}
	
	RINFO.turn=RINFO.victim;
	k=0;
	while (PINFO(RINFO.turn).style==-1 
		|| !(PINFO(RINFO.turn).flag & PEOPLE_ALIVE)
		|| !(PINFO(RINFO.turn).flag & PEOPLE_VOTED) )
	{
		if (RINFO.round%2)
			RINFO.turn--;
		else
			RINFO.turn++;
		if (RINFO.turn>=MAX_PLAYER)
			RINFO.turn=0;
		if (RINFO.turn<0)
			RINFO.turn=MAX_PLAYER-1;
		if (RINFO.turn==RINFO.victim)
		{
			k=1;
			break;
		}
	}
	if (!k)
	{
		send_msg (-1, "\33[31;1m现在开始辩护\33[m");
		sprintf (buf,"先请\33[32;1m %d %s \33[m辩护", RINFO.turn + 1, PINFO(RINFO.turn).nick);
		send_msg (-1, buf);

		send_msg (RINFO.turn, "辩护完后，用\33[31;1mCtrl+T\33[m结束");
		if (PINFO(RINFO.turn).flag & PEOPLE_KILLER)
			send_msg (RINFO.turn, "\33[31;1m把光标移到自己位置上，用Ctrl+S可自杀\33[m");
		RINFO.status=INROOM_DEFENCE;
	}
	else
	{
		send_msg(-1,"\33[31;1m没有辩护的人，取消辩护和投票\33[m");
		goto_night();
	}
}

void goto_day()
{
	char buf[200];
	if (RINFO.victim>=MAX_PLAYER || RINFO.victim<0)
	{
		sprintf(buf,"\33[31;1m没有死者,没有遗言\33[m");
		send_msg(-1,buf);
		goto_check();

	}
	else
	{
		int i,tk,tp,tc;
		tk=tp=tc=0;
		for(i=0;i<MAX_PLAYER;i++)
			if (i!=RINFO.victim && PINFO(i).style!=-1 && (PINFO(i).flag & PEOPLE_ALIVE))
			{
				if (PINFO(i).flag & PEOPLE_KILLER)
					tk++;
				else
				if (PINFO(i).flag & PEOPLE_POLICE)
					tp++;
				else
					tc++;
			}
		if (tk==0 || tp==0 || tc==0)
		{
			PINFO(RINFO.victim).flag&=~PEOPLE_ALIVE;
			goto_check();
		}
		else
		{
			sprintf (buf, "现在，被害者\33[32;1m %d %s \33[m在天堂有什么话要说？", RINFO.victim + 1, PINFO(RINFO.victim).nick);
			send_msg (-1, buf);
			RINFO.status=INROOM_DAY;
			send_msg (RINFO.victim, "请按\33[31;1mCtrl+T\33[m结束遗言");
		}
	}
}

void goto_vote()
{
	int i,k;
	k=0;
	for(i=0;i<MAX_PLAYER;i++)
		if (PINFO(i).style!=-1
			&& (PINFO(i).flag & PEOPLE_ALIVE)
			&&(PINFO(i).flag & PEOPLE_KILLER)
			&& (PINFO(i).flag & PEOPLE_SURRENDER))
		{
			PINFO(i).flag &=~PEOPLE_ALIVE;
			k++;
		}
	if (k>0)
	{
		send_msg(-1,"\33[31;1m杀手崩溃了，投票取消\33[m");
		goto_night();
	}
	else
	{
		for(i=0;i<MAX_PLAYER;i++)
		{
			RINFO.voted[i]=-1;
			PINFO(i).vote=-1;
		}
		RINFO.status=INROOM_VOTE;
				  
		send_msg (-1, "\33[31;1m现在请大家投票\33[m");
		for(i=0;i<MAX_PLAYER;i++)
			if (PINFO(i).style!=-1
				&& (PINFO(i).flag & PEOPLE_ALIVE))
				send_msg (i, "按\33[31;1mCtrl+S\33[m投票，\33[31;1mCtrl+T\33[m确认");
	}
}

void start_game()
{
	int i, j, totalk = 0, total = 0, totalc = 0, me;
	char buf[80];
	time_t t;
	struct tm * tt;
	me = mypos;
	for (i = 0; i < MAX_PLAYER; i++)
		if (PINFO(i).style != -1)
		{
			PINFO(i).flag=PEOPLE_ALIVE;
			PINFO(i).vote = -1;
		}
		
	if (RINFO.killernum<1)
		RINFO.killernum=1;
	if (RINFO.policenum<1)
	  RINFO.policenum=1;
		
	totalk = RINFO.killernum;
	totalc = RINFO.policenum;
	for (i = 0; i < MAX_PLAYER; i++)
		if (PINFO(i).style != -1)
			total++;
	if (total < 3)
	{
		send_msg(me, "\x1b[31;1m至少3人参加才能开始游戏\x1b[m");
		end_change_inroom();
		kill_msg(mypos);
		return;
	}

	if (totalk > total-totalk-totalc)
	{
		send_msg(me, "\x1b[31;1m平民数少于坏人人数,无法开始游戏\x1b[m");
		end_change_inroom();
		kill_msg(mypos);
		return;
	}
	if (totalc == 0)
		sprintf(buf, "\x1b[31;1m游戏开始啦! 人群中出现了%d个坏人\x1b[m", totalk);
	else
		sprintf(buf, "\x1b[31;1m游戏开始啦! 人群中出现了%d个坏人, %d个警察\x1b[m", totalk, totalc);
	send_msg(-1, buf);
	
	time(&t);
	tt=localtime(&t);
	sprintf(RINFO.gamename,"%s: %d-%d-%d, %04d/%02d/%02d %02d:%02d:%02d",rooms[myroom].name,totalk,totalc,total-totalk-totalc,
		tt->tm_year+1900,tt->tm_mon+1,tt->tm_mday,tt->tm_hour,tt->tm_min,tt->tm_sec);

#ifdef AUTOPOST
	unlink(ap_tmpname());
	fd = open(ap_tmpname(), O_WRONLY | O_CREAT | O_APPEND, 0644);
	if (fd>=0)
	{
		ldata.l_type = F_WRLCK;
		ldata.l_whence = 0;
		ldata.l_len = 0;
		ldata.l_start = 0;
		if (fcntl(fd, F_SETLKW, &ldata) != -1)
		{
			lseek(fd, 0, SEEK_END);
			for(i=0;i<MAX_PLAYER;i++)
			{
				if (PINFO(i).style!=-1)
				{
					sprintf(buf, "\x1b[34;1m%2d-%s(%s)\x1b[m", (i+1)%100, PINFO(i).nick, PINFO(i).id);
					write(fd,buf,strlen(buf));
					write(fd,"\n",1);
				}
			}
			ldata.l_type = F_UNLCK;
			fcntl(fd, F_SETLKW, &ldata);
		}
		close(fd);			
	}
#endif // AUTOPOST
	
	for(i=0;i<MAX_PLAYER;i++)
	{
		RINFO.killers[i]=-1;
		RINFO.polices[i]=-1;
		RINFO.seq_detect[i]=-1;
	}
	for (i = 0; i < totalk; i++)
	{
		do
		{
			j = rand() % MAX_PLAYER;
		} while (PINFO(j).style == -1 || PINFO(j).flag & PEOPLE_KILLER);
		PINFO(j).flag |= PEOPLE_KILLER;
		RINFO.killers[i]=j;
		send_msg(j, "你做了一个无耻的坏人");
	}
	if (totalc>1 && total-totalk-totalc>1)
		RINFO.nokill=0;
	else
		RINFO.nokill=1;
	for (i = 0; i < totalc; i++)
	{
		do
		{
			j = rand() % MAX_PLAYER;
		} while (PINFO(j).style == -1 
			|| PINFO(j).flag & PEOPLE_KILLER
			|| PINFO(j).flag & PEOPLE_POLICE);
		PINFO(j).flag |= PEOPLE_POLICE;
		RINFO.polices[i]=j;
		send_msg(j, "你做了一位光荣的人民警察");
	}
	goto_night();

	kill_msg(-1);
}

#define menust 10

int do_com_menu()
{
	char menus[menust][15] = { 
		"0-返回", 
		"1-退出", 
		"2-改名", 
		"3-列表", 
		"4-加入",
		"5-话题", 
		"6-设置", 
		"7-踢玩家", 
		"8-换房主", 
		"9-开始"
	};
	int menupos[menust], i, j, k, sel = 0, ch, max = 0, me, offset = 0;
	char buf[80],disp[256];
 
  me=mypos;
  if (me<MAX_PLAYER)
    strcpy(menus[4],"4-旁观");
  
	if (RINFO.status != INROOM_STOP)
		strcpy(menus[menust-1], "9-结束");
	menupos[0] = 0;
	for (i = 1; i < menust; i++)
		menupos[i] = menupos[i - 1] + strlen(menus[i - 1]) + 1;
	do
	{
		move(t_lines - 1, 0);
		clrtoeol();
		offset = 0;
		strcpy(disp,"\x1b[m");
		while (menupos[sel] - menupos[offset] + strlen(menus[sel]) >= scr_cols)
			offset++;
		j = mypos;
		for (i = 0; i < menust; i++)
			if (rooms[myroom].op==j || i <= 4)
				if (menupos[i] - menupos[offset] >= 0)
				{
					if (i == sel)
						k_setfcolor(RED, 1);
					else
						k_resetcolor();
					if (i >= max - 1)
						max = i + 1;
					if (i>0)
						strcat(disp," ");
					strcat(disp, menus[i]);
				}
		strcat(disp,"\x1b[m");
		move(t_lines - 1, 0);
		prints("%s",disp);
		
		ch = igetkey();
		if (kicked)
			return 0;
		switch (ch)
		{
		case KEY_LEFT:
		case KEY_UP:
			sel--;
			if (sel < 0)
				sel = max - 1;
			break;
		case KEY_RIGHT:
		case KEY_DOWN:
			sel++;
			if (sel >= max)
				sel = 0;
			break;
		case '\n':
		case '\r':
			me = mypos;
			switch (menus[sel][0]) 
			{
			case '0':
				return 0;
			case '1':
				if (me<MAX_PLAYER
					&& (PINFO(me).flag & PEOPLE_ALIVE)
					&& RINFO.status != INROOM_STOP)
					{
						send_msg(me, "你还在游戏,不能退出");
						kill_msg(me);
						return 0;
					}
				move(t_lines - 1, 0);
				k_resetcolor();
				clrtoeol();
				k_getdata(t_lines - 1, 0, "确认退出？ [y/N] ", buf, 3, 1, 0, 1);
				if (me<MAX_PLAYER
					&& (PINFO(me).flag & PEOPLE_ALIVE)
					&& RINFO.status != INROOM_STOP)
				{
					return 0;
				}
				if (kicked)
					return 0;
				if (toupper(buf[0]) != 'Y')
					return 0;
				return 1;
			case '2':
				if (RINFO.status != INROOM_STOP)
				{
					send_msg (me, "游戏中，不能改ID");
					kill_msg(me);
					return 0;
				}
				move(t_lines - 1, 0);
				k_resetcolor();
				clrtoeol();
				k_getdata(t_lines - 1, 0, "请输入名字:", buf, 13, 1, 0, 1);
				if (kicked || RINFO.status != INROOM_STOP)
					return 0;
				if (buf[0])
				{
					k = 1;
					for (j = 0; j < strlen(buf); j++)
						k = k && (isprint2(buf[j]));
					k = k && (buf[0] != ' ');
					k = k && (buf[strlen(buf) - 1] != ' ');
					if (!k)
					{
						move(t_lines - 1, 0);
						k_resetcolor();
						clrtoeol();
						prints(" 名字不符合规范");
						refresh();
						sleep(1);
						return 0;
					}
					j = 0;
					for (i = 0; i < MAX_PEOPLE; i++)
						if (PINFO(i).style != -1)
							if (i != me)
								if (!strcmp(buf, PINFO(i).id) || !strcmp(buf, PINFO(i).nick))
									j = 1;
					if (j)
					{
						move(t_lines - 1, 0);
						k_resetcolor();
						clrtoeol();
						prints(" 已有人用这个名字了");
						refresh();
						sleep(1);
						return 0;
					}
					start_change_inroom();
					strcpy(PINFO(me).nick, buf);
					end_change_inroom();
					kill_msg(-1);
				}
				return 0;
			case '3':
				for (i = 0; i < MAX_PEOPLE; i++)
					if (PINFO(i).style != -1)
					{
						sprintf(buf, "%2d %-12s %-12s", (i+1)%100,PINFO(i).id, PINFO(i).nick);
						send_msg(me, buf);
					}
				kill_msg(me);
				return 0;
			case '4':
				if (me<MAX_PLAYER
					&& (PINFO(me).flag & PEOPLE_ALIVE)
					&& RINFO.status != INROOM_STOP)
				{
					send_msg (me, "游戏中，不能离开");
					kill_msg(me);
					return 0;
				}
				else
				if ((me>=MAX_PLAYER && 
					(rooms[myroom].numplayer>=rooms[myroom].maxplayer
					||rooms[myroom].numplayer>=MAX_PLAYER))
					|| (me<MAX_PLAYER
					&& rooms[myroom].numspectator>=MAX_PEOPLE-MAX_PLAYER))
				{
					send_msg(me, "人员已满");
					kill_msg(me);
					return 0;
				}
				else
				if (me>=MAX_PLAYER)
				{
					// 变成游戏者
					int i;
					for(i=0;i<MAX_PLAYER;i++)
					{
						if (PINFO(i).style==-1)
							break;
					}
					if (i<MAX_PLAYER)
					{
						start_change_inroom();
						PINFO(me).style=-1;
						mypos = i;
						PINFO(i).style = 0;
						PINFO(i).flag = 0;
						strcpy(PINFO(i).id, CURRENTUSER->userid);
						strcpy(PINFO(i).nick, CURRENTUSER->userid);
						PINFO(i).pid = uinfo.pid;
					
						rooms[myroom].numplayer++;
						rooms[myroom].numspectator--;
					
						if (rooms[myroom].op==me || rooms[myroom].op==-1 || PINFO(rooms[myroom].op).style==-1)
						{
							rooms[myroom].op=mypos;
						}
					
						end_change_inroom();
					
						kill_msg(-1);
					
						room_refresh(0);
						
					}
				}
				else
				{
					// 变成旁观者
					int i;
					for(i=MAX_PLAYER;i<MAX_PEOPLE;i++)
					{
						if (PINFO(i).style==-1)
							break;
					}
					if (i<MAX_PEOPLE)
					{
						start_change_inroom();
						PINFO(me).style=-1;
						mypos = i;
						PINFO(i).style = 0;
						PINFO(i).flag = 0;
						strcpy(PINFO(i).id, CURRENTUSER->userid);
						strcpy(PINFO(i).nick, CURRENTUSER->userid);
						PINFO(i).pid = uinfo.pid;
					
						rooms[myroom].numspectator++;
						rooms[myroom].numplayer--;
					
						if (rooms[myroom].op==me || rooms[myroom].op==-1 || PINFO(rooms[myroom].op).style==-1)
						{
							rooms[myroom].op=mypos;
						}
					
						end_change_inroom();
					
						kill_msg(-1);
					
						room_refresh(0);
						
					}
				}
				return 0;
			case '5':
				move(t_lines - 1, 0);
				k_resetcolor();
				clrtoeol();
				k_getdata(t_lines - 1, 0, "请输入话题:", buf, 31, 1, 0, 1);
				if (kicked)
					return 0;
				if (buf[0]) 
				{
					start_change_inroom();
					strcpy(rooms[myroom].title, buf);
					end_change_inroom();
					kill_msg(-1);
				}
				return 0;
			case '6':
				move(t_lines - 1, 0);
				k_resetcolor();
				clrtoeol();
				k_getdata(t_lines - 1, 0, "请输入房间最大人数:", buf, 30, 1, 0, 1);
				if (kicked)
					return 0;
				if (buf[0])
				{
					i = atoi(buf);
					if (i > 0 && i <= MAX_PLAYER)
					{
						rooms[myroom].maxplayer = i;
						sprintf(buf, "屋主设置房间最大人数为%d", i);
						send_msg(-1, buf);
					}
				}
				move(t_lines - 1, 0);
				clrtoeol();
				k_getdata(t_lines - 1, 0, "设置为隐藏房间? [Y/N]", buf, 30, 1, 0, 1);
				if (kicked)
					return 0;
				buf[0] = toupper(buf[0]);
				if (buf[0] == 'Y' || buf[0] == 'N') 
				{
					if (buf[0] == 'Y')
						rooms[myroom].flag |= ROOM_SECRET;
					else
						rooms[myroom].flag &= ~ROOM_SECRET;
					sprintf(buf, "屋主设置房间为%s", (buf[0] == 'Y') ? "隐藏" : "不隐藏");
					send_msg(-1, buf);
				}
				move(t_lines - 1, 0);
				clrtoeol();
				k_getdata(t_lines - 1, 0, "设置为锁定房间? [Y/N]", buf, 30, 1, 0, 1);
				if (kicked)
					return 0;
				buf[0] = toupper(buf[0]);
				if (buf[0] == 'Y' || buf[0] == 'N')
				{
					if (buf[0] == 'Y')
						rooms[myroom].flag |= ROOM_LOCKED;
					else
						rooms[myroom].flag &= ~ROOM_LOCKED;
					sprintf(buf, "屋主设置房间为%s", (buf[0] == 'Y') ? "锁定" : "不锁定");
					send_msg(-1, buf);
				}
				move(t_lines - 1, 0);
				clrtoeol();
				k_getdata(t_lines - 1, 0, "设置为拒绝旁观者的房间? [Y/N]", buf, 30, 1, 0, 1);
				if (kicked)
					return 0;
				buf[0] = toupper(buf[0]);
				if (buf[0] == 'Y' || buf[0] == 'N')
				{
					if (buf[0] == 'Y')
						rooms[myroom].flag |= ROOM_DENYSPEC;
					else
						rooms[myroom].flag &= ~ROOM_DENYSPEC;
					sprintf(buf, "屋主设置房间为%s", (buf[0] == 'Y') ? "拒绝旁观" : "不拒绝旁观");
					send_msg(-1, buf);
				}
				/*
				move(t_lines - 1, 0);
				clrtoeol();
				k_getdata(t_lines - 1, 0, "设置为旁观者无法看见杀手警察的房间? [Y/N]", buf, 30, 1, 0, 1);
				if (kicked)
					return 0;
				buf[0] = toupper(buf[0]);
				if (buf[0] == 'Y' || buf[0] == 'N')
				{
					if (buf[0] == 'Y')
						rooms[myroom].flag |= ROOM_SPECBLIND;
					else
						rooms[myroom].flag &= ~ROOM_SPECBLIND;
					sprintf(buf, "屋主设置房间为%s", (buf[0] == 'Y') ? "旁观无法看见杀手警察" : "旁观可以看见杀手警察");
					send_msg(-1, buf);
				}
				*/
				move(t_lines - 1, 0);
				clrtoeol();
				k_getdata(t_lines - 1, 0, "设置坏人的数目:", buf, 30, 1, 0, 1);
				if (kicked)
					return 0;
				if (buf[0])
				{
					i = atoi(buf);
					if (i >= 0 && i <= MAX_KILLER)
					{
						RINFO.killernum = i;
						sprintf(buf, "屋主设置房间坏人数为%d", i);
						send_msg(-1, buf);
					}
				}
				move(t_lines - 1, 0);
				clrtoeol();
				k_getdata(t_lines - 1, 0, "设置警察的数目:", buf, 30, 1, 0, 1);
				if (kicked)
					return 0;
				if (buf[0])
				{
					i = atoi(buf);
					if (i >= 0 && i <= MAX_POLICE)
					{
						RINFO.policenum = i;
						sprintf(buf, "屋主设置房间警察数为%d", i);
						send_msg(-1, buf);
					}
				}
				kill_msg(-1);
				return 0;
			case '7':
				move(t_lines - 1, 0);
				k_resetcolor();
				clrtoeol();
				k_getdata(t_lines - 1, 0, "请输入要踢的序号(数字):", buf, 4, 1, 0, 1);
				if (kicked)
					return 0;
				if (buf[0])
				{
					i = atoi(buf) - 1;
					if (i >= 0 && i < MAX_PEOPLE && PINFO(i).style != -1 && PINFO(i).pid != uinfo.pid)
					{
						sprintf(buf,"\x1b[34;1m%d %s被踢了\x1b[m",i+1,PINFO(i).nick);
						send_msg(-1,buf);
						send_msg(i, "你被踢了");
						kill_msg(-1);
						start_change_inroom();
						player_drop(i);
						end_change_inroom();
				 
						return 2;
					}
				}
				return 0;
/*			case '7':

				move(t_lines - 1, 0);
				k_resetcolor();
				clrtoeol();
				k_getdata(t_lines - 1, 0, "请输入要禁止/恢复发言权的序号(数字):", buf, 4, 1, 0, 1);
				if (kicked)
					return 0;
				if (buf[0])
				{
					i = atoi(buf) - 1;
					if (i >= 0 && i < MAX_PEOPLE && PINFO(i).style != -1 && PINFO(i).pid != uinfo.pid)
					{
						if (PINFO(i).flag & PEOPLE_DENYSPEAK)
							PINFO(i).flag &= ~PEOPLE_DENYSPEAK;
						else
							PINFO(i).flag |= PEOPLE_DENYSPEAK;
						sprintf(buf, "%d %s被%s了发言权", i + 1, PINFO(i).nick, (PINFO(i).flag & PEOPLE_DENYSPEAK) ? "禁止" : "恢复");
						send_msg(-1, buf);
						kill_msg(-1);
						return 0;
					}
				}
				return 0;
*/				
			case '8':
				move(t_lines - 1, 0);
				k_resetcolor();
				clrtoeol();
				k_getdata(t_lines - 1, 0, "请输入转交房主的序号(数字):", buf, 4, 1, 0, 1);
				if (kicked)
					return 0;
				if (buf[0])
				{
					i = atoi(buf) - 1;
					if (i >= 0 && i < MAX_PLAYER && PINFO(i).style != -1 && PINFO(i).pid != uinfo.pid)
					{
						rooms[myroom].op=i;
						sprintf(buf, "\x1b[34;1m%d %s成为新房主\x1b[m", i+1,PINFO(i).nick);
						send_msg(-1, buf);
						kill_msg(-1);
						return 0;
					}
				}
				return 0;
			case '9':
				start_change_inroom();
				if (RINFO.status == INROOM_STOP)
					start_game();
				else
				{
					RINFO.status = INROOM_STOP;
					send_msg(-1, "游戏被屋主强制结束");
					show_killerpolice();
					kill_msg(-1);
				}
				end_change_inroom();
				return 0;
			}
			break;
		default:
			for (i = 0; i < max; i++)
				if (ch == menus[i][0])
					sel = i;
			break;
		}
	} while (1);
}

void vote_result()
{
	int i;
	int k;
	char buf[200];
	start_change_inroom();
	if (RINFO.status==INROOM_VOTE)
	{
		for (i=0;i<MAX_PLAYER;i++)
		if (PINFO(i).style!=-1
			&& (PINFO(i).flag & PEOPLE_ALIVE)
			&& PINFO(i).vote==-1)
			break;
				  
		if (i>=MAX_PLAYER)
		{
			int m1,m2,tt;
			tt=0;
			for(i=0;i<MAX_PLAYER;i++)
				if (PINFO(i).style!=-1
					&& (PINFO(i).flag & PEOPLE_ALIVE))
				{
					sprintf(buf,"\33[32;1m%d %s 投了 %d %s 一票\33[m",
						i+1,PINFO(i).nick,
						PINFO(i).vote+1,PINFO(PINFO(i).vote).nick);
					send_msg(-1,buf);
					PINFO(i).vnum=0;
					tt++;
				}
			for(i=0;i<MAX_PLAYER;i++)
				if (PINFO(i).style!=-1
					&& (PINFO(i).flag & PEOPLE_ALIVE))
				{
					PINFO(PINFO(i).vote).vnum++;
				}
			m1=-1;
			m2=-1;
					 
			for(i=0;i<MAX_PLAYER;i++)
				if (PINFO(i).style!=-1
					&& (PINFO(i).flag & PEOPLE_ALIVE)
					&& (PINFO(i).flag & PEOPLE_VOTED))
				{
					if (PINFO(i).vnum>m1)
					{
						m2=m1;
						m1=PINFO(i).vnum;
					}
					else if (PINFO(i).vnum>m2)
						m2=PINFO(i).vnum;

				}
			 
			if (m1*2>tt)
			{
				// 直接投死
				for(i=0;i<MAX_PLAYER;i++)
					if (PINFO(i).style!=-1
						&& (PINFO(i).flag & PEOPLE_ALIVE)
						&& (PINFO(i).flag & PEOPLE_VOTED))
					{
						if (PINFO(i).vnum==m1)
							break;
					}
				RINFO.victim=i;
				goto_dark();
			}
			else
			{
				// 未超过1/2

				tt=0;
				for(i=0;i<MAX_PLAYER;i++)
					if (PINFO(i).style!=-1
						&& (PINFO(i).flag & PEOPLE_ALIVE)
						&& (PINFO(i).flag & PEOPLE_VOTED))
					{
						if (PINFO(i).vnum!=m1
							&& PINFO(i).vnum!=m2)
							PINFO(i).flag &=~PEOPLE_VOTED;
						else
							tt++;
					}
				if (RINFO.round>3)
				{
					//超过3轮,随机处决
					send_msg(-1, "\33[31;1m投票三次仍未有结果，法官随机处决\33[m");
					k=rand()%tt;
					for(i=0;i<MAX_PLAYER;i++)
						if (PINFO(i).style!=-1
							&& (PINFO(i).flag & PEOPLE_ALIVE)
							&& (PINFO(i).flag & PEOPLE_VOTED))
						{
							if (k==0)
								break;
							k--;			
						}
					RINFO.victim=i;
					goto_dark();
				}
				else
				{
					send_msg (-1, "\33[31;1m最高票数未超过半数,请获得最多票数的人辩护\33[m");
					while (m1>0)
					{
						tt=0;
						for(i=0;i<MAX_PLAYER;i++)
							if (PINFO(i).style!=-1
								&& (PINFO(i).flag & PEOPLE_ALIVE)
								&& (PINFO(i).flag & PEOPLE_VOTED))
							{
								if (PINFO(i).vnum==m1)
									tt++;
							}
						if (tt>0)
							break;
						m1--;
					}
					if (m1>0 && tt>0)
					{
						k=rand()%tt;
						for(i=0;i<MAX_PLAYER;i++)
							if (PINFO(i).style!=-1
								&& (PINFO(i).flag & PEOPLE_ALIVE)
								&& (PINFO(i).flag & PEOPLE_VOTED)
								&& (PINFO(i).vnum==m1))
							{
								if (k==0)
									break;
								k--;			
							}
						RINFO.victim=i;
						goto_defence();
					}
					else
					{
						RINFO.victim=-1;
						goto_defence();
					}
				}
			}
			kill_msg(-1);
		}
	}
	end_change_inroom();
}

void player_drop(int d)
{
	char buf[200];
	if (PINFO(d).style!=-1)
	{
		PINFO(d).style=-1;
		if (d<MAX_PLAYER)
			rooms[myroom].numplayer--;
		else
			rooms[myroom].numspectator--;
		kill_msg(d);
	}
	switch(RINFO.status)
	{
	case INROOM_STOP:
		break;
	case INROOM_NIGHT:
		check_win();
		break;
	case INROOM_DAY:
		if (d==RINFO.victim)
		{
			sprintf (buf, "\x1b[1;32m%d %s\x1b[m: \x1b[1;31mOver.\x1b[m", d + 1, PINFO(d).nick);
			send_msg (-1, buf);
			goto_check();
		}
		break;
	case INROOM_CHECK:
		if (d==RINFO.turn)
		{
			sprintf (buf, "\x1b[1;32m%d %s\x1b[m: \x1b[1;31mOver.\x1b[m", d + 1, PINFO(d).nick);
			send_msg (-1, buf);
			while (1)
			{
				RINFO.turn++;
				if (RINFO.turn>=MAX_PLAYER)
					RINFO.turn=0;
				if (RINFO.turn==RINFO.victim)
					break;
				if (PINFO(RINFO.turn).style!=-1 
					&& (PINFO(RINFO.turn).flag & PEOPLE_ALIVE) )
					break;
			}
			if (RINFO.turn!=RINFO.victim)
			{
				sprintf (buf,"请\33[32;1m %d %s \33[m发言", RINFO.turn + 1, PINFO(RINFO.turn).nick);
				send_msg (-1, buf);
				send_msg (RINFO.turn, "请\x1b[31;1m上下移动光标\x1b[m，在您怀疑的对象上");
				send_msg (RINFO.turn, "按\33[31;1mCtrl+S\33[m指证");
				send_msg (RINFO.turn, "指证完后，用\33[31;1mCtrl+T\33[m结束发言");
				if (PINFO(RINFO.turn).flag & PEOPLE_KILLER)
					send_msg (RINFO.turn, "\33[31;1m把光标移到自己位置上，用Ctrl+S可自杀\33[m");
			}
			else
			{
				RINFO.round=1;
				goto_defence();
						 
			}
		}
		break;
	case INROOM_DEFENCE:
		if (d==RINFO.turn)
		{
			sprintf (buf, "\x1b[1;32m%d %s\x1b[m: \x1b[1;31mOver.\x1b[m", d + 1, PINFO(d).nick);
			send_msg (-1, buf);
			while (1)
			{
				if (RINFO.round%2)
					RINFO.turn--;
				else
					RINFO.turn++;
				if (RINFO.turn>=MAX_PLAYER)
					RINFO.turn=0;
				if (RINFO.turn<0)
					RINFO.turn=MAX_PLAYER-1;
				if (RINFO.turn==RINFO.victim)
					break;
				if (PINFO(RINFO.turn).style!=-1 
					&& (PINFO(RINFO.turn).flag & PEOPLE_ALIVE)
					&& (PINFO(RINFO.turn).flag & PEOPLE_VOTED))
					break;
			}
			if (RINFO.turn!=RINFO.victim)
			{
				sprintf (buf,"请\33[32;1m %d %s \33[m辩护", RINFO.turn + 1, PINFO(RINFO.turn).nick);
				send_msg (-1, buf);
				send_msg (RINFO.turn, "辩护完后，用\33[31;1mCtrl+T\33[m结束");
				if (PINFO(RINFO.turn).flag & PEOPLE_KILLER)
					send_msg (RINFO.turn, "\33[31;1m把光标移到自己位置上，用Ctrl+S可自杀\33[m");
			}
			else
			{
				goto_vote();
	
			}
		}
		break;
	case INROOM_VOTE:
		vote_result();
		break;
	case INROOM_DARK:
		if (d==RINFO.victim)
		{
			sprintf (buf, "\x1b[1;32m%d %s\x1b[m: \x1b[1;31mOver.\x1b[m", d + 1, PINFO(d).nick);
			send_msg (-1, buf);
			goto_night();
		}
		break;
	}
}

void join_room(int w, int spec)
{
	char buf[200], buf2[220], buf3[200], msg[200], roomname[80];
	int i, j, k, me;
	time_t ct;

	clear();

	selected = 0;
	ipage = 0;
	jpage = 0;
	
	myroom = w;
	start_change_inroom();
	if (rooms[myroom].style != 1)
	{
		end_change_inroom();
		return;
	}
	strcpy(roomname, rooms[myroom].name);
	signal(SIGUSR1, room_refresh);
	if (spec)
	  i=MAX_PLAYER;
	else
	  i = 0;
	while (PINFO(i).style != -1)
		i++;
	if ((i>=MAX_PLAYER && !spec) || (i>=MAX_PEOPLE && spec))
	{
		end_change_inroom();
		return;
	}
	mypos = i;
	PINFO(i).style = 0;
	PINFO(i).flag = 0;
	strcpy(PINFO(i).id, CURRENTUSER->userid);
	strcpy(PINFO(i).nick, CURRENTUSER->userid);
	PINFO(i).pid = uinfo.pid;

	if (!spec)
		rooms[myroom].numplayer++;
	else
		rooms[myroom].numspectator++;

	if (!spec)
	{
		if (rooms[myroom].op==-1 || PINFO(rooms[myroom].op).style==-1)
		{
			rooms[myroom].op=mypos;
		}
	}

	end_change_inroom();

	kill_msg(-1);

	room_refresh(0);
	while (1)
	{
		do
		{
			int ch;
			sprintf(buf2,"%2d输入:",(mypos+1)%100);
			ch = -k_getdata(t_lines - 1, 0, buf2, buf, 70, 1, NULL, 1);
			if (rooms[myroom].style != 1)
				kicked = 1;
			if (kicked)
				goto quitgame;
			if (ch == KEY_UP) 
			{
				selected--;
				if (selected < 0)
					selected = rooms[myroom].numplayer - 1;
				if (ipage > selected)
					ipage = selected;
				if (selected > ipage + t_lines - 5)
					ipage = selected - (t_lines - 5);
				kill_msg(mypos);
			}
			else if (ch == KEY_DOWN) 
			{
				selected++;
				if (selected >= rooms[myroom].numplayer)
					selected = 0;
				if (ipage > selected)
					ipage = selected;
				if (selected > ipage + t_lines - 5)
					ipage = selected - (t_lines - 5);
				kill_msg(mypos);
			} 
			else if (ch == KEY_PGUP)
			{
				jpage += t_lines / 2;
				kill_msg(mypos);
			} 
			else if (ch == KEY_PGDN)
			{
				jpage -= t_lines / 2;
				if (jpage <= 0)
					jpage = 0;
				kill_msg(mypos);
			} 
			else if (mypos<MAX_PLAYER
				&& (PINFO(mypos).flag & PEOPLE_ALIVE)
			   && ch == Ctrl('T'))
			{
				switch(RINFO.status)
				{
				case INROOM_STOP:
					break;
				case INROOM_NIGHT:
					if ((PINFO(mypos).flag & PEOPLE_KILLER) && RINFO.turn!=-1 && RINFO.victim==-1)
					{
						RINFO.victim=RINFO.turn;
						if (RINFO.nokill)
							sprintf (buf, "决定了，就吓唬\33[32;1m%d %s \33[m!", RINFO.victim + 1, PINFO(RINFO.victim).nick);
						else
							sprintf (buf, "决定了，就杀\33[32;1m%d %s \33[m!", RINFO.victim + 1, PINFO(RINFO.victim).nick);
				
						for(i=0;i<MAX_PLAYER;i++)
							if (PINFO(i).style!=-1
								&& (PINFO(i).flag & PEOPLE_ALIVE)
								&& (PINFO(i).flag & PEOPLE_KILLER))
							{
								send_msg(i,buf);
								kill_msg(i); 
							}
					}
					else if ((PINFO(mypos).flag & PEOPLE_POLICE) 
						&& RINFO.round!=-1 
						&& RINFO.informant==-1)
					{
						RINFO.informant=RINFO.round;
						for(i=0;i<MAX_PLAYER;i++)
							if (RINFO.seq_detect[i]==-1)
							{
								RINFO.seq_detect[i]=RINFO.informant;
								break;
							}
						if (PINFO(RINFO.informant).flag & PEOPLE_KILLER)
							sprintf (buf, "没错，\33[32;1m%d %s \33[m就是一位出色的杀手!", RINFO.informant + 1, PINFO(RINFO.informant).nick);
						else
							sprintf (buf, "笨笨，你猜错了，\33[32;1m%d %s \33[m是一个好人!", RINFO.informant + 1, PINFO(RINFO.informant).nick);
						for(i=0;i<MAX_PLAYER;i++)
							if (PINFO(i).style!=-1
								&& (PINFO(i).flag & PEOPLE_ALIVE)
							  && (PINFO(i).flag & PEOPLE_POLICE))
						  {
								send_msg(i,buf);
								kill_msg(i);
							}
					}
					start_change_inroom();
					if (RINFO.status==INROOM_NIGHT && RINFO.informant!=-1 && RINFO.victim!=-1)
					{
						// turn to check
						if (RINFO.nokill)
							sprintf (buf, "\33[1;33m注意，\33[32;1m %d %s \33[33m被盯上了...\33[m", RINFO.victim + 1, PINFO(RINFO.victim).nick);
						else
							sprintf (buf, "\33[1;33m阴森森的月光下，横躺着\33[32;1m %d %s \33[33m的尸体...\33[m", RINFO.victim + 1, PINFO(RINFO.victim).nick);
						send_msg(-1,buf);
					
						for(i=0;i<MAX_PLAYER;i++)
							RINFO.voted[i]=-1;
					
						if (RINFO.nokill)
						{
							// bluster
							goto_check();
						}
						else
						{
							goto_day();
						}
			 
					}
					end_change_inroom();
					kill_msg(-1);
					break;
				case INROOM_DAY:
					if (RINFO.victim==mypos)
					{
						  sprintf (buf, "\x1b[35;1m%d %s\x1b[m: \33[31;1mOver.\33[m", mypos + 1, PINFO(mypos).nick);
						send_msg (-1, buf);
						sprintf (buf, "那么，请\33[35;1m %d %s \33[m朋友以旁观者的身份，继续游戏...", mypos + 1, PINFO(mypos).nick);
						send_msg (-1, buf);
					
						if (PINFO(mypos).flag & PEOPLE_KILLER)
							PINFO(mypos).flag |=PEOPLE_HIDEKILLER;
						PINFO(mypos).flag&=~PEOPLE_ALIVE;
											
						goto_check();	
						kill_msg(-1);
					}
					break;
				case INROOM_CHECK:
					if (RINFO.turn==mypos && RINFO.voted[mypos]!=-1)
					{
						if ((PINFO(mypos).flag & PEOPLE_KILLER) && mypos==RINFO.voted[mypos])
						{
							PINFO(mypos).flag|=PEOPLE_SURRENDER;
							sprintf (buf, "\x1b[32;1m%d %s\x1b[m 崩溃了...\33[m", 
								mypos + 1, PINFO(mypos).nick);
						}
						else
							  sprintf (buf, "\x1b[32;1m%d %s\x1b[m 指证 %d %s\33[m",
								mypos + 1, PINFO(mypos).nick,
								  RINFO.voted[mypos]+1,PINFO((int)RINFO.voted[mypos]).nick);
						send_msg(-1, buf);
						sprintf(buf, "\x1b[32;1m%d %s\x1b[m: \33[31;1mOver.\33[m", mypos + 1, PINFO(mypos).nick);
						send_msg(-1, buf);
						
						if (!check_win())
						{
							PINFO((int)RINFO.voted[mypos]).flag|=PEOPLE_VOTED;
							for(i=0;i<MAX_PLAYER;i++)
								RINFO.voted[i]=-1;
						
							while (1)
							{
								RINFO.turn++;
								if (RINFO.turn>=MAX_PLAYER)
									RINFO.turn=0;
								if (RINFO.turn==RINFO.victim)
									break;
								if (PINFO(RINFO.turn).style!=-1 
									&& (PINFO(RINFO.turn).flag & PEOPLE_ALIVE) )
									break;
							}
							if (RINFO.turn!=RINFO.victim)
							{
								sprintf (buf,"请\33[32;1m %d %s \33[m发言", RINFO.turn + 1, PINFO(RINFO.turn).nick);
								send_msg (-1, buf);
								send_msg (RINFO.turn, "请\x1b[31;1m上下移动光标\x1b[m，在您怀疑的对象上");
								send_msg (RINFO.turn, "按\33[31;1mCtrl+S\33[m指证");
								send_msg (RINFO.turn, "指证完后，用\33[31;1mCtrl+T\33[m结束发言");
								if (PINFO(RINFO.turn).flag & PEOPLE_KILLER)
									send_msg (RINFO.turn, "\33[31;1m把光标移到自己位置上，用Ctrl+S可自杀\33[m");
							}
							else
							{
								RINFO.round=1;
								goto_defence();
							 
							}
						}
					}
					kill_msg(-1);
					break;
				case INROOM_DEFENCE:
					if (RINFO.turn==mypos && (PINFO(mypos).flag & PEOPLE_VOTED))
					{
						if ((PINFO(mypos).flag & PEOPLE_KILLER) && mypos==RINFO.voted[mypos])
						{
							PINFO(mypos).flag|=PEOPLE_SURRENDER;
							sprintf (buf, "\x1b[32;1m%d %s\x1b[m 崩溃了...\33[m", mypos + 1, PINFO(mypos).nick);
						}
						else
							  sprintf (buf, "\x1b[32;1m%d %s\x1b[m: \33[31;1mOver.\33[m", mypos + 1, PINFO(mypos).nick);
						send_msg (-1, buf);

						if (!check_win())
						{
							while (1)
							{
								if (RINFO.round%2)
									RINFO.turn--;
								else
									RINFO.turn++;
								if (RINFO.turn>=MAX_PLAYER)
									RINFO.turn=0;
								if (RINFO.turn<0)
									RINFO.turn=MAX_PLAYER-1;
								if (RINFO.turn==RINFO.victim)
									break;
								if (PINFO(RINFO.turn).style!=-1 
									&& (PINFO(RINFO.turn).flag & PEOPLE_ALIVE)
									&& (PINFO(RINFO.turn).flag & PEOPLE_VOTED))
									break;
							}
							if (RINFO.turn!=RINFO.victim)
							{
								sprintf (buf,"请\33[32;1m %d %s \33[m辩护", RINFO.turn + 1, PINFO(RINFO.turn).nick);
								send_msg (-1, buf);
								send_msg (RINFO.turn, "辩护完后，用\33[31;1mCtrl+T\33[m结束");
								if (PINFO(RINFO.turn).flag & PEOPLE_KILLER)
									send_msg (RINFO.turn, "\33[31;1m把光标移到自己位置上，用Ctrl+S可自杀\33[m");
	
							}
							else
							{
								goto_vote();
		
							}
						}
					}
					kill_msg(-1);
					break;
				case INROOM_VOTE:
					if ( RINFO.voted[mypos]!=-1
						&& PINFO(mypos).vote==-1 )
					{
						sprintf (buf, "\x1b[32;1m你投票给 %d %s\33[m", 
							RINFO.voted[mypos]+1,PINFO((int)RINFO.voted[mypos]).nick);
						send_msg (mypos, buf);
						PINFO(mypos).vote=RINFO.voted[mypos];
						vote_result();
						kill_msg(-1);
					}
					break;
				case INROOM_DARK:
					if (RINFO.victim==mypos)
					{
						sprintf (buf, "\x1b[35;1m%d %s\x1b[m: \33[31;1mOver.\33[m", mypos + 1, PINFO(mypos).nick);
						send_msg (-1, buf);
						sprintf (buf, "那么，请\33[35;1m %d %s \33[m朋友以旁观者的身份，继续游戏...", mypos + 1, PINFO(mypos).nick);
						send_msg (-1, buf);
						PINFO(mypos).flag&=~PEOPLE_ALIVE;
						goto_night();
						kill_msg(-1);
					}
					break;
				}
			} 
			else if (mypos<MAX_PLAYER
				&& (PINFO(mypos).flag & PEOPLE_ALIVE)
				&& ch == Ctrl('S'))
			{
				int sel;
				sel = getpeople(selected);
				if (sel!=-1 && sel<MAX_PLAYER
					&& PINFO(sel).style!=-1)
				{
					switch(RINFO.status)
					{
					case INROOM_STOP:
						break;
					case INROOM_NIGHT:
						if ((PINFO(mypos).flag & PEOPLE_KILLER) 
							&& RINFO.victim==-1
							&& (PINFO(sel).flag & PEOPLE_ALIVE))
						{
							RINFO.turn=sel;
							if (RINFO.nokill)
								sprintf (buf, "想吓唬\33[32;1m%d %s \33[m吗?",
									RINFO.turn + 1,
									PINFO(RINFO.turn).nick);
							else
								sprintf (buf, "想杀\33[32;1m%d %s \33[m吗?",
									RINFO.turn + 1,
									PINFO(RINFO.turn).nick);
							for(i=0;i<MAX_PLAYER;i++)
								if (PINFO(i).style!=-1
									&& (PINFO(i).flag & PEOPLE_ALIVE)
									&& (PINFO(i).flag & PEOPLE_KILLER))
								{
									send_msg(i,buf);
									kill_msg(i);
								}
						}
						else if ((PINFO(mypos).flag & PEOPLE_POLICE)
							&& RINFO.informant==-1 
							&& (PINFO(sel).flag & PEOPLE_ALIVE)
							&& !(PINFO(sel).flag & PEOPLE_POLICE))
						{
							RINFO.round=sel;
							sprintf (buf, "想查\33[32;1m%d %s \33[m吗?", RINFO.round + 1, PINFO(RINFO.round).nick);
							for(i=0;i<MAX_PLAYER;i++)
								if (PINFO(i).style!=-1
									&& (PINFO(i).flag & PEOPLE_ALIVE)
									&& (PINFO(i).flag & PEOPLE_POLICE))
								{
									send_msg(i,buf);
									kill_msg(i);
								}
						}
						break;
					case INROOM_DAY:
						break;
					case INROOM_CHECK:
						if (RINFO.turn==mypos
							&& (PINFO(sel).flag & PEOPLE_ALIVE))
						{
							if (mypos==sel)
							{
								if (PINFO(mypos).flag & PEOPLE_KILLER)
								{
									if (RINFO.voted[mypos]!=mypos)
									{
										sprintf (buf, "\33[31m你想崩溃了\33[m");
										RINFO.voted[mypos]=sel;
									}
									else
									{
										sprintf(buf,"\33[31m你不想崩溃了\33[m");
										RINFO.voted[mypos]=-1;
									}
								}
								else
								{
									sprintf(buf,"不能指证自己");
								}
							}
							else
							{

								sprintf (buf, "想指证\33[32;1m%d %s \33[m吗?", sel + 1, PINFO(sel).nick);
								RINFO.voted[mypos]=sel;
								 
							}
							send_msg(mypos,buf);
							kill_msg(mypos);
		
						}
						break;
					case INROOM_DEFENCE:
						if (RINFO.turn==mypos
							&& (PINFO(sel).flag & PEOPLE_ALIVE)
							&& (PINFO(sel).flag & PEOPLE_KILLER)
							&& mypos==sel)
						{	
							if (RINFO.voted[mypos]!=mypos)
							{
								  sprintf (buf, "\33[31m你想崩溃了\33[m");
								RINFO.voted[mypos]=sel;
							  }
							else
							{
								sprintf(buf,"\33[31m你不想崩溃了\33[m");
								RINFO.voted[mypos]=-1;
							}
 
							send_msg(mypos,buf);
							kill_msg(mypos);
		
						}
						break;
					case INROOM_VOTE:
						if ((PINFO(sel).flag & PEOPLE_VOTED)
							&& PINFO(mypos).vote==-1)
						{
							RINFO.voted[mypos]=sel;
							sprintf (buf, "想投票给\33[32;1m%d %s \33[m吗?", 
								sel + 1, PINFO(sel).nick);
							send_msg(mypos,buf);
							kill_msg(mypos);
						}
						break;
					case INROOM_DARK:
						break;
					}
				}
			}
			else if (ch <= 0 && !buf[0]) 
			{
				j = do_com_menu();
				if (kicked)
					goto quitgame;
				if (j == 1)
					goto quitgame;
			}
			else if (ch <= 0)
			{
				break;
			}
		} while (1);
		
		if (mypos<MAX_PLAYER  && (PINFO(mypos).flag & PEOPLE_ALIVE))
		{
			if (RINFO.status ==INROOM_VOTE)
				continue;
			if ((RINFO.status == INROOM_NIGHT)
				&& !(PINFO(mypos).flag & PEOPLE_POLICE)
				&& !(PINFO(mypos).flag & PEOPLE_KILLER))
				continue;
			if ((RINFO.status ==INROOM_DARK)
				&& RINFO.victim!=mypos)
				continue;
			if ((RINFO.status ==INROOM_DAY)
				&& RINFO.victim!=mypos)
				continue;
			if ((RINFO.status ==INROOM_CHECK)
				&& RINFO.turn!=mypos)
				continue;		  
			if ((RINFO.status ==INROOM_DEFENCE)
				&& RINFO.turn!=mypos)
				continue;
		}
		ct=time(NULL);
		if (!denytalk && ct==lasttalktime)
		{
//			send_msg(mypos,"你说话太快，禁止发言5秒钟");
//			denytalk=1;
//			kill_msg(mypos);
//			continue;
		}
		if (denytalk && ct-lasttalktime>5)
		{
			denytalk=0;
		}
		if (denytalk)
			continue;
		start_change_inroom();
	  
		lasttalktime=ct;
		me = mypos;
		strcpy(msg, buf);
		if (msg[0] == '/' && msg[1] == '/')
		{
			i = 2;
			while (msg[i] != ' ' && i < strlen(msg))
				i++;
			strcpy(buf, msg + 2);
			buf[i - 2] = 0;
			while (msg[i] == ' ' && i < strlen(msg))
				i++;
			buf2[0] = 0;
			buf3[0] = 0;
			if (msg[i - 1] == ' ' && i < strlen(msg))
			{
				k = i;
				while (msg[k] != ' ' && k < strlen(msg))
					k++;
				strcpy(buf2, msg + i);
				buf2[k - i] = 0;
				i = k;
				while (msg[i] == ' ' && i < strlen(msg))
					i++;
				if (msg[i - 1] == ' ' && i < strlen(msg))
				{
					k = i;
					while (msg[k] != ' ' && k < strlen(msg))
						k++;
					strcpy(buf3, msg + i);
					buf3[k - i] = 0;
				}
			}
			k = 1;
			for (i = 0;; i++)
			{
				if (!party_data[i].verb)
					break;
				if (!strcasecmp(party_data[i].verb, buf))
				{
					k = 0;
					sprintf(buf, "%s \x1b[1m%s\x1b[m %s", party_data[i].part1_msg, buf2[0] ? buf2 : "大家", party_data[i].part2_msg);
					break;
				}
			}
			if (k)
				for (i = 0;; i++)
				{
					if (!speak_data[i].verb)
						break;
					if (!strcasecmp(speak_data[i].verb, buf))
					{
						k = 0;
						sprintf(buf, "%s: %s", speak_data[i].part1_msg, buf2);
						break;
					}
				}
			if (k)
				for (i = 0;; i++)
				{
					if (!condition_data[i].verb)
						break;
					if (!strcasecmp(condition_data[i].verb, buf)) 
					{
						k = 0;
						sprintf(buf, "%s", condition_data[i].part1_msg);
						break;
					}
				}

			if (k)
				continue;
			strcpy(buf2, buf);
			sprintf(buf, "\x1b[1m%d %s\x1b[m %s", me + 1, PINFO(me).nick, buf2);
		} 
		else
		{
			strcpy(buf2, buf);
			sprintf(buf, "%d %s: %s", me + 1, PINFO(me).nick, buf2);
		}
		if (PINFO(me).flag & PEOPLE_DENYSPEAK)
		{
			send_msg(i, "你现在没有发言权");
		} 
		else
		{
			if (RINFO.status!=INROOM_STOP 
				&& mypos<MAX_PLAYER 
				&& (PINFO(mypos).flag & PEOPLE_ALIVE))
			{
				if (mypos<MAX_PLAYER
					&& (RINFO.status == INROOM_NIGHT) 
					&& PINFO(me).flag & PEOPLE_KILLER)
				{
					for (i = 0; i < MAX_PLAYER; i++)
						if (PINFO(i).style != -1
							&& (PINFO(i).flag & PEOPLE_ALIVE)
							&& (PINFO(i).flag & PEOPLE_KILLER)) 
						{
							send_msg(i, buf);
							kill_msg(i);
						}
			   
				}
				else
				if (mypos<MAX_PLAYER
					&& (RINFO.status == INROOM_NIGHT) 
					&& PINFO(me).flag & PEOPLE_POLICE)
				{
					for (i = 0; i < MAX_PLAYER; i++)
						if (PINFO(i).style != -1
							&& (PINFO(i).flag & PEOPLE_ALIVE)
							&& (PINFO(i).flag & PEOPLE_POLICE))
						{
							send_msg(i, buf);
							kill_msg(i);
						}
					
				}
				else
				{
					send_msg(-1,buf);
					kill_msg(-1);
				}
		  
			}
			else
			if (RINFO.status!=INROOM_STOP)
			{
				strcpy(buf2,"\33[35;1m*");
				strcat(buf2,buf);
				strcat(buf2,"\33[m");
				for (i = 0; i < MAX_PEOPLE; i++)
				if (PINFO(i).style != -1)
				{
					if (i>=MAX_PLAYER || !(PINFO(i).flag & PEOPLE_ALIVE))
					{
						send_msg(i, buf2);
						kill_msg(i);
					}
				}
			}
			else
			{
				send_msg(-1,buf);
				kill_msg(-1);
			}
		}
		end_change_inroom();
		//kill_msg(-1);
	}

	quitgame:
	start_change_inroom();
	me = mypos;
	if (rooms[myroom].op==me)
	{
		int k;
		for (k = 0; k < MAX_PEOPLE; k++)
			if (PINFO(k).style != -1)
				if (k != me)
				{
					rooms[myroom].op=k;
					sprintf(buf, "\x1b[34;1m%d %s成为新房主\x1b[m", k+1,PINFO(k).nick);
					send_msg(-1, buf);
					kill_msg(-1);
					break;
				}
		if (k>=MAX_PEOPLE)
		{
			rooms[myroom].op=-1;
			send_msg(-1, "房间解散了");
			kill_msg(-1);
			rooms[myroom].style = -1;
			end_change_inroom();
			goto quitgame2;
		}
	}
	if (PINFO(me).style != -1)
	{
		PINFO(me).style = -1;
		if (me>=MAX_PLAYER)
			rooms[myroom].numspectator--;
		else
			rooms[myroom].numplayer--;
		kill_msg(-1);
	}
	end_change_inroom();

/*
	if(killer)
		sprintf(buf, "杀手%s潜逃了", buf2);
	else
		sprintf(buf, "%s离开房间", buf2);
	for(i=0;i<myroom->people;i++)
	{
		send_msg(inrooms.peoples+i, buf);
		kill(inrooms.peoples[i].pid, SIGUSR1);
	}
*/
quitgame2:
	kicked = 0;
	sprintf(buf, "tmp/%d.msg", rand());
	save_msgs(buf);
	sprintf(buf2, "\"%s\"的杀人记录", roomname);
	k_getdata(t_lines - 1, 0, "寄回本次全部信息吗?[y/N]", buf3, 3, 1, 0, 1);
	if (toupper(buf3[0]) == 'Y')
	{
		k_mail_file(CURRENTUSER->userid, buf, CURRENTUSER->userid, buf2, BBSPOST_MOVE, NULL);
	}
	else
		unlink(buf);
	//kill_msg(-1);
	signal(SIGUSR1, DEFAULT_SIGNAL);
}

static int room_list_refresh(struct _select_def *conf)
{
	clear();
	docmdtitle("[游戏室选单]",
			   "  退出[\x1b[1;32m←\x1b[0;37m,\x1b[1;32me\x1b[0;37m] 进入[\x1b[1;32mEnter\x1b[0;37m] 选择[\x1b[1;32m↑\x1b[0;37m,\x1b[1;32m↓\x1b[0;37m] 添加[\x1b[1;32ma\x1b[0;37m] 加入[\x1b[1;32mJ\x1b[0;37m] \x1b[m	  作者: \x1b[31;1mbad@smth.org\x1b[m");
	move(2, 0);
	prints("\x1b[37;44;1m    %4s %-14s %-12s %4s %4s %6s %-20s", "编号", "游戏室名称", "管理员", "人数", "最多", "锁隐旁", "话题");
	clrtoeol();
//	k_resetcolor();
	update_endline();
	return SHOW_CONTINUE;
}

static int room_list_show(struct _select_def *conf, int i)
{
	struct room_struct *r;
	int j = room_get(i - 1);

	if (j != -1)
	{
		r = rooms + j;
		prints("  %3d  %-14s %-12s %3d  %3d  %2s%2s%2s %-36s", 
			i, r->name, inrooms[j].peoples[r->op].id,
			r->numplayer, r->maxplayer, (r->flag & ROOM_LOCKED) ? "√" : "",
			(r->flag & ROOM_SECRET) ? "√" : "",
			(!(r->flag & ROOM_DENYSPEC)) ? "√" : "", r->title);
	}
	return SHOW_CONTINUE;
}

static int room_list_select(struct _select_def *conf)
{
	struct room_struct *r, *r2;
	int i = room_get(conf->pos - 1), j;
	char ans[4];
	int spec;

	if (i == -1)
		return SHOW_CONTINUE;
	r = rooms + i;
	j = find_room(r->name);
	if (j == -1)
	{
		move(0, 0);
		clrtoeol();
		prints(" 该房间已被锁定!");
		refresh();
		sleep(1);
		return SHOW_REFRESH;
	}
	k_getdata(0, 0, "是否以旁观者身份进入? [y/N]", ans, 3, 1, NULL, 1);
	spec=(toupper(ans[0]) == 'Y');
	j = find_room(r->name);
	if (j == -1)
	{
		move(0, 0);
		clrtoeol();
		prints(" 该房间已被锁定!");
		refresh();
		sleep(1);
		return SHOW_REFRESH;
	}
	r2 = rooms + j;
	if (spec && r2->flag & ROOM_DENYSPEC && !K_HAS_PERM(CURRENTUSER, PERM_SYSOP))
	{
		move(0, 0);
		clrtoeol();
		prints(" 该房间拒绝旁观者");
		refresh();
		sleep(1);
		return SHOW_REFRESH;
	}
	if ((!spec && r2->numplayer >= r2->maxplayer && !K_HAS_PERM(CURRENTUSER, PERM_SYSOP))
	    ||(!spec && r2->numplayer>=MAX_PLAYER)
		||(spec && r2->numspectator >= MAX_PEOPLE-MAX_PLAYER))
	{
		move(0, 0);
		clrtoeol();
		prints(" 该房间人数已满");
		refresh();
		sleep(1);
		return SHOW_REFRESH;
	}
	join_room(find_room(r2->name), spec);
	return SHOW_DIRCHANGE;
}

static int room_list_k_getdata(struct _select_def *conf, int pos, int len)
{
	clear_room();
	conf->item_count = room_count();
	return SHOW_CONTINUE;
}

static int room_list_prekey(struct _select_def *conf, int *key)
{
	switch (*key)
	{
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
	int i, j;
	char name[40], ans[4];
	int spec;

	switch (key)
	{
	case 'a':
		r.op=-1;
		k_getdata(0, 0, "房间名:", name, 13, 1, NULL, 1);
		if (!name[0])
			return SHOW_REFRESH;
		if (name[0] == ' ' || name[strlen(name) - 1] == ' ')
		{
			move(0, 0);
			clrtoeol();
			prints(" 房间名开头结尾不能为空格");
			refresh();
			sleep(1);
			return SHOW_CONTINUE;
		}
		strcpy(r.name, name);
		r.style = 1;
		r.flag = ROOM_SPECBLIND;
		r.numplayer = 0;
		r.numspectator=0;
		r.maxplayer = MAX_PLAYER;
		r.op=-1;
		strcpy(r.title, "我杀我杀我杀杀杀");
		if (add_room(&r) == -1)
		{
			move(0, 0);
			clrtoeol();
			prints(" 有一样名字的房间啦!");
			refresh();
			sleep(1);
			return SHOW_REFRESH;
		}
		join_room(find_room(r.name), 0);
		return SHOW_DIRCHANGE;
	case 'J':
		k_getdata(0, 0, "房间名:", name, 12, 1, NULL, 1);
		if (!name[0])
			return SHOW_REFRESH;
		if ((i = find_room(name)) == -1)
		{
			move(0, 0);
			clrtoeol();
			prints(" 没有找到该房间!");
			refresh();
			sleep(1);
			return SHOW_REFRESH;
		}
		k_getdata(0, 0, "是否以旁观者身份进入? [y/N]", ans, 3, 1, NULL, 1);
		spec=(toupper(ans[0])=='Y');
		if ((i = find_room(name)) == -1) 
		{
			move(0, 0);
			clrtoeol();
			prints(" 没有找到该房间!");
			refresh();
			sleep(1);
			return SHOW_REFRESH;
		}
        r2 = rooms + i;
		if (spec && r2->flag & ROOM_DENYSPEC && !K_HAS_PERM(CURRENTUSER, PERM_SYSOP)) 
		{
			move(0, 0);
			clrtoeol();
			prints(" 该房间拒绝旁观者");
			refresh();
			sleep(1);
			return SHOW_REFRESH;
		}
		if ((!spec && r2->numplayer >= r2->maxplayer && !K_HAS_PERM(CURRENTUSER, PERM_SYSOP))
			||(!spec && r2->numplayer>=MAX_PLAYER)
			||(spec && r2->numspectator >= MAX_PEOPLE-MAX_PLAYER))
		{
			move(0, 0);
			clrtoeol();
			prints(" 该房间人数已满");
			refresh();
			sleep(1);
			return SHOW_REFRESH;
		}
		join_room(find_room(name), toupper(ans[0]) == 'Y');
		return SHOW_DIRCHANGE;
	case 'K':
		if (!K_HAS_PERM(CURRENTUSER, PERM_SYSOP))
			return SHOW_CONTINUE;
		i = room_get(conf->pos - 1);
		if (i != -1)
		{
			r2 = rooms + i;
			r2->style = -1;
			for (j = 0; j < MAX_PEOPLE; j++)
				if (inrooms[i].peoples[j].style != -1)
					kill(inrooms[i].peoples[j].pid, SIGUSR1);
		}
		return SHOW_DIRCHANGE;
	}
	return SHOW_CONTINUE;
}
/*
void show_top_board()
{
	FILE *fp;
	char buf[80];
	int i, j, x, y;

	clear();
	for (i = 1; i <= 6; i++) 
	{
		sprintf(buf, "service/killer.%d", i);
		fp = fopen(buf, "r");
		if (!fp)
			return;
		for (j = 0; j < 7; j++)
		{
			if (feof(fp))
				break;
			y = (i - 1) % 3 * 8 + j;
			x = (i - 1) / 3 * 40;
			if (fgets(buf, 80, fp) == NULL)
				break;
			move(y, x);
			k_resetcolor();
			if (j == 2)
				k_setfcolor(RED, 1);
			prints("%s", buf);
		}
		fclose(fp);
	}
	pressanykey();
}
*/
int choose_room()
{
	struct _select_def grouplist_conf;
	int i;
	POINT *pts;

	clear_room();
	bzero(&grouplist_conf, sizeof(struct _select_def));
	grouplist_conf.item_count = room_count();
	if (grouplist_conf.item_count == 0)
	{
		grouplist_conf.item_count = 1;
	}
	pts = (POINT *) malloc(sizeof(POINT) * BBS_PAGESIZE);
	for (i = 0; i < BBS_PAGESIZE; i++) 
	{
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
	grouplist_conf.pos = 1;	 /* initialize cursor on the first mailgroup */
	grouplist_conf.page_pos = 1;		/* initialize page to the first one */

	grouplist_conf.on_select = room_list_select;
	grouplist_conf.show_data = room_list_show;
	grouplist_conf.pre_key_command = room_list_prekey;
	grouplist_conf.key_command = room_list_key;
	grouplist_conf.show_title = room_list_refresh;
	grouplist_conf.get_data = room_list_k_getdata;
//	show_top_board();
	list_select_loop(&grouplist_conf);
	free(pts);
    return 0;
}

int killer_main()
{
	int i, oldmode;
	void *shm, *shm2;
	shm = k_attach_shm("GAMEROOM2_SHMKEY", 3452, sizeof(struct room_struct) * MAX_ROOM, &i);
	rooms = shm;
	if (i) 
	{
		for (i = 0; i < MAX_ROOM; i++)
		{
			rooms[i].style = -1;
			rooms[i].w = 0;
			rooms[i].level = 0;
		}
	}
	shm2 = k_attach_shm("KILLER2_SHMKEY", 9579, sizeof(struct inroom_struct) * MAX_ROOM, &i);
	inrooms = shm2;
	if (i) 
	{
		for (i = 0; i < MAX_ROOM; i++)
			inrooms[i].w = 0;
	}
	oldmode = uinfo.mode;
	lasttalktime=-1;
	denytalk=0;
	modify_user_mode(KILLER);
	choose_room();
	modify_user_mode(oldmode);
	shmdt(shm);
	shmdt(shm2);
    return 0;
}

