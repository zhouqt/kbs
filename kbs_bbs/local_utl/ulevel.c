/* etnlegend, 2006.04.17, 用户权限管理工具 */

/*
 * 命令形式:        ulevel [ 用户选项 ] [ 条件选项 ] [ 操作选项 ] [ 帮助选项 ]
 *
 * 用户选项:
 *   -a             操作对象为所有用户, 该选项与 -u 选项互斥, 默认启用
 *   -u <user>      操作对象为用户名为 <user> 的用户, 该选项与 -a 选项互斥, 默认不启用
 *
 * 条件选项:
 *   -p <perm>      启用权限检测, 操作对象为权限条件 <perm> 指定的用户, 默认不启用
 *                  <perm> 是一组权限符号的集合, 以字符 `&` 起始时表示各个权限之间是逻辑与关系,
 *                  即所有描述到的权限都是重要的, 默认情况下各个权限之前是逻辑或关系
 *   -t <tnum>      启用身份检测, 操作对象为身份条件 <tnum> 指定的用户, 默认不启用
 *                  <tnum> 是 `0 ~ 255` 之间的整数, 表示身份序号
 *
 * 操作选项:
 *   -A <perm>      为当前操作对象增加权限 <perm>, 该选项与 -S 选项互斥, 默认不启用
 *   -D <perm>      为当前操作对象移除权限 <perm>, 该选项与 -S 选项互斥, 默认不启用
 *   -S <perm>      将当前操作对象权限设置为 <perm>, 该选项与 -A 选项和 -D 选项互斥, 默认不启用
 *   -T <tnum>      将当前操作对象身份设置为 <tnum>, 默认不启用
 *
 * 帮助选项:
 *   -h             显示文档信息
 *
*/

#include "bbs.h"

#define UB_LEN      128                             /* 缓冲区长度 */
#define UU_USR      0x0001                          /* 用户选项 -u 设置标签 */
#define UC_PRM      0x0002                          /* 条件选项 -p 设置标签 */
#define UC_TTL      0x0004                          /* 条件选项 -t 设置标签 */
#define UO_ADD      0x0008                          /* 操作选项 -A 设置标签 */
#define UO_DEL      0x0010                          /* 操作选项 -D 设置标签 */
#define UO_SET      0x0020                          /* 操作选项 -S 设置标签 */
#define UO_TTL      0x0040                          /* 操作选项 -T 设置标签 */
#define UO_MSK      (UO_ADD|UO_DEL|UO_SET|UO_TTL)   /* 操作选项掩码 */
#define UE_AND      0x0080                          /* 条件选项 -p 设置时的逻辑标签 */
#define UE_RVS      0x0100                          /* 操作选项 -A 和 -D 同时设置时的顺序标签 */
#define UE_BTH      0x0200                          /* 用户选项 -u 和 -a 同时设置是的检测标签 */

static const char* const perms=XPERMSTR;            /* 权限符号列表 */
static unsigned int flag;                           /* 选项标签 */
static char user[IDLEN+1];                          /* 设定 -u 选项时的用户名参数 */
static unsigned int perm;                           /* 设定 -p 选项时的权限参数 */
static unsigned int tnum;                           /* 设定 -t 选项时的身份参数 */
static unsigned int permA;                          /* 设定 -A 选项时的权限参数 */
static unsigned int permD;                          /* 设定 -D 选项时的权限参数 */
static unsigned int permS;                          /* 设定 -S 选项时的权限参数 */
static unsigned int tnumT;                          /* 设定 -T 选项时的身份参数 */

/* 函数原型 */
#ifndef __nonnull
#define __nonnull(param)
#endif /* __nonnull */
static inline int u_usage(void);
static inline unsigned int u_gen_perm(const char*) __nonnull((1));
static inline unsigned int u_gen_title(const char*) __nonnull((1));
static inline int u_check_perm(const struct userec*) __nonnull((1));
static inline int u_check_title(const struct userec*) __nonnull((1));
static inline int u_add_perm(struct userec*) __nonnull((1));
static inline int u_del_perm(struct userec*) __nonnull((1));
static inline int u_set_perm(struct userec*) __nonnull((1));
static inline int u_set_title(struct userec*) __nonnull((1));
static inline int u_process_condition(const struct userec*) __nonnull((1));
static inline int u_process_operation(struct userec*) __nonnull((1));
static inline const char* u_gen_perms(unsigned int perm);
static inline int u_show_current(const struct userec*) __nonnull((1));
static inline int u_show_change(const struct userec*,unsigned int,unsigned int) __nonnull((1));
static inline int u_process(struct userec*,void*) __nonnull((1));

