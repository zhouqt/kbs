#include "bbs.h"

const char seccode[SECNUM][5] = {
    "1", "2", "3", "4", "5", "6", "0"
};
const char * const groups[] = {
        "GROUP_0",
        "GROUP_1",
        "GROUP_2",
        "GROUP_3",
        "GROUP_4",
        "GROUP_5",
        "GROUP_6",
         NULL
};
const char secname[SECNUM][2][20] = {
    {"本站系统", "[本站]"},
    {"人在美国", "[校园][创业]"},
    {"大话西游", "[留学][学校][团体]"},
    {"学术研讨", "[学术][科学][电脑]"},
    {"生活娱乐", "[休闲][娱乐][生活]"},
    {"知性感性", "[知性][感性][闲聊]"},
    {"内部精华", "[闲人免进]"}
};

const char * const explain[] = {
        "本站系统",
        "人在美国",
        "大话西游",
        "学术研讨",
        "生活娱乐",
        "知性感性",
        "内部精华",
        NULL
};

const char * const permstrings[] = {
    "基本权力",                 /* PERM_BASIC */
    "进入聊天室",               /* PERM_CHAT */
    "呼叫他人聊天",             /* PERM_PAGE */
    "发表文章",                 /* PERM_POST */
    "使用者资料正确",           /* PERM_LOGINOK */
    "实习站务",                 /* PERM_BMMANAGER */
    "可隐身",                   /* PERM_CLOAK */
    "可见隐身",                 /* PERM_SEECLOAK */
    "长期帐号",                 /* PERM_XEMPT */
    "编辑系统档案",             /* PERM_WELCOME */
    "板主",                     /* PERM_BOARDS */
    "帐号管理员",               /* PERM_ACCOUNTS */
    "牧场物语智囊团",           /* PERM_CHATCLOAK */
    "封禁娱乐权限",             /* PERM_DENYRELAX */
    "系统维护管理员",           /* PERM_SYSOP */
    "Read/Post 限制",           /* PERM_POSTMASK */
    "精华区总管",               /* PERM_ANNOUNCE */
    "讨论区总管",               /* PERM_OBOARDS */
    "活动看版总管",             /* PERM_ACBOARD */
    "不能 ZAP(讨论区专用)",     /* PERM_NOZAP */
    "聊天室OP(元老院专用)",     /* PERM_CHATOP */
    "系统总管理员",             /* PERM_ADMIN */
    "荣誉帐号",                 /* PERM_HONOR */
    "6 区总管",                 /* PERM_6SEC */
    "仲裁委员",                 /* PERM_JURY */
    "特殊权限 7",               /* PERM_UNUSE? */
    "自杀进行中",               /*PERM_SUICIDE */
    "集体专用帐号",             /* PERM_COLLECTIVE */
    "看系统讨论版",             /* PERM_UNUSE? */
    "封禁Mail",                 /* PERM_DENYMAIL */

};

struct count_arg {
    int www_count;
    int telnet_count;
};

int countuser(struct user_info *uinfo, struct count_arg *arg, int pos)
{
    if (uinfo->mode == WEBEXPLORE)
        arg->www_count++;
    else
        arg->telnet_count++;
    return COUNT;
}

int multilogin_user(struct userec *user, int usernum, int mode)
{
    int logincount;
    int curr_login_num;
    struct count_arg arg;

    bzero(&arg, sizeof(arg));
    logincount = apply_utmpuid((APPLY_UTMP_FUNC) countuser, usernum, &arg);

    if (logincount < 1)
        RemoveMsgCountFile(user->userid);

    if (HAS_PERM(user, PERM_SYSOP))
        return 0;               /* don't check sysops */
    curr_login_num = get_utmp_number();
    /*
     * binxun 2003.5 仲裁，版主，Chatop，等都可以三登
     */
    if ((HAS_PERM(user, PERM_BOARDS) || HAS_PERM(user, PERM_CHATOP)
         || HAS_PERM(user, PERM_JURY) || HAS_PERM(user, PERM_CHATCLOAK)
         || HAS_PERM(user, PERM_BMAMANGER))
        && logincount < 3)
        return 0;
    
    if (!strcmp("farmers", user->userid)) return 0;
    
    /*
     * allow multiple guest user 
     */
    if (!strcmp("guest", user->userid)) {
        if (logincount > MAX_GUEST_NUM) {
            return 2;
        }
        return 0;
    } else if (((curr_login_num < 30) && (logincount >= 3))     /*小于700可以三登 */
               ||((curr_login_num >= 30) && (logincount >= 2)   /*700人以上 */
                  &&!(((arg.telnet_count == 0) && (mode == 0))  /* telnet个数为零可以再登一个telnet */
                      ||(((arg.www_count == 0) && (mode == 1))))))      /*user login limit */
        return 1;
    return 0;
}

