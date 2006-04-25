#include "bbs.h"

#define MAX_EXPRESSION 50 /* too big? */
#define STRBUFLEN 200
#define STR_CONV_LEN 12

#define SUPER_FILTER_ERROR_GENERAL      1
#define SUPER_FILTER_ERROR_FN_SYNTAX    2
#define SUPER_FILTER_ERROR_STRBUF_SPACE 4
#define SUPER_FILTER_ERROR_BUF_SPACE    5
#define SUPER_FILTER_ERROR_TYPE_MISS    9
#define SUPER_FILTER_ERROR_EMPTY        11
#define SUPER_FILTER_ERROR_EXP_SPACE    16
#define SUPER_FILTER_ERROR_UNKNOWN_FN   18

struct fvar_struct {
    bool str;
    int s;
    char * p;
    int used;
};

struct super_filter_session;
struct super_filter_expression {
    void (*fn)(struct super_filter_expression *exp);
    struct super_filter_expression * arg1;
    struct super_filter_expression * arg2;
    struct super_filter_expression * arg3;
    union {
        struct fvar_struct *value;
        struct super_filter_session *sess;
    } data;
    bool str;
    int s;
    char * p;
    int op;
};

struct super_filter_args {
    struct fileheader *curfh;    /* 比较的 fileheader，telnet 下是进入超级搜索前光标所在的帖子 */
    struct fileheader *ptr;     /* .DIR 索引中遍历的 fileheader */
    char *boardname;
};


struct super_filter_vars {
    const char *name;
    int bmonly;
    int (*fn_int)(struct super_filter_args *arg);
    char *(*fn_string)(struct super_filter_args *arg);
};
struct super_filter_used_vars {
    const struct super_filter_vars *var;
    struct fvar_struct *pos;
};
    
#define SUPER_FILTER_PAIR(name) { #name, 0, sffn_##name, NULL }
#define SUPER_FILTER_BM_PAIR(name) { #name, 1, sffn_##name, NULL }
#define SUPER_FILTER_STRING_PAIR(name) { #name, 0, NULL, sffn_##name }

static int sffn_cid(struct super_filter_args *arg) {
    return (arg->curfh->id);
}
static int sffn_creid(struct super_filter_args *arg) {
    return (arg->curfh->reid);
}
static int sffn_cgroupid(struct super_filter_args *arg) {
     return (arg->curfh->groupid);
}
static char *sffn_cauthor(struct super_filter_args *arg) {
    return (arg->curfh->owner);
}
static int sffn_cftime(struct super_filter_args *arg) {
    return (get_posttime(arg->curfh));
}
static int sffn_ceffsize(struct super_filter_args *arg) {
    return (arg->curfh->eff_size);
}
static int sffn_id(struct super_filter_args *arg) {
    return (arg->ptr->id);
}
static int sffn_reid(struct super_filter_args *arg) {
    return (arg->ptr->reid);
}
static int sffn_groupid(struct super_filter_args *arg) {
    return (arg->ptr->groupid);
}
static int sffn_thread(struct super_filter_args *arg) {
    return (arg->ptr->groupid== arg->curfh->groupid);
}
static int sffn_origin(struct super_filter_args *arg) {
    return (arg->ptr->id==arg->ptr->groupid);
}
static int sffn_m(struct super_filter_args *arg) {
    return (arg->ptr->accessed[0]&FILE_MARKED);
}
static int sffn_g(struct super_filter_args *arg) {
    return (arg->ptr->accessed[0]&FILE_DIGEST);
}
static int sffn_b(struct super_filter_args *arg) {
    return ((arg->ptr->accessed[0]&FILE_MARKED)&&(arg->ptr->accessed[0]&FILE_DIGEST));
}
static int sffn_noreply(struct super_filter_args *arg) {
    return (arg->ptr->accessed[1]&FILE_READ);
}
static int sffn_sign(struct super_filter_args *arg) {
    return (arg->ptr->accessed[0]&FILE_SIGN);
}
static int sffn_percent(struct super_filter_args *arg) {
	return (arg->ptr->accessed[0]&FILE_PERCENT);
}
#ifdef FILTER
static int sffn_censor(struct super_filter_args *arg) {
    return (arg->ptr->accessed[1]&FILE_CENSOR);
}
#endif
static int sffn_del(struct super_filter_args *arg) {
    return (arg->ptr->accessed[1]&FILE_DEL);
}
static int sffn_import(struct super_filter_args *arg) {
    return (arg->ptr->accessed[0]&FILE_IMPORTED);
}
static int sffn_a(struct super_filter_args *arg) {
    return (arg->ptr->attachment);
}
static char *sffn_title(struct super_filter_args *arg) {
    return (arg->ptr->title);
}
static char *sffn_author(struct super_filter_args *arg) {
    return (arg->ptr->owner);
}
static char *sffn_fname(struct super_filter_args *arg) {
    return (arg->ptr->filename);
}
static int sffn_my(struct super_filter_args *arg) {
    return (!strcmp(arg->ptr->owner,getCurrentUser()->userid));
}
#ifdef HAVE_BRC_CONTROL
static int sffn_unread(struct super_filter_args *arg) {
    return (brc_unread(arg->ptr->id, getSession()));
}
#endif
static int sffn_ftime(struct super_filter_args *arg) {
    return (get_posttime(arg->ptr));
}
static int sffn_effsize(struct super_filter_args *arg) {
    return (arg->ptr->eff_size);
}

