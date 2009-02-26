#include "service.h"
#include "bbs.h"

/*--------------------------------------------------------------------------*/
/*  小鸡参数设定                               */
/*--------------------------------------------------------------------------*/
struct chicken {
    /*---基本的资料---*///14
    char name[20];  /*姓    名 */
    char birth[21];  /*生    日 */
    int year;  /*生日  年 */
    int month;  /*生日  月 */
    int day;  /*生日  日 */
    int sex;  /*性    别 1:♂   2:♀  */
    int death;  /*1:  死亡 2:抛弃 3:结局 */
    int nodone;  /*1:  未做 */
    int relation;  /*两人关系 */
    int liveagain;  /*复活次数 */
    int dataB;
    int dataC;
    int dataD;
    int dataE;

    /*---身体的参数---*///12
    int hp;   /*体    力 */
    int maxhp;  /*最大体力 */
    int weight;  /*体    重 */
    int tired;  /*疲 劳 度 */
    int sick;  /*病    气 */
    int shit;  /*清 洁 度 */
    int wrist;  /*腕    力 */
    int bodyA;
    int bodyB;
    int bodyC;
    int bodyD;
    int bodyE;

    /*---评价的参数---*///9
    int social;  /*社交评价 */
    int family;  /*家事评价 */
    int hexp;  /*战斗评价 */
    int mexp;  /*魔法评价 */
    int tmpA;
    int tmpB;
    int tmpC;
    int tmpD;
    int tmpE;

    /*---战斗用参数---*///13
    int mp;   /*法    力 */
    int maxmp;  /*最大法力 */
    int attack;  /*攻 击 力 */
    int resist;  /*防 御 力 */
    int speed;  /*速    度 */
    int hskill;  /*战斗技术 */
    int mskill;  /*魔法技术 */
    int mresist;  /*抗魔能力 */
    int magicmode;  /*魔法型态 */
    int fightB;
    int fightC;
    int fightD;
    int fightE;

    /*---武器的参数---*///10
    int weaponhead;  /*头部武器 */
    int weaponrhand; /*右手武器 */
    int weaponlhand; /*左手武器 */
    int weaponbody;  /*身体武器 */
    int weaponfoot;  /*脚的武器 */
    int weaponA;
    int weaponB;
    int weaponC;
    int weaponD;
    int weaponE;

    /*---各能力参数---*///17
    int toman;  /*待人接物 */
    int character;  /*气 质 度 */
    int love;  /*爱    心 */
    int wisdom;  /*智    慧 */
    int art;  /*艺术能力 */
    int etchics;  /*道    德 */
    int brave;  /*勇    敢 */
    int homework;  /*扫地洗衣 */
    int charm;  /*魅    力 */
    int manners;  /*礼    仪 */
    int speech;  /*谈    吐 */
    int cookskill;  /*烹    饪 */
    int learnA;
    int learnB;
    int learnC;
    int learnD;
    int learnE;

    /*---各状态数值---*///11
    int happy;  /*快 乐 度 */
    int satisfy;  /*满 意 度 */
    int fallinlove;  /*恋爱指数 */
    int belief;  /*信    仰 */
    int offense;  /*罪    孽 */
    int affect;  /*感    受 */
    int stateA;
    int stateB;
    int stateC;
    int stateD;
    int stateE;

    /*---吃的东西啦---*///9
    int food;  /*食    物 */
    int medicine;  /*灵    芝 */
    int bighp;  /*大 补 丸 */
    int cookie;  /*零    食 */
    int ginseng;  /*千年人参 */
    int snowgrass;  /*天山雪莲 */
    int eatC;
    int eatD;
    int eatE;

    /*---拥有的东西---*///8
    int book;  /*书    本 */
    int playtool;  /*玩    具 */
    int money;  /*金    钱 */
    int thingA;
    int thingB;
    int thingC;
    int thingD;
    int thingE;

    /*---猜拳的参数---*///2
    int winn;
    int losee;