/* 处理 -h 选项 */
static inline int u_usage(void){
    fprintf(stdout,"\n%s\n%s\n%s\n%s\n%s\n",
        "  命令形式:        ulevel [ 用户选项 ] [ 条件选项 ] [ 操作选项 ] [ 帮助选项 ]\n",
        "  用户选项:\n"
        "    -a             操作对象为所有用户, 该选项与 -u 选项互斥, 默认启用\n"
        "    -u <user>      操作对象为用户名为 <user> 的用户, 该选项与 -a 选项互斥, 默认不启用\n",
        "  条件选项:\n"
        "    -p <perm>      启用权限检测, 操作对象为权限条件 <perm> 指定的用户, 默认不启用\n"
        "                   <perm> 是一组权限符号的集合, 以字符 `&` 起始时表示各个权限之间是逻辑与关系,\n"
        "                   即所有描述到的权限都是重要的, 默认情况下各个权限之前是逻辑或关系\n"
        "    -t <tnum>      启用身份检测, 操作对象为身份条件 <tnum> 指定的用户, 默认不启用\n"
        "                   <tnum> 是 `0 ~ 255` 之间的整数, 表示身份序号\n",
        "  操作选项:\n"
        "    -A <perm>      为当前操作对象增加权限 <perm>, 该选项与 -S 选项互斥, 默认不启用\n"
        "    -D <perm>      为当前操作对象移除权限 <perm>, 该选项与 -S 选项互斥, 默认不启用\n"
        "    -S <perm>      将当前操作对象权限设置为 <perm>, 该选项与 -A 选项和 -D 选项互斥, 默认不启用\n"
        "    -T <tnum>      将当前操作对象身份设置为 <tnum>, 默认不启用\n",
        "  帮助选项:\n"
        "    -h             显示文档信息\n");
    return 0;
}

/* 处理权限参数 */
static inline unsigned int u_gen_perm(const char *s){
    const char *p,*r;
    unsigned int ret;
    for(p=s,ret=0;*p;p++){
        if(!(r=strchr(perms,(*p))))
            continue;
        ret|=(1<<(r-perms));
    }
    return ret;
}

/* 处理身份参数 */
static inline unsigned int u_gen_title(const char *s){
    const char *p;
    unsigned int ret;
    for(p=s,ret=0;*p;p++)
        if(!isdigit(*p))
            return 0x80000100;
    return (!((ret=atoi(s))&~0xFF)?(ret):(0x80000200));
}

/* 处理 -p 选项 */
static inline int u_check_perm(const struct userec *user){
    return (!(flag&UC_PRM)?(1):(!(flag&UE_AND)?(user->userlevel&perm):((user->userlevel&perm)==perm)));
}

/* 处理 -t 选项 */
static inline int u_check_title(const struct userec *user){
    return (!(flag&UC_TTL)?(1):(user->title==tnum));
}

/* 处理 -A 选项 */
static inline int u_add_perm(struct userec *user){
    if(flag&UO_ADD)
        user->userlevel|=permA;
    return 0;
}

/* 处理 -D 选项 */
static inline int u_del_perm(struct userec *user){
    if(flag&UO_DEL)
        user->userlevel&=~permD;
    return 0;
}

/* 处理 -S 选项 */
static inline int u_set_perm(struct userec *user){
    if(flag&UO_SET)
        user->userlevel=permS;
    return 0;
}

/* 处理 -T 选项 */
static inline int u_set_title(struct userec *user){
    if(flag&UO_TTL)
        user->title=tnumT;
    return 0;
}

/* 处理条件 */
static inline int u_process_condition(const struct userec *user){
    return (u_check_perm(user)&&u_check_title(user));
}

/* 处理操作 */
static inline int u_process_operation(struct userec *user){
    if(!(flag&UE_RVS)){
        u_add_perm(user);
        u_del_perm(user);
    }
    else{
        u_del_perm(user);
        u_add_perm(user);
    }
    u_set_perm(user);
    u_set_title(user);
    return 0;
}

