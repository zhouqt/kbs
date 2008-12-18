/* etnlegend, 2006.09.14, 全站文章全文搜索系统 ... */

/*
 * 命令行:          ultimate_search [选项] <模式串>
 *   <模式串>       需要全文搜索的字符串
 *
 * [选项]           [说明]                              [冲突]
 *
 *   {时间范围}     无限制                              默认
 *     -r <n>       最近 <n> 天                         不可与 -f 或 -t 同时使用
 *     -f <time>    从 <time> 所指定的时间开始          不可与 -r 同时使用
 *     -t <time>    至 <time> 所指定的时间结束          不可与 -r 同时使用
 *
 *        <time> 的格式为 YYYYMMDDHHMMSS (年月日时分秒)
 *
 *   {版面范围}     全站公开版面                        默认
 *     -a           全部版面                            不可与 -b 或 -u 同时使用
 *     -b <board>   仅 <board> 版面                     不可与 -a 或 -u 同时使用
 *     -u <user>    仅 <user> 用户可读的版面            不可与 -a 或 -b 同时使用
 *
 *   {用户范围}     全站用户                            默认
 *     -p <poster>  仅 <poster> 发表的文章              无
 *
 *   {位置范围}     版面当前文章                        默认
 *     -d           包含回收站文章                      无
 *     -j           包含自删区文章                      无
 *     -e           不包含版面当前文章                  必须与 -d 或 -j 同时使用
 *
 *   {搜索参数}     搜索全部匹配位置                    默认
 *                  进行中文断字验证                    默认
 *                  大小写敏感                          默认
 *     -s           仅搜索每篇文章的第一处匹配位置      无
 *     -n           不进行中文断字验证                  无
 *     -i           大小写不敏感                        无
 *
 *   {输出选项}     输出到当前目录下 res_<mark>.us 文件 默认
 *                  进行详悉进度输出                    默认
 *     -o <file>    输出到文件 <file> 中                无
 *     -q           安静模式                            无
 *
 *        <mark> 为运行当前命令的时间戳 ( UNIX 格式)
 *
 *   {帮助选项}     参数错误时输出帮助信息              默认
 *     -h           输出帮助信息                        无
 *
*/

#include "bbs.h"

#define PARAM_R                 0x0001
#define PARAM_F                 0x0002
#define PARAM_T                 0x0004
#define PARAM_A                 0x0008
#define PARAM_B                 0x0010
#define PARAM_U                 0x0020
#define PARAM_P                 0x0040
#define PARAM_D                 0x0080
#define PARAM_J                 0x0100
#define PARAM_E                 0x0200
#define PARAM_S                 0x0400
#define PARAM_N                 0x0800
#define PARAM_I                 0x1000
#define PARAM_O                 0x2000
#define PARAM_Q                 0x4000
#define LENGTH                  1024
#define BOUND                   _POSIX_PATH_MAX
#define ISSET(param)            (flag&(param))

typedef int (*APPLY_BIDS_FUNC)(
    int(*)(const struct boardheader*,int,void*),
    void*
);
typedef int (*APPLY_RECORD_FUNC)(
    const char*,
    int(*)(const struct fileheader*,int,const struct boardheader*),
    int,
    const struct boardheader*,
    int,
    bool
);

static const APPLY_BIDS_FUNC    APPLY_BIDS          = (APPLY_BIDS_FUNC)   apply_bids;
static const APPLY_RECORD_FUNC  APPLY_RECORD        = (APPLY_RECORD_FUNC) apply_record;

static FILE                     *out;
static const struct userec      *user;
static const char               *P;
static const char               *L;
static const char               *mode;
static char                     post[OWNER_LEN];
static int                      flag;
static int                      current;
static int                      count;
static int                      number;
static time_t                   from;
static time_t                   to;
static time_t                   mark;
static size_t                   size;