    /*---参见王臣--*///16
    int royalA;  /*from守卫 */
    int royalB;  /*from近卫 */
    int royalC;  /*from将军 */
    int royalD;  /*from大臣 */
    int royalE;  /*from祭司 */
    int royalF;  /*from宠妃 */
    int royalG;  /*from王妃 */
    int royalH;  /*from国王 */
    int royalI;  /*from小丑 */
    int royalJ;  /*from王子 */
    int seeroyalJ;  /*是否已经看过王子了 */
    int seeA;
    int seeB;
    int seeC;
    int seeD;
    int seeE;

    /*---结局----*///2
    int wantend;  /*20岁结局 */
    /*1:不要且未婚 2:不要且已婚  3:不要且当第三者 4:要且未婚  5:要且已婚 6:要且当第三者 */
    int lover;  /*爱人 */
    /*0:没有 1:魔王 2:龙族 3:A 4:B 5:C 6:D 7:E */

    /*-------工作次数--------*/
    int workA;  /*家事 */
    int workB;  /*保姆 */
    int workC;  /*旅店 */
    int workD;  /*农场 */
    int workE;  /*餐厅 */
    int workF;  /*教堂 */
    int workG;  /*地摊 */
    int workH;  /*伐木 */
    int workI;  /*美发 */
    int workJ;  /*猎人 */
    int workK;  /*工地 */
    int workL;  /*守墓 */
    int workM;  /*家教 */
    int workN;  /*酒家 */
    int workO;  /*酒店 */
    int workP;  /*夜 */
    int workQ;
    int workR;
    int workS;
    int workT;
    int workU;
    int workV;
    int workW;
    int workX;
    int workY;
    int workZ;

    /*-------上课次数--------*/
    int classA;
    int classB;
    int classC;
    int classD;
    int classE;
    int classF;
    int classG;
    int classH;
    int classI;
    int classJ;
    int classK;
    int classL;
    int classM;
    int classN;
    int classO;

    /*---小鸡的时间---*///1
    int bbtime;
};
typedef struct chicken chicken;

/*--------------------------------------------------------------------------*/
/*  选单的设定                                                              */
/*--------------------------------------------------------------------------*/
struct pipcommands {
    int (*fptr)();
    int key1;
    int key2;
};
typedef struct pipcommands pipcommands;

/*--------------------------------------------------------------------------*/
/*  怪物参数设定                                                            */
/*--------------------------------------------------------------------------*/
struct playrule {
    //char *name;           /*名字*/
    char name[20];
    int hp;   /*体力 */
    int maxhp;
    /*MAXHP*/ int mp; /*法力 */
    int maxmp;
    /*MAXMP*/ int attack; /*攻击 */
    int resist;  /*防护 */
    int speed;  /*速度 */
    int money;  /*财宝 */
    char special[6]; /*特别 *//*1:怕火 2:怕冰 */
    int map;  /*图档 */
    int death;  /*死活 */
};
typedef struct playrule playrule;

/* d.special的定义            */
/* 00000 第一系的魔法伤害较小 */
/* 10000 第一系的魔法伤害较大 */
/* 11000 一二系的魔法伤害较大 */
/* 以下类推                    */
/* 雷系 冰系 火系 土系 风系 */
/*struct playrule goodmanlist[] = {
}*/

#define PIP_BADMAN       (3) /* 怪物整体个数  */

/*--------------------------------------------------------------------------*/
/*  魔法参数设定                                                            */
/*--------------------------------------------------------------------------*/
struct magicset {
    //char *name;           /*魔法的名字*/
    char name[20];
    int needmp;  /*需要的法力 */
    int hpmode;  /*0:减少 1:增加 2:最大值 */
    int hp;   /*杀伤生命力 */
    int tiredmode;  /*0:减少 1:增加 2:最小值 */
    int tired;  /*疲劳度改变 */
    int map;  /*图档 */
};
typedef struct magicset magicset;