static int sffn_asize(struct super_filter_args *arg) {
    char ffn[PATHLEN];
    struct stat st;
    setbfile(ffn, arg->boardname, arg->ptr->filename);
    return (stat(ffn, &st)!=-1) ? st.st_size : 0;
}

const static struct super_filter_vars varnames[] = {
    SUPER_FILTER_PAIR(cid),
    SUPER_FILTER_PAIR(creid),
    SUPER_FILTER_PAIR(cgroupid),
    SUPER_FILTER_PAIR(cftime),
    SUPER_FILTER_PAIR(ceffsize),
    SUPER_FILTER_PAIR(id),          {"文章号", 0, sffn_id, NULL},
    SUPER_FILTER_PAIR(reid),
    SUPER_FILTER_PAIR(groupid),
    SUPER_FILTER_PAIR(thread),      {"本主题", 0, sffn_thread, NULL},
    SUPER_FILTER_PAIR(origin),      {"原作", 0, sffn_origin, NULL},
    SUPER_FILTER_PAIR(m),           {"保留", 0, sffn_m, NULL}, 
    SUPER_FILTER_PAIR(g),           {"文摘", 0, sffn_g, NULL},
    SUPER_FILTER_PAIR(b),
    SUPER_FILTER_PAIR(a),           {"attach", 0, sffn_a, NULL}, {"附件", 0, sffn_a, NULL},
    SUPER_FILTER_PAIR(my),          {"我的", 0, sffn_my, NULL}, 
#ifdef HAVE_BRC_CONTROL
    SUPER_FILTER_PAIR(unread),      {"未读", 0, sffn_unread, NULL}, 
#endif
    SUPER_FILTER_PAIR(ftime),       {"时间", 0, sffn_ftime, NULL}, 
    SUPER_FILTER_PAIR(effsize),     {"有效长度", 0, sffn_effsize, NULL}, 
    SUPER_FILTER_PAIR(asize),       {"总长度", 0, sffn_asize, NULL}, 

    /* 开放 noreply 搜索 - atppp 20060117 */
    SUPER_FILTER_PAIR(noreply),     {"不可回复", 0, sffn_noreply, NULL},
    
    SUPER_FILTER_STRING_PAIR(cauthor), 
    SUPER_FILTER_STRING_PAIR(title),    {"标题", 0, NULL, sffn_title}, 
    SUPER_FILTER_STRING_PAIR(author),   {"作者", 0, NULL, sffn_author},  
    SUPER_FILTER_STRING_PAIR(fname),    {"文件名", 0, NULL, sffn_fname},

    SUPER_FILTER_BM_PAIR(sign),     {"标记", 1, sffn_sign, NULL}, 
    SUPER_FILTER_BM_PAIR(percent),  {"百分号", 1, sffn_percent, NULL}, 
#ifdef FILTER
    SUPER_FILTER_BM_PAIR(censor),   {"审核", 1, sffn_censor, NULL}, 
#endif
    SUPER_FILTER_BM_PAIR(del),      {"删除", 1, sffn_del, NULL}, 
    SUPER_FILTER_BM_PAIR(import),   {"精华", 1, sffn_import, NULL}
};