/* 处理权限字符串 */
static inline const char* u_gen_perms(unsigned int perm){
    static char buf[NUMPERMS+1];
    int i;
    for(i=0;i<NUMPERMS;i++)
        buf[i]=(!(perm&(1<<i))?('-'):perms[i]);
    buf[i]=0;
    return ((const char*)buf);
}

/* 处理输出当前 */
static inline int u_show_current(const struct userec *user){
    fprintf(stdout,"%-12.12s  <%03u>  %s\n",user->userid,user->title,u_gen_perms(user->userlevel));
    return 0;
}

/* 处理输出更改 */
static inline int u_show_change(const struct userec *user,unsigned int save_perm,unsigned int save_tnum){
    fprintf(stdout,"%-12.12s  <%03u>  %s  ",user->userid,save_tnum,u_gen_perms(save_perm));
    fprintf(stdout,"=>  <%03u>  %s\n",user->title,u_gen_perms(user->userlevel));
    return 0;
}

/* 处理单一用户 */
static inline int u_process(struct userec *user,void *arg){
    unsigned int save_perm;
    unsigned int save_tnum;
    if(!(user->userid[0])||!u_process_condition(user))
        return 0;
    if(!(flag&UO_MSK))
        u_show_current(user);
    else{
        save_perm=user->userlevel;
        save_tnum=user->title;
        u_process_operation(user);
        u_show_change(user,save_perm,save_tnum);
    }
    return 1;
}

/* 处理主交互 */
int main(int argc,char **argv){
#define UE_QUIT(s) do{u_usage();fprintf(stderr,"error: %s\n",s);exit(__LINE__);}while(0)
    struct userec *p_user;
    int ret;
    opterr=0;
    while((ret=getopt(argc,argv,"au:p:t:A:D:S:T:h"))!=-1){
        switch(ret){
            case 'a':
                if(flag&(UE_BTH|UU_USR))
                    UE_QUIT("options conflict or overlap ...");
                flag|=UE_BTH;
                break;
            case 'u':
                if(flag&(UE_BTH|UU_USR))
                    UE_QUIT("options conflict or overlap ...");
                flag|=UU_USR;
                snprintf(user,IDLEN+1,"%s",optarg);
                break;
            case 'p':
                if(flag&UC_PRM)
                    UE_QUIT("options conflict or overlap ...");
                flag|=UC_PRM;
                (*optarg=='&')?(flag|=UE_AND):(flag&=~UE_AND);
                perm=u_gen_perm(optarg);
                break;
            case 't':
                if(flag&UC_TTL)
                    UE_QUIT("options conflict or overlap ...");
                flag|=UC_TTL;
                tnum=u_gen_title(optarg);
                if(tnum&~0xFF)
                    UE_QUIT("invalid title number ...");
                break;
            case 'A':
                if(flag&UO_ADD)
                    UE_QUIT("options conflict or overlap ...");
                flag|=UO_ADD;
                permA=u_gen_perm(optarg);
                (flag&UO_DEL)?(flag|=UE_RVS):(flag&=~UE_RVS);
                break;
            case 'D':
                if(flag&UO_DEL)
                    UE_QUIT("options conflict or overlap ...");
                flag|=UO_DEL;
                permD=u_gen_perm(optarg);
                (flag&UO_ADD)?(flag&=~UE_RVS):(flag|=UE_RVS);
                break;
            case 'S':
                if(flag&(UO_ADD|UO_DEL|UO_SET))
                    UE_QUIT("options conflict or overlap ...");
                flag|=UO_SET;
                permS=u_gen_perm(optarg);
                break;
            case 'T':
                if(flag&UO_TTL)
                    UE_QUIT("options conflict or overlap ...");
                flag|=UO_TTL;
                tnumT=u_gen_title(optarg);
                if(tnumT&~0xFF)
                    UE_QUIT("invalid title number ...");
                break;
            case 'h':
                u_usage();
                return 0;;
            default:
                UE_QUIT("unknown usage of options ...");
        }
    }
    if(optind!=argc)
        UE_QUIT("useless parameters ...");
    if(chdir(BBSHOME)==-1)
        UE_QUIT("change directory to BBSHOME ...");
    resolve_ucache();
    if(flag&UU_USR){
        if(!getuser(user,&p_user))
            UE_QUIT("locate specified user ...");
        u_process(p_user,NULL);
    }
    else
        apply_users(u_process,NULL);
    return 0;
#undef UE_QUIT
}