//自动通过注册的函数  binxun
int auto_register(char *userid, char *email, int msize)
{
    struct userdata ud;
    struct userec *uc;
    //char *item;
    char fdata[7][STRLEN];
    char genbuf[STRLEN];
    char buf[STRLEN];
    char fname[STRLEN];
    int unum, isGreen = false;
    FILE *fout;
    int n;
    struct userec deliveruser;
    static const char *finfo[] = { "帐号位置", "申请代号", "真实姓名", "服务单位",
        "目前住址", "连络电话", "生    日", NULL
    };
    static const char *field[] = { "usernum", "userid", "realname", "career",
        "addr", "phone", "birth", NULL
    };

    bzero(&deliveruser, sizeof(struct userec));
    strcpy(deliveruser.userid, "deliver");
    deliveruser.userlevel = -1;
    strcpy(deliveruser.username, "自动发信系统");

    {
        char buf[STRLEN];
        char *ptr;
        FILE *fp;

        if ((fp = fopen("etc/GreenChannel", "r")) != NULL) {
            while (fgets(buf, STRLEN, fp) != NULL) {
                ptr = strtok(buf, " \n\t\r");
                if (ptr != NULL && *ptr != '#') {
                    if (strcasecmp(ptr, email) == 0) {
                        isGreen = true;
                        break;
                    }
                }
                bzero(buf, STRLEN);
            }
            fclose(fp);
        }
    }
    if (!isGreen) return -1;

    bzero(fdata, 7 * STRLEN);

    if ((unum = getuser(userid, &uc)) == 0)
        return -1;              //faild
    if (read_userdata(userid, &ud) < 0)
        return -1;

    strncpy(ud.realemail, email, STRLEN-16);
    /*if (item)
        strncpy(ud.realname, item, NAMELEN);
    item = strtok(NULL, "#");   //学号
    item = strtok(NULL, "#");
    if (item)
        strncpy(ud.address, item, STRLEN);

    email[strlen(email) - 3] = '@';
    strncpy(ud.realemail, email, STRLEN - 16);  //email length must be less STRLEN-16
    */
    

    sprintf(fdata[0], "%d", unum);
    strncpy(fdata[2], ud.realname, NAMELEN);
    strncpy(fdata[4], ud.address, STRLEN);
    strncpy(fdata[5], ud.email, STRLEN);
    strncpy(fdata[1], userid, IDLEN);

    sprintf(buf, "tmp/email/%s", userid);
    if ((fout = fopen(buf, "w")) != NULL) {
        fprintf(fout, "%s\n", email);
        fclose(fout);
    }

    if (write_userdata(userid, &ud) < 0)
        return -1;
    mail_file("deliver", "etc/s_fill", userid, "恭喜你,你已经完成注册.", 0, 0);
    //sprintf(genbuf,"deliver 让 %s 自动通过身份确认.",uinfo.userid);

    sprintf(fname, "tmp/security.%d", getpid());
    if ((fout = fopen(fname, "w")) != NULL) {
        fprintf(fout, "系统安全记录系统\n\033[32m原因：%s自动通过注册\033[m\n", userid);
        fprintf(fout, "以下是通过者个人资料");
        fprintf(fout, "\n\n您的代号     : %s\n", ud.userid);
        fprintf(fout, "您的昵称     : %s\n", uc->username);
        fprintf(fout, "真实姓名     : %s\n", ud.realname);
        fprintf(fout, "电子邮件信箱 : %s\n", ud.email);
        fprintf(fout, "真实 E-mail  : %s\n", ud.realemail);
        fprintf(fout, "服务单位     : %s\n", "");
        fprintf(fout, "目前住址     : %s\n", ud.address);
        fprintf(fout, "连络电话     : %s\n", "");
        fprintf(fout, "注册日期     : %s", ctime(&uc->firstlogin));
        fprintf(fout, "最近光临日期 : %s", ctime(&uc->lastlogin));
        fprintf(fout, "最近光临机器 : %s\n", uc->lasthost);
        fprintf(fout, "上站次数     : %d 次\n", uc->numlogins);
        fprintf(fout, "文章数目     : %d(Board)\n", uc->numposts);
        fprintf(fout, "生    日     : %s\n", "");

        fclose(fout);
        //post_file(currentuser, "", fname, "Registry", str, 0, 2);

        sprintf(genbuf, "%s 自动通过注册", ud.userid);
        post_file(&deliveruser, "", fname, "syslog", genbuf, 0, 1, getSession());
        /*
         * if (( fout = fopen(logfile,"a")) != NULL)
         * {
         * fclose(fout);
         * }
         */
    }

    sethomefile(buf, userid, "/register");
    if ((fout = fopen(buf, "w")) != NULL) {
        for (n = 0; field[n] != NULL; n++)
            fprintf(fout, "%s     : %s\n", finfo[n], fdata[n]);
        fprintf(fout, "您的昵称     : %s\n", uc->username);
        fprintf(fout, "电子邮件信箱 : %s\n", ud.email);
        fprintf(fout, "真实 E-mail  : %s\n", ud.realemail);
        fprintf(fout, "注册日期     : %s\n", ctime(&uc->firstlogin));
        fprintf(fout, "注册时的机器 : %s\n", uc->lasthost);
        fprintf(fout, "Approved: %s\n", userid);
        fclose(fout);
    }

    return 0;
}


char * showuserip(struct userec *user, char *fromhost)
{
    static char *maskip[] = {"128.12.", "171.64.", "171.65.", "171.66.", "171.67.", "192.168.0.", "136.152.", "128.32.", "169.229.", NULL};
    static char *masks[]  = {"ＦＡＲＭ","ＦＡＲＭ","ＦＡＲＭ","ＦＡＲＭ","ＦＡＲＭ","ＦＡＲＭ",   "金狗熊的窝", "金狗熊的窝","金狗熊的窝", NULL};
	static char sip[25];
    int i;
	char *c;
	    
	if( user!=NULL && (!DEFINE(user, DEF_HIDEIP)) )
		return "就不告诉你"; //(user->userid);

    for(i=0;maskip[i]!=NULL;i++) {
        if (!strncmp(fromhost, maskip[i], strlen(maskip[i]))) { 
	        return masks[i];
       }
    }
	strncpy(sip, fromhost, 24);
	sip[24]=0;
	if( (c=strrchr(sip, '.')) != NULL){
		*(++c)='*';
		*(++c)='\0';
	}
	return sip;
}
