#include "bbs.h"

#define MAX_FVAR 100
#define LIBLEN 1000

int ferr=0;

static struct fvar_struct {
    bool num;
    char name[12];
    int s;
    char * p;
};

struct fvar_struct fvars[MAX_FVAR];
int fvart = 0;
char * libs, * libptr;

#define fmakesure(o,p) if(ferr) return;\
    if(!(o)) {ferr=p; return;}

static int fget_var(char * name)
{
    int i;
    if(ferr) return 0;
    if(!name[0]||strlen(name)>11) {
        ferr=14;
        return 0;
    }
    for(i=0;i<fvart;i++)
        if(!strncasecmp(fvars[i].name, name, 12)) {
            return i;
        }
    if(fvart>=MAX_FVAR) {
        ferr=15;
        return 0;
    }
    strncpy(fvars[fvart].name, name, 12);
    fvars[fvart].p = 0;
    fvars[fvart].num = true;
    fvart++;
    return (fvart-1);
}

static int fexist_var(char * name)
{
    int i;
    if(ferr) return 0;
    if(!name[0]||strlen(name)>11) {
        return -1;
    }
    for(i=0;i<fvart;i++)
        if(!strncasecmp(fvars[i].name, name, 12)) {
            return i;
        }
    return -1;
}

static void set_vard(struct fvar_struct * a, int f)
{
    a->num = true;
    a->s = f;
}