#define NUM_VARNAMES  sizeof(varnames) / sizeof(struct super_filter_vars)


struct content_struct {
    bool mmap;
    char *ptr;
    off_t fsize;
    int searchsize;
};

struct super_filter_session {
    struct fvar_struct fvars[NUM_VARNAMES];
    struct fvar_struct loop;
    struct content_struct content;
    char strbuf[STRBUFLEN];
    char * strbufptr;
    struct super_filter_expression fexp[MAX_EXPRESSION];
    int fexpt;
    int ferr;
    int isbm;
    struct super_filter_args arg;
};

#define FMAKESURE(o,p) if(sess->ferr) return;\
    if(!(o)) {sess->ferr=SUPER_FILTER_ERROR_##p; return;}

#define REQ_EXP(o)  { FMAKESURE(sess->fexpt < MAX_EXPRESSION, EXP_SPACE); \
    o = &sess->fexp[sess->fexpt]; sess->fexpt++;}

#define REQ_STRBUF(ret, len)  { \
        FMAKESURE((len)+1+sess->strbufptr<sess->strbuf+STRBUFLEN,STRBUF_SPACE); \
        ret->p = sess->strbufptr; \
        ret->str = true; \
        sess->strbufptr += (len) + 1; \
    }

static struct fvar_struct* fget_var(struct super_filter_session* sess, const char * name)
{
    int i;
    if(sess->ferr) return NULL;
    for(i=0;i<NUM_VARNAMES;i++) {
        if (varnames[i].bmonly && !sess->isbm) continue;
        if(!strcasecmp(varnames[i].name, name)) {
            sess->fvars[i].used = true;
            return &sess->fvars[i];
        }
    }
    return NULL;
}

static int fcheck_var_name(char * s, int l)
{
    int i,p=1;
    for(i=0;i<l;i++)
        p=p&&(isalpha(s[i])||s[i]<0);
    return p;
}

static int check_var_int(char * s, int l)
{
    int i,p=1;
    for(i=0;i<l;i++)
        p=p&&(isdigit(s[i]));
    return p;
}

static int get_rl(char * s, int r, int l)
{
    int n=r,i=0;
    do{
        if(s[n]==')') i++;
        if(s[n]=='(') i--;
        n--;
    }while(i!=0&&n>=l);
    if(i==0) return n+1;
    else return -1;
}

static int get_rl2(char * s, int r, int l)
{
    int n=r,i=0;
    do{
        if(s[n]=='\'') i++;
        if(s[n]=='"') i++;
        if(i==2) return n;
        n--;
    }while(n>=l);
    return -1;
}

static void feval_expression(struct super_filter_expression *exp) {
    if (exp->fn) {
        exp->fn(exp);
    } else if (exp->data.value) {
        exp->p = exp->data.value->p;
        exp->s = exp->data.value->s;
    }
    /* otherwise, use what was in exp->p,s */
}

static void sff_sub(struct super_filter_expression *exp) {
    char *res;
    feval_expression(exp->arg1);
    feval_expression(exp->arg2);
    res = strcasestr(exp->arg1->p, exp->arg2->p);
    if(res==NULL) exp->s=0;
    else exp->s=res-exp->arg1->p+1;
}

static void sff_len(struct super_filter_expression *exp) {
    feval_expression(exp->arg1);
    exp->s = strlen(exp->arg1->p);
}

static void sff_date(struct super_filter_expression *exp) {
    struct tm t;
    feval_expression(exp->arg1);
    feval_expression(exp->arg2);
    feval_expression(exp->arg3);
    t.tm_sec=0;
    t.tm_min=0;
    t.tm_hour=0;
    t.tm_mday=exp->arg3->s;
    t.tm_mon=exp->arg2->s-1;
    t.tm_year=exp->arg1->s-1900;
    exp->s = mktime(&t);
}