static inline int usage(void)
{
    fprintf(stderr,"\n%s\n",
            "命令行:          ultimate_search [选项] <模式串>\n"
            "  <模式串>       需要全文搜索的字符串\n"
            "\n"
            "[选项]           [说明]                              [冲突]\n"
            "\n"
            "  {时间范围}     无限制                              默认\n"
            "    -r <n>       最近 <n> 天                         不可与 -f 或 -t 同时使用\n"
            "    -f <time>    从 <time> 所指定的时间开始          不可与 -r 同时使用\n"
            "    -t <time>    至 <time> 所指定的时间结束          不可与 -r 同时使用\n"
            "\n"
            "       <time> 的格式为 YYYYMMDDHHMMSS (年月日时分秒)\n"
            "\n"
            "  {版面范围}     全站公开版面                        默认\n"
            "    -a           全部版面                            不可与 -b 或 -u 同时使用\n"
            "    -b <board>   仅 <board> 版面                     不可与 -a 或 -u 同时使用\n"
            "    -u <user>    仅 <user> 用户可读的版面            不可与 -a 或 -b 同时使用\n"
            "\n"
            "  {用户范围}     全站用户                            默认\n"
            "    -p <poster>  仅 <poster> 发表的文章              无\n"
            "\n"
            "  {位置范围}     版面当前文章                        默认\n"
            "    -d           包含回收站文章                      无\n"
            "    -j           包含自删区文章                      无\n"
            "    -e           不包含版面当前文章                  必须与 -d 或 -j 同时使用\n"
            "\n"
            "  {搜索参数}     搜索全部匹配位置                    默认\n"
            "                 进行中文断字验证                    默认\n"
            "                 大小写敏感                          默认\n"
            "    -s           仅搜索每篇文章的第一处匹配位置      无\n"
            "    -n           不进行中文断字验证                  无\n"
            "    -i           大小写不敏感                        无\n"
            "\n"
            "  {输出选项}     输出到当前目录下 res_<mark>.us 文件 默认\n"
            "                 进行详悉进度输出                    默认\n"
            "    -o <file>    输出到文件 <file> 中                无\n"
            "    -q           安静模式                            无\n"
            "\n"
            "       <mark> 为运行当前命令的时间戳 ( UNIX 格式)\n"
            "\n"
            "  {帮助选项}     参数错误时输出帮助信息              默认\n"
            "    -h           输出帮助信息                        无\n"
           );
    return 0;
}

static inline int set_pattern(const char *s)
{
#define ALPHA(c)    (!((c)<'A')&&!((c)>'Z'))
    static char pattern[LENGTH];
    static int initialized,i;
    if (!initialized) {
        if (!ISSET(PARAM_I)) {
            snprintf(pattern,LENGTH,"%s",s);
            size=strlen(pattern);
        } else {
            for (size=0;s[size];size++) {
                pattern[size]=toupper(s[size]);
                if (ALPHA(pattern[size]))
                    i++;
            }
            if (!i)
                flag&=(~PARAM_I);
        }
        P=pattern;
        initialized=1;
    }
    return 0;
#undef ALPHA
}

static inline int set_link(const char *s)
{
    static char link[LENGTH];
    static int initialized,i,j;
    if (!initialized) {
        for (i=0,j=1;s[j];j++) {
            while (i>0&&s[i]!=s[j])
                i=link[i-1];
            if (s[i]==s[j])
                i++;
            link[j]=i;
        }
        L=link;
        initialized=1;
    }
    return 0;
}

static inline int process_article(const struct fileheader *f,int n,const struct boardheader *b)
{
    static const struct flock lck_set={.l_type=F_RDLCK,.l_whence=SEEK_SET,.l_start=0,.l_len=0,.l_pid=0};
    static const struct flock lck_clr={.l_type=F_UNLCK,.l_whence=SEEK_SET,.l_start=0,.l_len=0,.l_pid=0};
    static struct stat st;
    static struct tm *p;
    static char name[BOUND];
    static int fd,i,j,k,l;
    static time_t timestamp;
    static const char *S,*M,*N;
    static void *vp;
    do {
        if ((timestamp=get_posttime(f))<from||timestamp>to)
            break;
        if (ISSET(PARAM_P)&&strcmp(f->owner,post))
            break;
        setbfile(name,b->filename,f->filename);
        if (stat(name,&st)==-1||!S_ISREG(st.st_mode)||st.st_size<size)
            break;
        if ((fd=open(name,O_RDONLY
#ifdef O_NOATIME
                     |O_NOATIME
#endif /* O_NOATIME */
                     ,0644))==-1)
            break;
        if (fcntl(fd,F_SETLKW,&lck_set)==-1) {
            close(fd);
            break;
        }
        vp=mmap(NULL,st.st_size,PROT_READ,MAP_PRIVATE,fd,0);
        fcntl(fd,F_SETLKW,&lck_clr);
        close(fd);
        if ((S=(const char*)vp)==MAP_FAILED)
            break;
        for (p=NULL,j=0,i=0;S[i]&&i<st.st_size;i++) {
#define EQUAL(cp,cs)    (((cp)==(cs))||(ISSET(PARAM_I)&&((cp)==toupper(cs))))
            while (j>0&&!EQUAL(P[j],S[i]))
                j=L[j-1];
            if (EQUAL(P[j],S[i]))
                j++;
            if (!P[j]) {
                M=&S[l=((i-j)+1)];
                if (!ISSET(PARAM_N)) {
                    for (k=0,N=M;!(N<S);N--)
                        if ((*N)&0x80)
                            k++;
                    if (!(k&0x01))
                        continue;
                }
                if (!p&&!(p=localtime(&timestamp)))
                    continue;
                count++;
                fprintf(out,"%6d %-20.20s %4d %4s %04d%02d%02d%02d%02d%02d %-17.17s %6d %-13.13s %s\n",
                        n,b->filename,current,mode,(p->tm_year+1900),(p->tm_mon+1),(p->tm_mday),
                        (p->tm_hour),(p->tm_min),(p->tm_sec),f->filename,l,f->owner,f->title);
                if (ISSET(PARAM_S))
                    break;
                j=L[j-1];
            }
#undef EQUAL
        }
        munmap(vp,st.st_size);
        number++;
    } while (0);
    return 0;
}