/*--------------------------------------------------------------------------*/
/*  参见王臣参数设定                                                        */
/*--------------------------------------------------------------------------*/
struct royalset {
    //char *num;          /*代码*/
    //char *name;           /*王臣的名字*/
    char num[2];
    char name[20];
    int needmode;  /*需要的mode *//*0:不需要 1:礼仪 2:谈吐 */
    int needvalue;  /*需要的value */
    int addtoman;  /*最大的增加量 */
    int maxtoman;  /*库存量 */
    //char *words1;
    //char *words2;
    char words1[40];
    char words2[40];
};
typedef struct royalset royalset;

struct goodsofpip {
    int num;        /*编号 */
    char *name;     /*名字 */
    char *msgbuy;       /*功用 */
    char *msguse;       /*说明 */
    int money;      /*金钱 */
    int change;     /*改变量 */
    int pic1;
    int pic2;
};
typedef struct goodsofpip goodsofpip;

struct weapon {
    char *name;     /*名字 */
    int needmaxhp;      /*需要hp */
    int needmaxmp;      /*需要mp */
    int needspeed;      /*需要的speed */
    int attack;     /*攻击 */
    int resist;     /*防护 */
    int speed;      /*速度 */
    int cost;       /*买价 */
    int sell;       /*卖价 */
    int special;        /*特别 */
    int map;        /*图档 */
};
typedef struct weapon weapon;

struct newendingset {
    char *girl;     /*女生结局的职业 */
    char *boy;      /*男生结局的职业 */
    int grade;      /*评分 */
};
typedef struct newendingset newendingset;

#undef MAPLE /* etnlegend, 2006.04.08, 这个就算了吧... */

#define MSG_UID "请输入使用者代号："
#define ERR_UID "这里没有这个人啦！"
#define b_lines t_lines-1
SMTH_API int t_lines;

int show_system_pic(int);
void temppress(const char*);

int clrchyiuan(int,int);
int count_tired(int,int,char*,int,int);
int lose();
int pip_basic_feed();
int pip_buy_goods_new(int,const struct goodsofpip*,int*);
int pip_data_list();
int pipdie(const char*,int);
int pip_endingall_purpose(char*,int*,int*,int*,int);
int pip_endingart(char*,int*,int*,int*);
int pip_endingblack(char*,int*,int*,int*);
int pip_endingcombat(char*,int*,int*,int*);
int pip_ending_decide(char*,char*,char*,int*,int*);
int pip_endingfamily(char*,int*,int*,int*);
int pip_endingmagic(char*,int*,int*,int*);
int pip_ending_screen();
int pip_endingsocial(char*,int*,int*,int*);
int pip_fight_bad(int);
int pip_fight_main(int,const struct playrule*,int);
int pip_future_decide(int*);
int pip_game_over(int);
int pip_go_palace_screen(const struct royalset*);
int pip_live_again();
void pip_log_record(char*);
int pip_magic_doing_menu(const struct magicset*);
int pip_magic_menu();
int pip_mainmenu(int);
int pip_main_menu();
int pip_marriage_offer();
int pip_marry_decide();
int pip_max_worktime(int*);
int pip_new_game();
int pip_practice_function(int,int,int,int,int*,int*,int*,int*,int*);
int pip_practice_gradeup(int,int,int);
int pip_read(char*);
int pip_read_backup();
void pip_read_file();
int pip_results_show_ending(int,int,int,int,int);
int pip_time_change(time_t);
int pip_vs_man(int,const struct playrule*,int);
int pip_weapon_doing_menu(int,int,const struct weapon*);
void pip_write_file();
int show_badman_pic(int);
int show_basic_pic(int);
int show_buy_pic(int);
int show_die_pic(int);
int show_ending_pic(int);
int show_feed_pic(int);
int show_fight_pic(int);
int show_file(char*,int,int,int);
int show_guess_pic(int);
int show_job_pic(int);
int show_palace_pic(int);
int show_play_pic(int);
int show_practice_pic(int);
int show_resultshow_pic(int);
int show_special_pic(int);
int show_usual_pic(int);
int show_weapon_pic(int);
int situ();
int tie();
int win();