static void sff_time(struct super_filter_expression *exp) {
    feval_expression(exp->arg1);
    feval_expression(exp->arg2);
    feval_expression(exp->arg3);
    exp->s = (exp->arg1->s * 60 + exp->arg2->s) * 60 + exp->arg3->s;
}

static void sff_num(struct super_filter_expression *exp) {
    feval_expression(exp->arg1);
    exp->s = atoi(exp->arg1->p);
}

static void sff_str(struct super_filter_expression *exp) {
    feval_expression(exp->arg1);
    snprintf(exp->p, STR_CONV_LEN, "%d", exp->arg1->s);
}

static void sff_or(struct super_filter_expression *exp) {
    feval_expression(exp->arg1);
    if (exp->arg1->s) {
        exp->s = 1;
    } else {
        feval_expression(exp->arg2);
        exp->s=exp->arg2->s;
    }
}

static void sff_and(struct super_filter_expression *exp) {
#if 1
    feval_expression(exp->arg1);
    if (!exp->arg1->s) {
        exp->s = 0;
    } else {
        feval_expression(exp->arg2);
        exp->s=exp->arg2->s;
    }
#else
    feval_expression(exp->arg1);
    feval_expression(exp->arg2);
    exp->s = exp->arg1->s && exp->arg2->s;
#endif
}

static void sff_op(struct super_filter_expression *exp) {
    char *res;
    feval_expression(exp->arg1);
    feval_expression(exp->arg2);
    switch(exp->op) {
        case 2:
            exp->s=exp->arg1->s==exp->arg2->s;
            break;
        case -2:
            exp->s=!strcasecmp(exp->arg1->p,exp->arg2->p);
            break;
        case 3:
            exp->s=exp->arg1->s!=exp->arg2->s;
            break;
        case -3:
            exp->s=strcasecmp(exp->arg1->p,exp->arg2->p);
            break;
        case 4:
            exp->s=exp->arg1->s>=exp->arg2->s;
            break;
        case 5:
            exp->s=exp->arg1->s<=exp->arg2->s;
            break;
        case 6:
            exp->s=exp->arg1->s>exp->arg2->s;
            break;
        case 7:
            exp->s=exp->arg1->s<exp->arg2->s;
            break;
        case 8:
            exp->s=exp->arg1->s+exp->arg2->s;
            break;
        case 9:
            exp->s=exp->arg1->s-exp->arg2->s;
            break;
        case 10:
            exp->s=exp->arg1->s*exp->arg2->s;
            break;
        case 11:
            if (exp->arg2->s==0) exp->s = 0;
            exp->s=exp->arg1->s/exp->arg2->s;
            break;
        case 12:
            if (exp->arg2->s==0) exp->s = 0;
            exp->s=exp->arg1->s%exp->arg2->s;
            break;
        case 13:
            res = strcasestr(exp->arg1->p, exp->arg2->p);
            if(res==NULL) exp->s=0;
            else exp->s=res-exp->arg1->p+1;
            break;
    }
}

static void sff_not(struct super_filter_expression *exp) {
    feval_expression(exp->arg1);
    exp->s = !exp->arg1->s;
}

static void sff_content(struct super_filter_expression *exp) {
    /* content(str)
     * exp->arg1 points to str expression
     */
    char *res;
    struct super_filter_session *sess = exp->data.sess;
    if (!sess->content.mmap) {
        char ffn[PATHLEN];
        struct fileheader * fh = sess->arg.ptr;
        setbfile(ffn, sess->arg.boardname, fh->filename);
        sess->content.mmap = safe_mmapfile(ffn, O_RDONLY, PROT_READ, MAP_SHARED, &sess->content.ptr, &sess->content.fsize, NULL);
        if(sess->content.mmap) {
            if (fh->attachment) sess->content.searchsize = -1;
            else sess->content.searchsize = sess->content.fsize;
        } else {
            exp->s = 0;
            return;
        }
    }
    feval_expression(exp->arg1);
    if (sess->content.searchsize <= 0) { //has attachment. article content has \0 ending.
        res = strcasestr(sess->content.ptr, exp->arg1->p);
    } else {
        res = (char *)memmem(sess->content.ptr, sess->content.searchsize, exp->arg1->p, strlen(exp->arg1->p));
    }
    if(res == NULL) exp->s = 0;
    else exp->s = res - sess->content.ptr + 1;
}