static inline int process_board(const struct boardheader *b,int n,void *v)
{
    static char name[BOUND];
    do {
        if (ISSET(PARAM_A))
            break;
        if (ISSET(PARAM_U)) {
            if (!check_read_perm(user,b))
                return -1;
            break;
        }
        if (ISSET(PARAM_B))
            break;
        if (!public_board(b))
            return -2;
    } while (0);
    current=n;
    if (!ISSET(PARAM_Q))
        fprintf(stdout,"正在处理版面 %-29.29s ... ",b->filename);
    if (!ISSET(PARAM_E)) {
        mode="版面";
        setbdir(DIR_MODE_NORMAL,name,b->filename);
        APPLY_RECORD(name,process_article,sizeof(struct fileheader),b,0,true);
    }
    if (ISSET(PARAM_D)) {
        mode="回收";
        setbdir(DIR_MODE_DELETED,name,b->filename);
        APPLY_RECORD(name,process_article,sizeof(struct fileheader),b,0,true);
    }
    if (ISSET(PARAM_J)) {
        mode="自删";
        setbdir(DIR_MODE_JUNK,name,b->filename);
        APPLY_RECORD(name,process_article,sizeof(struct fileheader),b,0,true);
    }
    if (!ISSET(PARAM_Q))
        fprintf(stdout,"%s\n","处理完成!");
    return 0;
}

