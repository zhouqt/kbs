#include "bbs.h"
#include <math.h>

#define MAX_FVAR 100
#define LIBLEN 1000

int ferr=0;

struct fvar_struct {
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

extern struct boardheader* currboard;
extern char currdirect[255];
extern int digestmode;

int fget_var(char * name)
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

int fexist_var(char * name)
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
    return -1;
}

void set_vard(struct fvar_struct * a, int f)
{
    a->num = true;
    a->s = f;
}

void set_vars(struct fvar_struct * a, char * f)
{
    a->num = false;
    a->p = f;
}

int fcheck_var_name(char * s, int l)
{
    int i,p=1;
    for(i=0;i<l;i++)
        p=p&&(isalpha(s[i])||s[i]<0);
    return p;
}

int check_var_int(char * s, int l)
{
    int i,p=1;
    for(i=0;i<l;i++)
        p=p&&(isdigit(s[i]));
    return p;
}

int get_rl(char * s, int r, int l)
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

int get_rl2(char * s, int r, int l)
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

void feval(struct fvar_struct * p, char * s, int l, int r)
{
    int i,j,n;
    char op[14][4]={"&&","||","==","!=",">=","<=",">","<","+","-","*","/","%","<<"};
    char op2[14][10]={"且","或","是","不是","大等于","小等于","大于","小于","加","减","乘","除","模", "包含"};
    struct fvar_struct * t,q;
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
            char * res;
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
            char * res;
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
            struct tm t;
            char * res;
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
    if (r-l>=6&&!strncmp("的长度", s+r-5, 6)) {
        struct fvar_struct u;
        u.p = 0;
        feval(&u, s, l, r-6);
        fmakesure(!u.num&&u.p, 3);
        p->num=true;
        p->s = strlen(u.p);
        return;
    }
    for(j=0;j<14;j++) {
        n=r;
        do{
            if(n+strlen(op[j])<=r&&!strncmp(s+n, op[j], strlen(op[j]))||n+strlen(op2[j])<=r&&!strncmp(s+n, op2[j], strlen(op2[j]))) {
                struct fvar_struct m1,m2,m3;
                char * res;
                m1.p=0; m2.p=0; m3.p=0;
                feval(&m1,s,l,n-1);
                if(j==2||j==3) {fmakesure(m1.num||!m1.num&&m1.p,1);}
                else if(j==13) {fmakesure(!m1.num,1);}
                else {fmakesure(m1.num,1);}
                if(!strncmp(s+n, op[j], strlen(op[j])))
                    feval(&m2,s,n+strlen(op[j]),r);
                else
                    feval(&m2,s,n+strlen(op2[j]),r);
                if(j==2||j==3) {fmakesure(m1.num&&m2.num||!m1.num&&!m2.num&&m2.p,1);}
                else if(j==13) {fmakesure(!m2.num,1);}
                else {fmakesure(m2.num,1);}
                p->num=true;
                switch(j) {
                    case 0:
                        p->s=m1.s&&m2.s;
                        break;
                    case 1:
                        p->s=m1.s||m2.s;
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
            n--;
        }while(n>=l);
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
    fmakesure(r-l+2+libptr<libs+LIBLEN,4);
    strncpy(libptr, s+l, r-l+1);
    libptr[r-l+1]=0;
    p->num=false;
    p->p=libptr;
    libptr+=r-l+2;
}

int super_filter(int ent, struct fileheader *fileinfo, char *direct)
{
    struct fileheader *ptr1;
    struct flock ldata, ldata2;
    int fd, fd2, size = sizeof(fileheader), total, i, count = 0;
    char olddirect[PATHLEN];
    char *ptr;
    struct stat buf;
    int mode=8, load_content=0, found=0, load_stat=0;
    int gid = fileinfo->groupid;
    extern int scr_cols;
    static char index[1024]="";

    clear();
    prints("                  超强文章选择\n\n");
    move(5,0);
    prints("变量: no(文章号) m(保留) g(文摘) b(m&&g) noreply(不可回复) sign(标记)\n"
           "      del(删除) attach(附件) unread(未读)\n"
           "      title(标题) author(作者)\n"
           "函数: sub(s1,s2)第一个字符串在第二个中的位置,如果不存在返回0\n"
           "      len(s)字符串长度\n"
           "举例: 我要查询所有bad写的标记是b的文章:\n"
           "              author=='bad'&&b\n"
           "      我要查询所有不可回复并且未读的文章:\n"
           "              noreply&&unread\n"
           "      我要查询所有1000-2000范围内带附件的文章:\n"
           "              (no>=1000)&&(no<=2000)&&attach\n"
           "      我要查询标题长度在5-10之间的文章:\n"
           "              len(title)>=5&&len(title)<=10\n"
           "      我要查询标题里含有faint的文章:\n"
           "              sub('faint',title)\n"
           "      我要查询标题里包含hehe并且位置在最后的文章:\n"
           "              sub('hehe',title)==len(title)-3\n"
           "      我要查询......自己动手查吧,hehe"
    );
    multi_getdata(2, 0, scr_cols-1, "请输入表达式: ", index, 1020, 20, 0, 0);
    if(!index[0]) 
        return FULLUPDATE;
    load_content = (strstr(index, "content")!=NULL);
    load_stat = (strstr(index, "asize")!=NULL);
    if (digestmode==7||digestmode==8 ) {
        if (digestmode == 7 || digestmode == 8)
            unlink(currdirect);
        digestmode = 0;
        setbdir(digestmode, currdirect, currboard->filename);
    }
    setbdir(digestmode, olddirect, currboard->filename);
    digestmode = mode;
    setbdir(digestmode, currdirect, currboard->filename);
    if ((fd = open(currdirect, O_WRONLY | O_CREAT, 0664)) == -1) {
        bbslog("user", "%s", "recopen err");
        return FULLUPDATE;      /* 创建文件发生错误*/
    }
    ldata.l_type = F_WRLCK;
    ldata.l_whence = 0;
    ldata.l_len = 0;
    ldata.l_start = 0;
    if (fcntl(fd, F_SETLKW, &ldata) == -1) {
        bbslog("user", "%s", "reclock err");
        close(fd);
        return FULLUPDATE;      /* lock error*/
    }

    if ((fd2 = open(olddirect, O_RDONLY, 0664)) == -1) {
        bbslog("user", "%s", "recopen err");
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
        return FULLUPDATE;
    }
    fstat(fd2, &buf);
    ldata2.l_type = F_RDLCK;
    ldata2.l_whence = 0;
    ldata2.l_len = 0;
    ldata2.l_start = 0;
    fcntl(fd2, F_SETLKW, &ldata2);
    total = buf.st_size / size;

    if ((i = safe_mmapfile_handle(fd2, O_RDONLY, PROT_READ, MAP_SHARED, (void **) &ptr, (size_t*)&buf.st_size)) != 1) {
        if (i == 2)
            end_mmapfile((void *) ptr, buf.st_size, -1);
        ldata2.l_type = F_UNLCK;
        fcntl(fd2, F_SETLKW, &ldata2);
        close(fd2);
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
        return FULLUPDATE;
    }
    ptr1 = (struct fileheader *) ptr;
    libs = (char*)malloc(LIBLEN);
    for (i = 0; i < total; i++) {
        struct stat st;
        char* p;
        char ffn[80];
        int j;
        size_t fsize;
        libptr = libs;
        ferr = 0;
        set_vard(fvars+fget_var("no"), i+1); set_vard(fvars+fget_var("文章号"), i+1);
        set_vard(fvars+fget_var("id"), ptr1->id);
        set_vard(fvars+fget_var("reid"), ptr1->reid);
        set_vard(fvars+fget_var("groupid"), ptr1->groupid);
        set_vard(fvars+fget_var("thread"), ptr1->groupid==gid); set_vard(fvars+fget_var("本主题"), ptr1->groupid==gid);
        set_vard(fvars+fget_var("origin"), ptr1->id==ptr1->groupid); set_vard(fvars+fget_var("原作"), ptr1->id==ptr1->groupid);
        set_vard(fvars+fget_var("m"), ptr1->accessed[0]&FILE_MARKED); set_vard(fvars+fget_var("保留"), ptr1->accessed[0]&FILE_MARKED);
        set_vard(fvars+fget_var("g"), ptr1->accessed[0]&FILE_DIGEST); set_vard(fvars+fget_var("文摘"), ptr1->accessed[0]&FILE_DIGEST);
        set_vard(fvars+fget_var("b"), (ptr1->accessed[0]&FILE_MARKED)&&(ptr1->accessed[0]&FILE_DIGEST));
        if (chk_currBM(currBM, currentuser)) {
            set_vard(fvars+fget_var("noreply"), ptr1->accessed[1]&FILE_READ); set_vard(fvars+fget_var("不可回复"), ptr1->accessed[1]&FILE_READ);
            set_vard(fvars+fget_var("sign"), ptr1->accessed[0]&FILE_SIGN); set_vard(fvars+fget_var("标记"), ptr1->accessed[0]&FILE_SIGN);
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
        set_vard(fvars+fget_var("my"), !strcmp(ptr1->owner,currentuser->userid)); set_vard(fvars+fget_var("我的"), !strcmp(ptr1->owner,currentuser->userid));
#ifdef HAVE_BRC_CONTROL
        set_vard(fvars+fget_var("unread"), brc_unread(ptr1->id)); set_vard(fvars+fget_var("未读"), brc_unread(ptr1->id));
#endif
        setbfile(ffn, currboard, ptr1->filename);
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
            int k,abssize=0,entercount=0,ignoreline=0;
            set_vars(fvars+fget_var("content"), ptr1->filename);
            set_vars(fvars+fget_var("内容"), ptr1->filename);
            j = safe_mmapfile(ffn, O_RDONLY, PROT_READ, MAP_SHARED, (void **) &p, &fsize, NULL);
            if(j) {
                set_vars(fvars+fget_var("content"), p);
                set_vars(fvars+fget_var("内容"), p);
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
        move(3, 0);
        clrtoeol();
        prints("表达式错误");
        refresh();
        sleep(1);
    }
    else if(count==0) {
        move(3, 0);
        clrtoeol();
        prints("一个都没有找到....");
        refresh();
        sleep(1);
    }
/*    else if (chk_currBM(currBM, currentuser)) {
        char ans[4];
        int i,j,k;
        int fflag;
        int y,x;
        move(3, 0);
        clrtoeol();
        prints("找到 %d 篇文章(0-退出, 1-保留标记m, 2-删除标记t, 3-不可回复标记;) [0]", count);
        getyx(&y, &x);
        getdata(y, x, 0, ans, 3, 1, 0, 1);
        if(ans[0]>='1'&&ans[0]<='3') {
            struct fileheader f;
            k=ans[0]-'0';
            if(ans[0]=='1') fflag=FILE_MARK_FLAG;
            else if(ans[0]=='2') fflag=FILE_DELETE_FLAG;
            else if(ans[0]=='3') fflag=FILE_NOREPLY_FLAG;
            for(i=0;i<count;i++)
                change_post_flag(currBM, currentuser, digestmode, currboard, i+1, &f, currdirect, fflag, 0);
        }
    }*/
    return NEWDIRECT;
}