static void feval(struct super_filter_expression *ret, char * s, int l, int r, struct super_filter_session* sess)
{
    int i,j,n;
    char op[14][4]={"||","&&","==","!=",">=","<=",">","<","+","-","*","/","%","<<"};
    char op2[14][10]={"或","且","是","不是","大等于","小等于","大于","小于","加","减","乘","除","模", "包含"};
    char buf[STRBUFLEN];
    while(s[l]==' '&&l<=r) l++;
    while(s[r]==' '&&l<=r) r--;
    FMAKESURE(l<=r,EMPTY);
    while(s[l]=='('&&s[r]==')'&&get_rl(s,r,l)==l) {
        l++; r--;
        while(s[l]==' '&&l<=r) l++;
        while(s[r]==' '&&l<=r) r--;
    }
    if(fcheck_var_name(s+l, r-l+1)) {
        FMAKESURE(r-l+1<sizeof(buf), BUF_SPACE);
        strncpy(buf, s+l, r-l+1);
        buf[r-l+1]=0;
        if (!strcmp(buf, "no")) {
            ret->data.value = &sess->loop;
        } else {
            ret->data.value = fget_var(sess, buf);
        }
        if (ret->data.value) {
            ret->str = ret->data.value->str;
            ret->p = ret->data.value->p;
            return;
        }
    }
    if(check_var_int(s+l, r-l+1)) {
        FMAKESURE(r-l+1<sizeof(buf), BUF_SPACE);
        strncpy(buf, s+l, r-l+1);
        buf[r-l+1]=0;
        ret->s = atoi(buf);
        return;
    }
    if((s[l]=='\''||s[l]=='"')&&(s[r]=='\''||s[r]=='"')&&get_rl2(s,r,l)==l) {
        REQ_STRBUF(ret, r-l-1);
        strncpy(ret->p, s+l+1, r-l-1);
        ret->p[r-l-1]=0;
        return;
    }
    i=l;
    while(isalpha(s[i])&&i<=r) i++;
    if(i>l&&s[i]=='('&&s[r]==')'&&get_rl(s,r,l)==i) {
        struct super_filter_expression *u,*v,*w;

        FMAKESURE(i-l<sizeof(buf), BUF_SPACE);
        strncpy(buf, s+l, i-l);
        buf[i-l]=0;
        if(!strcmp("sub",buf)) {
            int j=strchr(s+i+1, ',')-s;
            REQ_EXP(u);
            REQ_EXP(v);
            FMAKESURE(strchr(s+i+1, ',')!=NULL, FN_SYNTAX);
            FMAKESURE(strchr(s+i+1, ',')<=s+r, FN_SYNTAX);
            feval(u, s, i+1, j-1, sess);
            FMAKESURE(u->str&&u->p, TYPE_MISS);
            feval(v, s, j+1, r-1, sess);
            FMAKESURE(v->str&&v->p, TYPE_MISS);
            ret->arg1 = v;
            ret->arg2 = u;
            ret->fn = sff_sub;
            return;
        } else if(!strcmp("len",buf)){
            REQ_EXP(u);
            feval(u, s, i+1, r-1, sess);
            FMAKESURE(u->str&&u->p, TYPE_MISS);
            ret->arg1 = u;
            ret->fn = sff_len;
            return;
        } else if (!strcmp("num",buf)){
            REQ_EXP(u);
            feval(u, s, i+1, r-1, sess);
            FMAKESURE(u->str&&u->p, TYPE_MISS);
            ret->arg1 = u;
            ret->fn = sff_num;
            return;
        } else if (!strcmp("str",buf)){
            REQ_STRBUF(ret, STR_CONV_LEN);
            REQ_EXP(u);
            feval(u, s, i+1, r-1, sess);
            FMAKESURE(!u->str, TYPE_MISS);
            ret->arg1 = u;
            ret->fn = sff_str;
            return;
        } else if (!strcmp("content",buf)/* && sess->isbm */){
            REQ_EXP(u);
            feval(u, s, i+1, r-1, sess);
            FMAKESURE(u->str&&u->p, TYPE_MISS);
            ret->arg1 = u;
            ret->fn = sff_content;
            ret->data.sess = sess;
            return;
        } else if(!strcmp("date",buf)){
            int j=strchr(s+i+1, ',')-s, k;
            REQ_EXP(u);
            REQ_EXP(v);
            REQ_EXP(w);
            FMAKESURE(strchr(s+i+1, ',')!=NULL, FN_SYNTAX);
            FMAKESURE(strchr(s+i+1, ',')<=s+r, FN_SYNTAX);
            FMAKESURE(strchr(s+j+1, ',')!=NULL, FN_SYNTAX);
            FMAKESURE(strchr(s+j+1, ',')<=s+r, FN_SYNTAX);
            k=strchr(s+j+1, ',')-s;
            feval(u, s, i+1, j-1, sess);
            FMAKESURE(!u->str, TYPE_MISS);
            feval(v, s, j+1, k-1, sess);
            FMAKESURE(!v->str, TYPE_MISS);
            feval(w, s, k+1, r-1, sess);
            FMAKESURE(!w->str, TYPE_MISS);
            ret->fn = sff_date;
            ret->arg1 = u;
            ret->arg2 = v;
            ret->arg3 = w;
            return;
        } else if(!strcmp("today",buf)){
            struct tm t;
            time_t tt;
            ret->fn = NULL;
            ret->data.value = NULL;
            tt=time(0);
            gmtime_r(&tt, &t);
            t.tm_sec=0;
            t.tm_min=0;
            t.tm_hour=0;
            ret->s = mktime(&t);
            return;
        } else if(!strcmp("time",buf)){
            int j=strchr(s+i+1, ',')-s, k;
            REQ_EXP(u);
            REQ_EXP(v);
            REQ_EXP(w);
            FMAKESURE(strchr(s+i+1, ',')!=NULL, FN_SYNTAX);
            FMAKESURE(strchr(s+i+1, ',')<=s+r, FN_SYNTAX);
            FMAKESURE(strchr(s+j+1, ',')!=NULL, FN_SYNTAX);
            FMAKESURE(strchr(s+j+1, ',')<=s+r, FN_SYNTAX);
            k=strchr(s+j+1, ',')-s;
            feval(u, s, i+1, j-1, sess);
            FMAKESURE(!u->str, TYPE_MISS);
            feval(v, s, j+1, k-1, sess);
            FMAKESURE(!v->str, TYPE_MISS);
            feval(w, s, k+1, r-1, sess);
            FMAKESURE(!w->str, TYPE_MISS);
            ret->fn = sff_time;
            ret->arg1 = u;
            ret->arg2 = v;
            ret->arg3 = w;
            return;
        }
        sess->ferr=SUPER_FILTER_ERROR_UNKNOWN_FN;
        return ;
    }
    for(j=0;j<14;j++) {
        n=r;
        do{
            if((n+strlen(op[j])<=r&&!strncmp(s+n, op[j], strlen(op[j])))
             ||(n+strlen(op2[j])<=r&&!strncmp(s+n, op2[j], strlen(op2[j])))) {
                struct super_filter_expression *m1, *m2;
                REQ_EXP(m1);
                REQ_EXP(m2);
                feval(m1,s,l,n-1, sess);
                if(j==2||j==3) {FMAKESURE(!m1->str||(m1->str&&m1->p),TYPE_MISS);}
                else if(j==13) {FMAKESURE(m1->str,TYPE_MISS);}
                else {FMAKESURE(!m1->str,TYPE_MISS);}
                if(!strncmp(s+n, op[j], strlen(op[j])))
                    feval(m2,s,n+strlen(op[j]),r,sess);
                else
                    feval(m2,s,n+strlen(op2[j]),r,sess);
                if(j==2||j==3) {
                    if (!m1->str&&!m2->str) {
                    } else if (m1->str&&m2->str&&m2->p) {
                        j = -j;
                    } else {
                        sess->ferr = SUPER_FILTER_ERROR_TYPE_MISS;
                        return;
                    }
                }
                else if(j==13) {FMAKESURE(m2->str,TYPE_MISS);}
                else {FMAKESURE(!m2->str,TYPE_MISS);}
                ret->arg1 = m1;
                ret->arg2 = m2;
                if (j == 0) {
                    ret->fn = sff_or;
                } else if (j == 1) {
                    ret->fn = sff_and;
                } else {
                    ret->fn = sff_op;
                    ret->op = j;
                }
                return;
            }
            if(s[n]==')') n=get_rl(s,n,l);
            if(s[n]=='\'') {n--;while(n>=l&&s[n]!='\'') n--;}
            if(s[n]=='"') {n--;while(n>=l&&s[n]!='"') n--;}
            n--;
        }while(n>=l);
    }
    if (r-l>=6&&!strncmp("的长度", s+r-5, 6)) {
        struct super_filter_expression *u;
        REQ_EXP(u);
        feval(u, s, l, r-6, sess);
        FMAKESURE(u->str&&u->p, TYPE_MISS);
        ret->arg1 = u;
        ret->fn = sff_len;
        return;
    }
    if(s[l]=='!') {
        struct super_filter_expression *m;
        REQ_EXP(m);
        feval(m, s, l+1, r, sess);
        FMAKESURE(!m->str,TYPE_MISS);
        ret->fn = sff_not;
        ret->arg1 = m;
        return;
    }
    if(!strncmp(s+l,"没有",4)) {
        struct super_filter_expression *m;
        REQ_EXP(m);
        feval(m, s, l+4, r, sess);
        FMAKESURE(!m->str,TYPE_MISS);
        ret->fn = sff_not;
        ret->arg1 = m;
        return;
    }
    REQ_STRBUF(ret, r-l+1);
    strncpy(ret->p, s+l, r-l+1);
    ret->p[r-l+1]=0;
}