int main(int argc,char **argv)
{
#define EXIT(msg)  do{fprintf(stderr,"%s\n",(msg));if(out)fclose(out);exit(__LINE__);}while(0)
    const struct boardheader *board;
    char name[BOUND],path[BOUND];
    const char *desc;
    int ret;
    double cost;
    if (!getcwd(path,BOUND))
        EXIT("获取当前工作目录时发生错误");
    if (chdir(BBSHOME)==-1)
        EXIT("切换工作目录时发生错误...");
    if ((mark=time(NULL))==(time_t)(-1))
        EXIT("获取时间时发生错误...");
    resolve_ucache();
    resolve_boards();
    to=mark;
    opterr=0;
    while ((ret=getopt(argc,argv,"r:f:t:ab:u:p:djesnio:qh"))!=-1) {
        switch (ret) {
#define CHECK_CONFLICT(param)   do{if(ISSET(param))EXIT("给定的选项间存在冲突...");}while(0)
#define CHECK_DEPENDENCE(param) do{if(!ISSET(param))EXIT("给定的选项间缺少依赖...");}while(0)
#define CHECK_DUP(param)        do{if(ISSET(param))EXIT("给定的选项中存在重复...");}while(0)
#define SET(param)              do{CHECK_DUP(param);flag|=(param);}while(0)
            case 'r':
                CHECK_CONFLICT(PARAM_F|PARAM_T);
                SET(PARAM_R);
                do {
                    struct tm t,*p;
                    int n;
                    if (!isdigit(optarg[0]))
                        EXIT("选项 -r 的参数无法解析...");
                    n=atoi(optarg);
                    if (!(p=localtime(&mark)))
                        EXIT("解析时间时发生错误...");
                    memcpy(&t,p,sizeof(struct tm));
                    t.tm_hour=0;
                    t.tm_min=0;
                    t.tm_sec=0;
                    if ((from=mktime(&t))==(time_t)(-1))
                        EXIT("设定时间时发生错误...");
                } while (0);
                break;
#define PARSE2(p)   ((((p)[0]*10)+((p)[1]*1))-('0'*11))
#define PARSE4(p)   ((PARSE2(p)*100)+(PARSE2(&(p)[2])*1))
            case 'f':
                CHECK_CONFLICT(PARAM_R);
                SET(PARAM_F);
                do {
                    struct tm t;
                    int i;
                    for (i=0;optarg[i];i++)
                        if (!isdigit(optarg[i]))
                            break;
                    if (i!=14)
                        EXIT("选项 -f 的参数无法解析...");
                    memset(&t,0,sizeof(struct tm));
                    t.tm_year=(PARSE4(optarg)-1900);
                    t.tm_mon=(PARSE2(&optarg[4])-1);
                    t.tm_mday=PARSE2(&optarg[6]);
                    t.tm_hour=PARSE2(&optarg[8]);
                    t.tm_min=PARSE2(&optarg[10]);
                    t.tm_sec=PARSE2(&optarg[12]);
                    if ((from=mktime(&t))==(time_t)(-1))
                        EXIT("设定时间时发生错误...");
                } while (0);
                break;
            case 't':
                CHECK_CONFLICT(PARAM_R);
                SET(PARAM_T);
                do {
                    struct tm t;
                    int i;
                    for (i=0;optarg[i];i++)
                        if (!isdigit(optarg[i]))
                            break;
                    if (i!=14)
                        EXIT("选项 -t 的参数无法解析...");
                    memset(&t,0,sizeof(struct tm));
                    t.tm_year=(PARSE4(optarg)-1900);
                    t.tm_mon=(PARSE2(&optarg[4])-1);
                    t.tm_mday=PARSE2(&optarg[6]);
                    t.tm_hour=PARSE2(&optarg[8]);
                    t.tm_min=PARSE2(&optarg[10]);
                    t.tm_sec=PARSE2(&optarg[12]);
                    if ((from=mktime(&t))==(time_t)(-1))
                        EXIT("设定时间时发生错误...");
                } while (0);
                break;
#undef PARSE2
#undef PARSE4
            case 'a':
                CHECK_CONFLICT(PARAM_B|PARAM_U);
                SET(PARAM_A);
                break;
            case 'b':
                CHECK_CONFLICT(PARAM_A|PARAM_U);
                SET(PARAM_B);
                if (!(current=getbid(optarg,&board)))
                    EXIT("选项 -b 所指定的版面无法获取...");
                break;
            case 'u':
                CHECK_CONFLICT(PARAM_A|PARAM_B);
                SET(PARAM_U);
                do {
                    struct userec *u;
                    if (!getuser(optarg,&u))
                        EXIT("选项 -u 所指定的用户无法获取...");
                    user=u;
                } while (0);
                break;
            case 'p':
                SET(PARAM_P);
                snprintf(post,OWNER_LEN,"%s",optarg);
                break;
            case 'd':
                SET(PARAM_D);
                break;
            case 'j':
                SET(PARAM_J);
                break;
            case 'e':
                CHECK_DEPENDENCE(PARAM_D|PARAM_J);
                SET(PARAM_E);
                break;
            case 's':
                SET(PARAM_S);
                break;
            case 'n':
                SET(PARAM_N);
                break;
            case 'i':
                SET(PARAM_I);
                break;
            case 'o':
                SET(PARAM_O);
                if (optarg[0]!='/')
                    snprintf(name,BOUND,"%s/%s",path,optarg);
                else
                    snprintf(name,BOUND,"%s",optarg);
                break;
            case 'q':
                SET(PARAM_Q);
                break;
            case 'h':
                usage();
                return 0;
            default:
                usage();
                EXIT("不可识别的选项...");
                break;
#undef CHECK_CONFLICT
#undef CHECK_DEPENDENCE
#undef CHECK_DUP
#undef SET
        }
    }
    if (from>to) {
        usage();
        EXIT("当前时间设定不合法...");
    }
    if (!ISSET(PARAM_Q)&&setvbuf(stdout,NULL,_IONBF,BUFSIZ))
        EXIT("调整文件缓冲时发生错误...");
    if ((argc-optind)!=1) {
        usage();
        EXIT("不可识别的参数...");
    }
    set_pattern(argv[optind]);
    set_link(argv[optind]);
    if (!size)
        EXIT("模式串不能为空串...");
    if (!ISSET(PARAM_O))
        snprintf(name,BOUND,"%s/res_%lu.us",path,mark);
    if (!(out=fopen(name,"w")))
        EXIT("打开文件时发生错误...");
    fprintf(out,"%6s %-20.20s %4s %4s %-14.14s %-17.17s %6s %-13.13s %s\n",
            "文章号","版面名称"," BID","位置","发表时间","文件名","偏移量","作者","标题");
    if (!(P[0]&0x80))
        flag|=PARAM_N;
    if (ISSET(PARAM_B))
        process_board(board,current,NULL);
    else
        APPLY_BIDS(process_board,NULL);
    fclose(out);
    cost=difftime(time(NULL),mark);
    if (cost>86400) {
        cost/=86400;
        desc="天";
    } else if (cost>3600) {
        cost/=3600;
        desc="小时";
    } else if (cost>60) {
        cost/=60;
        desc="分钟";
    } else
        desc="秒";
    fprintf(stdout,"\n操作已完成! 共处理 %d 篇文章, 获得 %d 处匹配, 耗时 %.2lf %s!\n",
            number,count,cost,desc);
    return 0;
#undef EXIT
}

