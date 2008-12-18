/* etnlegend, 2006.04.11, 信件有效性检测及修复工具 */

/*
 * 命令行:          process_invalid_mail -a [ -t ]
                    process_invalid_mail -u <userid> [ -t ]
                    process_invalid_mail -h
 * 选项表:
 *   -a             对全部用户的信件进行处理, 不能与 -u 选项同时使用
 *   -h             显式文档信息
 *   -t             测试并输出处理信息, 但不进行真实操作
 *   -u <userid>    仅对用户 <userid> 的信件进行处理, 不能与 -a 选项同时使用
 *
*/

#include "bbs.h"
#define PIMP_LEN _POSIX_PATH_MAX
#define PIMF_MOD 0644
#define PIMC_ALL 0x01                           /* 对全部用户的信件进行处理 */
#define PIMC_SIG 0x02                           /* 对指定用户的信件进行处理 */
#define PIMC_DRY 0x04                           /* 测试并输出处理信息 */

static char dirs[MAILBOARDNUM+3][16];           /* 所有可能有效的 DIR 名称*/
static char path[PIMP_LEN];                     /* 当前用户信箱路径缓冲区 */
static char *p;                                 /* 当前路径缓冲区结束指针 */
static struct fileheader *dir;                  /* 当前 DIR 映像指针 */
static int count;                               /* 当前 DIR 包含信件数量 */
static int dir_handle;                          /* 当前 DIR 映像的唯一描述 */
static unsigned int flag;                       /* 选项标签信息 */

static int pim_usage(void)                      /* 打印文档信息 */
{
    fprintf(stdout,"%s",
            "\n"
            "  命令行:          process_invalid_mail -a [ -t ]\n"
            "                   process_invalid_mail -u <userid> [ -t ]\n"
            "                   process_invalid_mail -h\n"
            "  选项表:\n"
            "    -a             对全部用户的信件进行处理, 不能与 -u 选项同时使用\n"
            "    -h             显式文档信息\n"
            "    -t             测试并输出处理信息, 但不进行真实操作\n"
            "    -u <userid>    仅对用户 <userid> 的信件进行处理, 不能与 -a 选项同时使用\n"
            "\n");
    return 0;
}

static int gen_dirs(void)                       /* 生成所有可能有效 DIR 名称 */
{
    int index,suffix;
    index=0;
    sprintf(dirs[index++],"%s",".DIR");
    sprintf(dirs[index++],"%s",".SENT");
    sprintf(dirs[index++],"%s",".DELETED");
    for (suffix=1;!(suffix>MAILBOARDNUM);suffix++)
        sprintf(dirs[index++],".MAILBOX%d",suffix);
    return 0;
}

static int map_dir(int index)                   /* 处理 DIR 映像操作 */
{
    static const struct flock lck_set={.l_type=F_WRLCK,.l_whence=SEEK_SET,.l_start=0,.l_len=0,.l_pid=0};
    static const struct flock lck_clr={.l_type=F_UNLCK,.l_whence=SEEK_SET,.l_start=0,.l_len=0,.l_pid=0};
    static struct stat st;
    static int dir_mapped,fd;
    static void *p_map;
    if (dir_mapped) {
        munmap(p_map,st.st_size);
        ftruncate(fd,(count*sizeof(struct fileheader)));
        fcntl(fd,F_SETLKW,&lck_clr);
        close(fd);
        dir_mapped=0;
    }
    if (index<0||index>(MAILBOARDNUM+2))
        return 1;
    sprintf(p,"%s",dirs[index]);
    if (stat(path,&st)==-1||!S_ISREG(st.st_mode))
        return 2;
    if ((fd=open(path,O_RDWR
#ifdef O_NOATIME
                 |O_NOATIME                              /* 可能会有点优化作用吧 */
#endif /* O_NOATIME */
                 ,PIMF_MOD))==-1)
        return 3;
    if (fcntl(fd,F_SETLKW,&lck_set)==-1) {
        close(fd);
        return 4;
    }
    if ((p_map=mmap(NULL,st.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0))==MAP_FAILED) {
        fcntl(fd,F_SETLKW,&lck_clr);
        close(fd);
        return 5;
    }
    dir=(struct fileheader*)p_map;
    count=(st.st_size/sizeof(struct fileheader));
    dir_handle++;
    dir_mapped=1;
    return 0;
}