int query_super_filter_mmap(struct fileheader *allfh, int start, int total, int down, struct super_filter_query_arg *q_arg) {
    int i, count = 0;
    struct super_filter_session sess;
    struct super_filter_used_vars used_vars[NUM_VARNAMES];
    int n_used_vars = 0;
    struct super_filter_expression res;

    struct flock ldata;
    int write_fd = -1;

    if (start < 0 || start >= total) return 0;
    if (strlen(q_arg->query) >= STRBUFLEN)
        return -SUPER_FILTER_ERROR_GENERAL;
    
    /* these initialize a lot of things ... */
    memset(&sess, 0, sizeof(sess));
    memset(&res, 0, sizeof(res));

    if (q_arg->write_file) {
        if ((write_fd = open(q_arg->write_file, O_WRONLY | O_CREAT, 0664)) == -1) {
            bbslog("user", "%s", "recopen err");
            return -SUPER_FILTER_ERROR_GENERAL;      /* 创建文件发生错误*/
        }
        ldata.l_type = F_WRLCK;
        ldata.l_whence = 0;
        ldata.l_len = 0;
        ldata.l_start = 0;
        if (fcntl(write_fd, F_SETLKW, &ldata) == -1) {
            bbslog("user", "%s", "reclock err");
            close(write_fd);
            return -SUPER_FILTER_ERROR_GENERAL;      /* lock error*/
        }
    }
    
    /* fake precompile... */
    {
        sess.strbufptr = sess.strbuf;
        sess.isbm = q_arg->isbm;
        
        for (i = 0; i < NUM_VARNAMES; i++) {
            if (varnames[i].fn_string) {
                sess.fvars[i].str = true;
                sess.fvars[i].p = "";
            } else {
                sess.fvars[i].str = false;
                sess.fvars[i].s = 0;
            }
        }

        feval(&res, q_arg->query, 0, strlen(q_arg->query)-1, &sess);
        if(sess.ferr) {
            return -sess.ferr;
        }
        for (i = 0; i < NUM_VARNAMES; i++) {
            if (!sess.fvars[i].used) continue;
            used_vars[n_used_vars].var = &varnames[i];
            used_vars[n_used_vars].pos = &sess.fvars[i];
            n_used_vars++;
        }
    }

    sess.arg.boardname = q_arg->boardname;
    sess.arg.curfh = q_arg->curfh;
    sess.arg.ptr = allfh + start;

    sess.loop.s = start + 1;
    while(1) {
        for (i = 0; i < n_used_vars; i++) {
            if (used_vars[i].var->fn_int) {
                used_vars[i].pos->s = used_vars[i].var->fn_int(&sess.arg);
            } else {
                used_vars[i].pos->p = used_vars[i].var->fn_string(&sess.arg);
            }
        }

        feval_expression(&res);

        if (sess.content.mmap) {
            sess.content.mmap = false;
            end_mmapfile((void*)sess.content.ptr, sess.content.fsize, -1);
        }

        if(res.s) {
            if (q_arg->write_file) {
                write(write_fd, sess.arg.ptr, sizeof(struct fileheader));
            } else if (q_arg->array) {
                if (count < q_arg->array_size) {
                    struct fileheader_num *fhn = &q_arg->array[count];
                    memcpy(&fhn->fh, sess.arg.ptr, sizeof(struct fileheader));
                    fhn->num = sess.loop.s;
                    if (!q_arg->detectmore && (count == q_arg->array_size - 1)) {
                        return q_arg->array_size;
                    }
                } else {
                    count++;
                    break;
                }
            }
            count++;
        }
        if (down) {
            sess.loop.s++;
            sess.arg.ptr++;
            if (sess.loop.s > total) break;
        } else {
            sess.loop.s--;
            sess.arg.ptr--;
            if (sess.loop.s < 1) break;
        }
    }
    if (q_arg->write_file) {
        ftruncate(write_fd, count * sizeof(struct fileheader));
        ldata.l_type = F_UNLCK;
        fcntl(write_fd, F_SETLKW, &ldata);        /* 退出互斥区域*/
        close(write_fd);
    }
    return count;
}


