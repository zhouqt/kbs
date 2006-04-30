/* etnlegend, 2006.04.08, 信件发送工具 */

/*
 * 命令行:          mails [options] <m_title> <m_path>
 * 参数表:
 *   <m_title>      设定信件的标题为 <m_title>, 标题的最大有效长度为 [MT_LEN] 字节
 *   <m_path>       设定信件文件的路径, 若此参数以字符 `+` 起始, 则表示相对于 [BBSHOME] 的相对路径
 * 选项表:
 *   -p <perm>      启用权限检测, 信件将只发送给具有 <perm> 权限的用户, 默认不启用, 
                    <perm> 以字符 `&` 起始时表示各个权限之间是逻辑与关系, 即所有描述到的权限都是重要的, 
                    默认情况下各个权限之前是逻辑或关系
 *   -u <user>      启用用户检测, 信件将只发送给用户名为 <user> 的用户, 默认不启用, 不能与 -l 选项同时使用
 *   -l <list>      启用用户检测, 信件将只发送给文件 <list> 指定的用户, 默认不启用, 不能与 -u 选项同时使用
 *   -m <mode>      设定发送模式, 有效设定值为 `copy` 和 `link`, `copy` 表示以复制模式发送信件, 
 *                  `link` 表示以链接模式发送信件, 默认模式为 `link`
 *   -s <name>      设定信件的发送者为 <name>, 默认为 `SYSOP`, 发送者的最大有效长度为 [IDLEN + 1] 字节
 *   -v             显示进度信息, 仅当未启用用户检测并且目标用户多于 [MCV_THRESHOLD] 时有效
 *   -h             显示文档信息
 *
*/

#include "bbs.h"

#define MT_LEN ARTICLE_TITLE_LEN                /* 信件标题长度 */
#define MP_LEN _POSIX_PATH_MAX                  /* 信件路径长度 */
#define MB_LEN 128                              /* 缓冲区长度 */
#define MC_PERM 0x01                            /* 权限检测标签 */
#define MC_USER 0x02                            /* 用户检测标签 */
#define MC_LIST 0x04                            /* 列表检测标签 */
#define MC_MODE 0x08                            /* 发送模式标签 */
#define MC_NAME 0x10                            /* 发送者设定标签 */
#define MC_VERB 0x20                            /* 显示进度信息 */
#define MCP_AND_FLAG 0x80000000                 /* 权限检测设定为逻辑与时的标签 */
#define MCV_SEPS 20                             /* 进度显示更新精度 */
#define MCV_THRESHOLD 1000                      /* 进度显示目标用户门槛 */
#define MFR_DELIM " \t\n\r,;:"                  /* 字符串分析间断符 */

static const char* const perm_str=XPERMSTR;     /* 权限符号列表 */
static char title[MT_LEN];                      /* 信件标题 */
static char path[MP_LEN];                       /* 信件路径 */
static unsigned int flag;                       /* 使用选项时的选项标签 */
static unsigned int perm;                       /* 设定权限检测时的被检测权限 */
static char user[IDLEN+1];                      /* 设定用户检测时的用户名 */
static char list[MP_LEN];                       /* 设定列表检测时的列表文件路径 */
static char name[IDLEN+2];                      /* 设定发送者时的发送者名称*/
static int count;                               /* 进度显示当前记数 */
static int total;                               /* 进度显示总记数 */

/* 打印文档信息 */
static inline int m_usage(void){
    fprintf(stdout,
        "  命令行:          mails [options] <m_title> <m_path>\n"
        "  参数表:\n"
        "    <m_title>      设定信件的标题为 <m_title>, 标题的最大有效长度为 [%d] 字节\n"
        "    <m_path>       设定信件文件的路径, 若此参数以字符 `+` 起始, 则表示相对于 [%s] 的相对路径\n"
        "  选项表:\n"
        "    -p <perm>      启用权限检测, 信件将只发送给具有 <perm> 权限的用户, 默认不启用, \n"
        "                    <perm> 以字符 `&` 起始时表示各个权限之间是逻辑与关系, 即所有描述到的权限都是重要的, \n"
        "                    默认情况下各个权限之前是逻辑或关系\n"
        "    -u <user>      启用用户检测, 信件将只发送给用户名为 <user> 的用户, 默认不启用, 不能与 -l 选项同时使用\n"
        "    -l <list>      启用用户检测, 信件将只发送给文件 <list> 指定的用户, 默认不启用, 不能与 -u 选项同时使用\n"
        "    -m <mode>      设定发送模式, 有效设定值为 `copy` 和 `link`, `copy` 表示以复制模式发送信件, \n"
        "                   `link` 表示以链接模式发送信件, 默认模式为 `link`\n"
        "    -s <name>      设定信件的发送者为 <name>, 默认为 `SYSOP`, 发送者的最大有效长度为 [%d] 字节\n"
        "    -v             显示进度信息, 仅当未启用用户检测并且目标用户多于 [%d] 时有效\n"
        "    -h             显示文档信息\n",
        MT_LEN,BBSHOME,(IDLEN+1),MCV_THRESHOLD);
    return 0;
}