static int map_dir_dry(int index)
{
    static const struct flock lck_set={.l_type=F_WRLCK,.l_whence=SEEK_SET,.l_start=0,.l_len=0,.l_pid=0};
    static const struct flock lck_clr={.l_type=F_UNLCK,.l_whence=SEEK_SET,.l_start=0,.l_len=0,.l_pid=0};
    static struct stat st;
    static int dir_mapped,fd;
    static void *p_map;
    if (dir_mapped) {
        munmap(p_map,st.st_size);
        fcntl(fd,F_SETLKW,&lck_clr);
        close(fd);
        dir_mapped=0;
    }
    if (index<0||index>(MAILBOARDNUM+2))
        return 1;
    sprintf(p,"%s",dirs[index]);
    if (stat(path,&st)==-1||!S_ISREG(st.st_mode))
        return 2;
    if ((fd=open(path,O_RDONLY
#ifdef O_NOATIME
                 |O_NOATIME                              /* 可能会有点优化作用吧 */
#endif /* O_NOATIME */
                 ,PIMF_MOD))==-1)
        return 3;
    if (fcntl(fd,F_SETLKW,&lck_set)==-1) {
        close(fd);
        return 4;
    }
    if ((p_map=mmap(NULL,st.st_size,PROT_READ,MAP_SHARED,fd,0))==MAP_FAILED) {
        fcntl(fd,F_SETLKW,&lck_clr);
        close(fd);
        return 5;
    }
    dir=(struct fileheader*)p_map;
    count=(st.st_size/sizeof(struct fileheader));
    dir_handle++;
    dir_mapped=1;
    return 0;
}

static int process_dir(const char *userid,int dir_index)
{
    static int dir_processed_handle;
    struct stat st;
    int index;
    if (!(dir_handle>dir_processed_handle))
        return 1;
    index=0;
    while (index<count) {
        sprintf(p,"%s",dir[index].filename);
        if (stat(path,&st)==-1||!S_ISREG(st.st_mode)) {
            if (!(flag&PIMC_DRY)) {
                unlink(path);
                memmove(&dir[index],&dir[index+1],((count-(index+1))*sizeof(struct fileheader)));
                count--;
                continue;
            } else
                printf(" %-12.12s : %15.15s %10.10s %d\n",userid,dir[index].filename,dirs[dir_index],index);
        }
        index++;
    }
    dir_processed_handle=dir_handle;
    return 0;
}

static int process_user(const char *userid)
{
    struct stat st;
    int index;
    setmailpath(path,userid);
    if (stat(path,&st)==-1||!S_ISDIR(st.st_mode))
        return 1;
    p=&path[strlen(path)];
    (*p++)='/';
    for (index=0;index<(MAILBOARDNUM+3);index++) {
        if (!(flag&PIMC_DRY)?map_dir(index):map_dir_dry(index))
            continue;
        process_dir(userid,index);
    }
    !(flag&PIMC_DRY)?map_dir(-1):map_dir_dry(-1);
    return 0;
}

static int pim_callback_func(struct userec *user,void *arg)
{
    if (!(user->userid[0])) {
        return 0;
    }
    return process_user(user->userid);
}

int main(int argc,char **argv)
{
#define PIM_QUIT(s) do{fprintf(stderr,"error: %s\n",s);exit(__LINE__);}while(0)
    struct userec *user;
    char userid[IDLEN+1];
    int ret;
    if (argc==1) {
        pim_usage();
        return 0;
    }
    opterr=0;
    while ((ret=getopt(argc,argv,"u:aht"))!=-1) {
        switch (ret) {
            case 'u':
                flag|=PIMC_SIG;
                snprintf(userid,IDLEN+1,"%s",optarg);
                break;
            case 'a':
                flag|=PIMC_ALL;
                break;
            case 't':
                flag|=PIMC_DRY;
                break;
            case 'h':
                pim_usage();
                return 0;
            default:
                pim_usage();
                PIM_QUIT("get options ...");
        }
    }
    if ((flag&PIMC_ALL)&&(flag&PIMC_SIG)) {
        pim_usage();
        PIM_QUIT("options -a and -u conflict ...");
    }
    if (!(flag&(PIMC_ALL|PIMC_SIG))) {
        pim_usage();
        PIM_QUIT("absence of both -a and -u options ...");
    }
    if (optind!=argc) {
        pim_usage();
        PIM_QUIT("get parameters ...");
    }
    if (chdir(BBSHOME)==-1)
        PIM_QUIT("change directory to BBSHOME ...");
    resolve_ucache();
    gen_dirs();
    if (flag&PIMC_SIG) {
        if (!getuser(userid,&user))
            PIM_QUIT("get specified user ...");
        process_user(user->userid);
    } else
        apply_users(pim_callback_func,NULL);
    return 0;
#undef PIM_QUIT
}