int query_super_filter(int fd, struct super_filter_query_arg *q_arg) {
    struct flock ldata2;
    int fd2, total, i, count;
    char direct[PATHLEN];
    char *ptr;
    struct stat buf;
    if (fd == -1) {
        setbdir(DIR_MODE_NORMAL, direct, q_arg->boardname);
        if ((fd2 = open(direct, O_RDONLY, 0664)) == -1) {
            bbslog("user", "%s", "recopen err");
            return -SUPER_FILTER_ERROR_GENERAL;
        }
        ldata2.l_type = F_RDLCK;
        ldata2.l_whence = 0;
        ldata2.l_len = 0;
        ldata2.l_start = 0;
        fcntl(fd2, F_SETLKW, &ldata2);
    } else {
        fd2 = fd;
    }
    fstat(fd2, &buf);
    total = buf.st_size / sizeof(struct fileheader);

    if ((i = safe_mmapfile_handle(fd2, PROT_READ, MAP_SHARED, &ptr, &buf.st_size)) != 1) {
        if (i == 2)
            end_mmapfile((void *) ptr, buf.st_size, -1);
        if (fd == -1) {
            ldata2.l_type = F_UNLCK;
            fcntl(fd2, F_SETLKW, &ldata2);
            close(fd2);
        }
        return -SUPER_FILTER_ERROR_GENERAL;
    }

    count = query_super_filter_mmap((struct fileheader *) ptr, 0, total, true, q_arg);

    end_mmapfile((void *) ptr, buf.st_size, -1);
    if (fd == -1) {
        ldata2.l_type = F_UNLCK;
        fcntl(fd2, F_SETLKW, &ldata2);
        close(fd2);
    }
    return count;
}