/* 处理权限检测 */
static inline int m_check_perm(const struct userec *user){
    return (!(flag&MC_PERM)?1:(!(perm&MCP_AND_FLAG)?(user->userlevel&perm):(((user->userlevel|MCP_AND_FLAG)&perm)==perm)));
}

/* 处理发送模式 */
static inline int m_check_mode(void){
    return (!(flag&MC_MODE)?BBSPOST_LINK:BBSPOST_COPY);
}

/* 处理发送者 */
static inline int m_set_name(void){
    if(!(flag&MC_NAME))
        snprintf(name,IDLEN+2,"%s","SYSOP");
    return 0;
}

/* 处理记数 */
static inline int m_count(struct userec *user,void *arg){
    if(m_check_perm(user))
        total++;
    return 0;
}

/* 处理进度 */
static inline int m_verb(void){
    if((flag&MC_VERB)&&!(total<MCV_THRESHOLD)){
#define M_VERB_OUT(t,n) do{fprintf(stdout,(t),(n));fflush(stdout);return 0;}while(0)
        if(!count)
            M_VERB_OUT("%d%%",0);
        if(count==(total-1))
            M_VERB_OUT("...%d%%\n",100);
        if(!(count%(total/MCV_SEPS)))
            M_VERB_OUT("...%d%%",((100*count)/total));
#undef M_VERB_OUT
    }
    return 0;
}

/* 处理信件发送 */
static inline int m_send(struct userec *user,void *arg){
    if(m_check_perm(user)){
        mail_file(name,path,user->userid,title,m_check_mode(),NULL);
        m_verb();
        count++;
    }
    return 0;
}

int main(int argc,char **argv){
#define ME_QUIT(s) do{fprintf(stderr,"error: %s\n",s);exit(__LINE__);}while(0)
    FILE *fp;
    struct userec *p_user;
    struct stat st;
    char cwd[MP_LEN],buf[MB_LEN],*p,*q;
    int ret;
    if(!getcwd(cwd,MP_LEN))
        ME_QUIT("get current working directory ...");
    opterr=0;
    while((ret=getopt(argc,argv,"p:u:l:m:s:vh"))!=-1){
        switch(ret){
            case 'p':                           /* 权限检测 */
                flag|=MC_PERM;
                if(*optarg=='&')                /* 使用逻辑与方式进行权限检测 */
                    perm|=MCP_AND_FLAG;
                for(p=optarg;*p;p++){
                    if(!(q=strchr(perm_str,*p)))
                        continue;
                    perm|=(1<<(q-perm_str));
                }
                break;
            case 'u':                           /* 用户检测 */
                flag|=MC_USER;
                snprintf(user,IDLEN+1,"%s",optarg);
                break;
            case 'l':                           /* 列表检测 */
                flag|=MC_LIST;
                if(*optarg=='/')
                    snprintf(list,MP_LEN,"%s",optarg);
                else
                    snprintf(list,MP_LEN,"%s/%s",cwd,optarg);
                break;
            case 'm':                           /* 设定发送模式 */
                if(!strcasecmp(optarg,"copy"))
                    flag|=MC_MODE;
                break;
            case 's':                           /* 设定发送者 */
                flag|=MC_NAME;
                snprintf(name,IDLEN+2,"%s",optarg);
                break;
            case 'v':
                flag|=MC_VERB;
                break;
            case 'h':
                m_usage();
                return 0;
            default:
                m_usage();
                ME_QUIT("get options ...");
        }
    }
    if((flag&MC_USER)&&(flag&MC_LIST)){         /* 冲突检测 */
        m_usage();
        ME_QUIT("options `-u` and `-l` conflict ...");
    }
    if((argc-optind)!=2){
        m_usage();
        ME_QUIT("get parameters ...");
    }
    snprintf(title,MT_LEN,"%s",argv[optind++]);
    if(argv[optind][0]=='+')
        snprintf(path,MP_LEN,"%s/%s",BBSHOME,&argv[optind][1]);
    else if(argv[optind][0]=='/')
        snprintf(path,MP_LEN,"%s",argv[optind]);
    else
        snprintf(path,MP_LEN,"%s/%s",cwd,argv[optind]);
    if(stat(path,&st)||!(st.st_mode&(S_IFREG|S_IFLNK)))
        ME_QUIT("get vaild file to mail ...");
    if(chdir(BBSHOME)==-1)
        ME_QUIT("change directory to BBSHOME ...");
    resolve_ucache();
    resolve_boards();
    resolve_utmp();
    m_set_name();
    if(flag&MC_USER){
        if(!getuser(user,&p_user))
            ME_QUIT("locate specified user ...");
        m_send(p_user,NULL);
    }
    else if(flag&MC_LIST){
        if(!(fp=fopen(list,"r")))
            ME_QUIT("open list file ...");
        while(fgets(buf,MB_LEN,fp))
            for(p=strtok(buf,MFR_DELIM);p;p=strtok(NULL,MFR_DELIM))
                if(getuser(p,&p_user))
                    m_send(p_user,NULL);
        fclose(fp);
    }
    else{
        if(flag&MC_VERB)
            apply_users(m_count,NULL);
        apply_users(m_send,NULL);
    }
    return 0;
#undef ME_QUIT
}