static void set_vars(struct fvar_struct * a, char * f)
{
    a->num = false;
    a->p = f;
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

static void feval(struct fvar_struct * p, char * s, int l, int r)
{
    int i,j,n;
    char op[14][4]={"||","&&","==","!=",">=","<=",">","<","+","-","*","/","%","<<"};
    char op2[14][10]={"或","且","是","不是","大等于","小等于","大于","小于","加","减","乘","除","模", "包含"};
    char buf[1000];
    while(s[l]==' '&&l<=r) l++;
    while(s[r]==' '&&l<=r) r--;
    fmakesure(l<=r,11);
    while(s[l]=='('&&s[r]==')'&&get_rl(s,r,l)==l) {
        l++; r--;
        while(s[l]==' '&&l<=r) l++;
        while(s[r]==' '&&l<=r) r--;
    }
    if(fcheck_var_name(s+l, r-l+1)) {
        strncpy(buf, s+l, 1000);
        buf[r-l+1]=0;
        if(fexist_var(buf)!=-1) {
            i=fget_var(buf);
            p->num = fvars[i].num;
            p->s = fvars[i].s;
            p->p = fvars[i].p;
            return;
        }
    }
    if(check_var_int(s+l, r-l+1)) {
        int f;
        strncpy(buf, s+l, 1000);
        buf[r-l+1]=0;
        f = atoi(buf);
        set_vard(p, f);
        return;
    }
    if((s[l]=='\''||s[l]=='"')&&(s[r]=='\''||s[r]=='"')&&get_rl2(s,r,l)==l) {
        fmakesure(r-l+libptr<libs+LIBLEN,4);
        strncpy(libptr, s+l+1, r-l-1);
        libptr[r-l-1]=0;
        p->num=false;
        p->p=libptr;
        libptr+=r-l;
        return;
    }
    i=l;
    while(isalpha(s[i])&&i<=r) i++;
    if(i>l&&s[i]=='('&&s[r]==')'&&get_rl(s,r,l)==i) {
        struct fvar_struct u,v,w;
        u.p=0; v.p=0; w.p=0;
        strncpy(buf, s+l, 1000);
        buf[i-l]=0;
        if(!strcmp("sub",buf)) {
            int j=strchr(s+i+1, ',')-s;
            char * res;
            fmakesure(strchr(s+i+1, ',')!=NULL, 2);
            fmakesure(strchr(s+i+1, ',')<=s+r, 2);
            feval(&u, s, i+1, j-1);
            fmakesure(!u.num&&u.p, 2);
            feval(&v, s, j+1, r-1);
            fmakesure(!v.num&&v.p, 2);
            p->num=true;
            res = bm_strcasestr(v.p, u.p);
            if(res==NULL) p->s=0;
            else p->s=res-v.p+1;
            return;
        }
        else if(!strcmp("len",buf)){
            feval(&u, s, i+1, r-1);
            fmakesure(!u.num&&u.p, 3);
            p->num=true;
            p->s = strlen(u.p);
            return;
        }
        else if(!strcmp("date",buf)){
            int j=strchr(s+i+1, ',')-s, k;
            struct tm t;
            fmakesure(strchr(s+i+1, ',')!=NULL, 4);
            fmakesure(strchr(s+i+1, ',')<=s+r, 4);
            fmakesure(strchr(s+j+1, ',')!=NULL, 4);
            fmakesure(strchr(s+j+1, ',')<=s+r, 4);
            k=strchr(s+j+1, ',')-s;
            feval(&u, s, i+1, j-1);
            fmakesure(u.num, 4);
            feval(&v, s, j+1, k-1);
            fmakesure(v.num, 4);
            feval(&w, s, k+1, r-1);
            fmakesure(w.num, 4);
            p->num=true;
            t.tm_sec=0;
            t.tm_min=0;
            t.tm_hour=0;
            t.tm_mday=w.s;
            t.tm_mon=v.s-1;
            t.tm_year=u.s-1900;
            p->s = mktime(&t);
            return;
        }
        else if(!strcmp("today",buf)){
            struct tm t;
            time_t tt;
            p->num=true;
            tt=time(0);
            gmtime_r(&tt, &t);
            t.tm_sec=0;
            t.tm_min=0;
            t.tm_hour=0;
            p->s = mktime(&t);
            return;
        }
        else if(!strcmp("time",buf)){
            int j=strchr(s+i+1, ',')-s, k;
            fmakesure(strchr(s+i+1, ',')!=NULL, 4);
            fmakesure(strchr(s+i+1, ',')<=s+r, 4);
            fmakesure(strchr(s+j+1, ',')!=NULL, 4);
            fmakesure(strchr(s+j+1, ',')<=s+r, 4);
            k=strchr(s+j+1, ',')-s;
            feval(&u, s, i+1, j-1);
            fmakesure(u.num, 4);
            feval(&v, s, j+1, k-1);
            fmakesure(v.num, 4);
            feval(&w, s, k+1, r-1);
            fmakesure(w.num, 4);
            p->num=true;
            p->s = (u.s*60+v.s)*60+w.s;
            return;
        }
        ferr=18;
        return ;
    }
    for(j=0;j<14;j++) {
        n=r;
        do{
            if((n+strlen(op[j])<=r&&!strncmp(s+n, op[j], strlen(op[j])))
             ||(n+strlen(op2[j])<=r&&!strncmp(s+n, op2[j], strlen(op2[j])))) {
                struct fvar_struct m1,m2,m3;
                char * res;
                m1.p=0; m2.p=0; m3.p=0;
                feval(&m1,s,l,n-1);
                if(j==2||j==3) {fmakesure(m1.num||(!m1.num&&m1.p),1);}
                else if(j==13) {fmakesure(!m1.num,1);}
                else {fmakesure(m1.num,1);}
                if(!strncmp(s+n, op[j], strlen(op[j])))
                    feval(&m2,s,n+strlen(op[j]),r);
                else
                    feval(&m2,s,n+strlen(op2[j]),r);
                if(j==2||j==3) {fmakesure((m1.num&&m2.num)||(!m1.num&&!m2.num&&m2.p),1);}
                else if(j==13) {fmakesure(!m2.num,1);}
                else {fmakesure(m2.num,1);}
                p->num=true;
                switch(j) {
                    case 0:
                        p->s=m1.s||m2.s;
                        break;
                    case 1:
                        p->s=m1.s&&m2.s;
                        break;
                    case 2:
                        if(m1.num) p->s=m1.s==m2.s;
                        else p->s=!strcasecmp(m1.p,m2.p);
                        break;
                    case 3:
                        if(m1.num) p->s=m1.s!=m2.s;
                        else p->s=strcasecmp(m1.p,m2.p);
                        break;
                    case 4:
                        p->s=m1.s>=m2.s;
                        break;
                    case 5:
                        p->s=m1.s<=m2.s;
                        break;
                    case 6:
                        p->s=m1.s>m2.s;
                        break;
                    case 7:
                        p->s=m1.s<m2.s;
                        break;
                    case 8:
                        p->s=m1.s+m2.s;
                        break;
                    case 9:
                        p->s=m1.s-m2.s;
                        break;
                    case 10:
                        p->s=m1.s*m2.s;
                        break;
                    case 11:
                        fmakesure(m2.s!=0, 6);
                        p->s=m1.s/m2.s;
                        break;
                    case 12:
                        fmakesure(m2.s!=0, 6);
                        p->s=m1.s%m2.s;
                        break;
                    case 13:
                        res = bm_strcasestr(m1.p, m2.p);
                        if(res==NULL) p->s=0;
                        else p->s=res-m1.p+1;
                        break;
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
        struct fvar_struct u;
        u.p = 0;
        feval(&u, s, l, r-6);
        fmakesure(!u.num&&u.p, 3);
        p->num=true;
        p->s = strlen(u.p);
        return;
    }
    if(s[l]=='!') {
        struct fvar_struct m;
        m.p = 0;
        feval(&m, s, l+1, r);
        fmakesure(m.num,1);
        p->num=true;
        p->s=!m.s;
        return;
    }
    if(!strncmp(s+l,"没有",4)) {
        struct fvar_struct m;
        m.p = 0;
        feval(&m, s, l+4, r);
        fmakesure(m.num,1);
        p->num=true;
        p->s=!m.s;
        return;
    }
    fmakesure(r-l+2+libptr<libs+LIBLEN,4);
    strncpy(libptr, s+l, r-l+1);
    libptr[r-l+1]=0;
    p->num=false;
    p->p=libptr;
    libptr+=r-l+2;
}

int gen_super_filter_index2(char *index, struct fileheader* fileinfo, char * boardname, int isbm) {
    struct fileheader *ptr1;
    struct flock ldata, ldata2;
    int fd, fd2, size = sizeof(fileheader), total, i, count = 0;
    char direct[PATHLEN];
    char newdirect[PATHLEN];
    char *ptr;
    struct stat buf;
    int load_content=0, found=0, load_stat=0;
    int gid = fileinfo->groupid;

    //TODO: 这么大的index!
    load_content = (strstr(index, "content")!=NULL||strstr(index, "文章内容")!=NULL);
    load_stat = (strstr(index, "asize")!=NULL||strstr(index, "总长度")!=NULL);
    setbdir(DIR_MODE_NORMAL, direct, boardname);
    setbdir(DIR_MODE_SUPERFITER, newdirect, boardname);
    if ((fd = open(newdirect, O_WRONLY | O_CREAT, 0664)) == -1) {
        bbslog("user", "%s", "recopen err");
        return -9999;
    }
    ldata.l_type = F_WRLCK;
    ldata.l_whence = 0;
    ldata.l_len = 0;
    ldata.l_start = 0;
    if (fcntl(fd, F_SETLKW, &ldata) == -1) {
        bbslog("user", "%s", "reclock err");
        close(fd);
        return -9999;
    }

    if ((fd2 = open(direct, O_RDONLY, 0664)) == -1) {
        bbslog("user", "%s", "recopen err");
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
        return -9999;
    }
    fstat(fd2, &buf);
    ldata2.l_type = F_RDLCK;
    ldata2.l_whence = 0;
    ldata2.l_len = 0;
    ldata2.l_start = 0;
    fcntl(fd2, F_SETLKW, &ldata2);
    total = buf.st_size / size;

    if ((i = safe_mmapfile_handle(fd2, PROT_READ, MAP_SHARED, (void **) &ptr, &buf.st_size)) != 1) {
        if (i == 2)
            end_mmapfile((void *) ptr, buf.st_size, -1);
        ldata2.l_type = F_UNLCK;
        fcntl(fd2, F_SETLKW, &ldata2);
        close(fd2);
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
        return -9999;
    }
    ptr1 = (struct fileheader *) ptr;
    libs = (char*)malloc(LIBLEN);
    for (i = 0; i < total; i++) {
        struct stat st;
        char* p;
        char ffn[80];
        int j=0;
        off_t fsize;
        libptr = libs;
        ferr = 0;
        
        set_vard(fvars+fget_var("cid"), fileinfo->id);
        set_vard(fvars+fget_var("creid"), fileinfo->reid);
        set_vard(fvars+fget_var("cgroupid"), fileinfo->groupid);
        set_vars(fvars+fget_var("cauthor"), fileinfo->owner);
        set_vars(fvars+fget_var("cfname"), fileinfo->filename);
        set_vard(fvars+fget_var("cftime"), get_posttime(fileinfo));
        set_vard(fvars+fget_var("ceffsize"), fileinfo->eff_size);
        
        set_vard(fvars+fget_var("no"), i+1); set_vard(fvars+fget_var("文章号"), i+1);
        set_vard(fvars+fget_var("id"), ptr1->id);
        set_vard(fvars+fget_var("reid"), ptr1->reid);
        set_vard(fvars+fget_var("groupid"), ptr1->groupid);
        set_vard(fvars+fget_var("thread"), ptr1->groupid==gid); set_vard(fvars+fget_var("本主题"), ptr1->groupid==gid);
        set_vard(fvars+fget_var("origin"), ptr1->id==ptr1->groupid); set_vard(fvars+fget_var("原作"), ptr1->id==ptr1->groupid);
        set_vard(fvars+fget_var("m"), ptr1->accessed[0]&FILE_MARKED); set_vard(fvars+fget_var("保留"), ptr1->accessed[0]&FILE_MARKED);
        set_vard(fvars+fget_var("g"), ptr1->accessed[0]&FILE_DIGEST); set_vard(fvars+fget_var("文摘"), ptr1->accessed[0]&FILE_DIGEST);
        set_vard(fvars+fget_var("b"), (ptr1->accessed[0]&FILE_MARKED)&&(ptr1->accessed[0]&FILE_DIGEST));
        if (isbm) {
            set_vard(fvars+fget_var("noreply"), ptr1->accessed[1]&FILE_READ); set_vard(fvars+fget_var("不可回复"), ptr1->accessed[1]&FILE_READ);
            set_vard(fvars+fget_var("sign"), ptr1->accessed[0]&FILE_SIGN); set_vard(fvars+fget_var("标记"), ptr1->accessed[0]&FILE_SIGN);
#ifdef PERCENT_SIGN_SUPPORT
	     set_vard(fvars+fget_var("percent"), ptr1->accessed[0]&FILE_PERCENT); set_vard(fvars+fget_var("百分号"), ptr1->accessed[0]&FILE_PERCENT);
#endif
#ifdef FILTER
            set_vard(fvars+fget_var("censor"), ptr1->accessed[1]&FILE_CENSOR); set_vard(fvars+fget_var("审核"), ptr1->accessed[1]&FILE_CENSOR);
#endif
            set_vard(fvars+fget_var("del"), ptr1->accessed[1]&FILE_DEL); set_vard(fvars+fget_var("删除"), ptr1->accessed[1]&FILE_DEL);
            set_vard(fvars+fget_var("import"), ptr1->accessed[0]&FILE_IMPORTED); set_vard(fvars+fget_var("精华"), ptr1->accessed[0]&FILE_IMPORTED);
        }
        set_vard(fvars+fget_var("attach"), ptr1->attachment); set_vard(fvars+fget_var("附件"), ptr1->attachment);
        set_vars(fvars+fget_var("title"), ptr1->title); set_vars(fvars+fget_var("标题"), ptr1->title);
        set_vars(fvars+fget_var("author"), ptr1->owner); set_vars(fvars+fget_var("作者"), ptr1->owner);
        set_vars(fvars+fget_var("fname"), ptr1->filename); set_vars(fvars+fget_var("文件名"), ptr1->filename);
        set_vard(fvars+fget_var("my"), !strcmp(ptr1->owner,getCurrentUser()->userid)); set_vard(fvars+fget_var("我的"), !strcmp(ptr1->owner,getCurrentUser()->userid));
#ifdef HAVE_BRC_CONTROL
        set_vard(fvars+fget_var("unread"), brc_unread(ptr1->id, getSession())); set_vard(fvars+fget_var("未读"), brc_unread(ptr1->id, getSession()));
#endif
        setbfile(ffn, boardname, ptr1->filename);
        set_vard(fvars+fget_var("ftime"), get_posttime(ptr1)); set_vard(fvars+fget_var("时间"), get_posttime(ptr1));
        set_vard(fvars+fget_var("effsize"), ptr1->eff_size); set_vard(fvars+fget_var("有效长度"), ptr1->eff_size);
        if(load_stat) {
            if(stat(ffn, &st)!=-1) {
                set_vard(fvars+fget_var("asize"), st.st_size); set_vard(fvars+fget_var("总长度"), st.st_size);
            }
            else {
                set_vard(fvars+fget_var("asize"), 0); set_vard(fvars+fget_var("总长度"), 0);
            }
        }
        if(load_content) {
            set_vars(fvars+fget_var("content"), ptr1->filename);
            set_vars(fvars+fget_var("文章内容"), ptr1->filename);
            j = safe_mmapfile(ffn, O_RDONLY, PROT_READ, MAP_SHARED, (void **) &p, &fsize, NULL);
            if(j) {
                set_vars(fvars+fget_var("content"), p);
                set_vars(fvars+fget_var("文章内容"), p);
            }
        }
        ferr=0;
        feval(fvars+fget_var("res"), index, 0, strlen(index)-1);
        if(ferr) break;
        if(fvars[fget_var("res")].s) {
            write(fd, ptr1, size);
            count++;
            found++;
        }
        if(load_content) {
            if(j)
                end_mmapfile((void*)p, fsize, -1);
        }
        ptr1++;
    }
    free(libs);
    end_mmapfile((void *) ptr, buf.st_size, -1);
    ldata2.l_type = F_UNLCK;
    fcntl(fd2, F_SETLKW, &ldata2);
    close(fd2);
    ftruncate(fd, count * size);

    ldata.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &ldata);        /* 退出互斥区域*/
    close(fd);
    if(ferr) {
        return -ferr;
    }
    else {
        return count;
    }
}