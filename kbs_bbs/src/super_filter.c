#include "bbs.h"
#include <math.h>

#define MAX_FVAR 100
#define LIBLEN 1000

int ferr=0;

struct fvar_struct {
    bool num;
    char name[8];
    int s;
    char * p;
};

struct fvar_struct fvars[MAX_FVAR];
int fvart = 0;
char * libs, * libptr;

#define fmakesure(o,p) if(ferr) return;\
    if(!(o)) {ferr=p; return;}

extern char * currdirect;

int fget_var(char * name)
{
    int i;
    if(ferr) return 0;
    if(!name[0]||strlen(name)>6) {
        ferr=14;
        return 0;
    }
    for(i=0;i<fvart;i++)
        if(!strncasecmp(fvars[i].name, name, 8)) {
            return i;
        }
    if(fvart>=MAX_FVAR) {
        ferr=15;
        return 0;
    }
    strncpy(fvars[fvart].name, name, 8);
    fvars[fvart].p = 0;
    fvars[fvart].num = true;
    fvart++;
    return (fvart-1);
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
        p=p&&(isalpha(s[i]));
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
    char op[12][4]={"&&","||","==","!=",">","<",">=","<=","+","-","*","/"};
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
        i=fget_var(buf);
        p->num = fvars[i].num;
        p->s = fvars[i].s;
        p->p = fvars[i].p;
        return;
    }
    if(check_var_int(s+l, r-l+1)) {
        int f;
        strncpy(buf, s+l, 1000);
        buf[r-l+1]=0;
        f = atoi(buf);
        set_vard(p, f);
        return;
    }
    if(s[l]=='\''&&s[r]=='\''&&get_rl2(s,r,l)==l) {
        fmakesure(r-l-2+libptr<libs+LIBLEN,4);
        strncpy(libptr, s+l+1, r-l-2);
        libptr[r-l-2]=0;
        p->num=false;
        p->p=libptr;
        libptr+=r-l-1;
        return;
    }
    i=l;
    while(isalpha(s[i])&&i<=r) i++;
    if(i>l&&s[i]=='('&&s[r]==')'&&get_rl(s,r,l)==i) {
        struct fvar_struct u,v;
        u.p=0; v.p=0;
        strncpy(buf, s+l, 1000);
        buf[i-l]=0;
        if(!strncmp("sub",buf,4)) {
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
        }
        else if(!strncmp("len",buf,4)){
            feval(&u, s, i+1, r-1);
            fmakesure(!u.num&&u.p, 3);
            p->num=true;
            p->s = strlen(u.p);
        }
        ferr=18;
        return ;
    }
    for(j=0;j<12;j++) {
        n=r;
        do{
            if(toupper(s[n])==op[j][0]&&(!op[j][1]||toupper(s[n+1])==op[j][1])) {
                struct fvar_struct m1,m2,m3;
                m1.p=0; m2.p=0; m3.p=0;
                feval(&m1,s,l,n-1);
                if(j==2||j==3) {fmakesure(m1.num||!m1.num&&m1.p,1);}
                else {fmakesure(m1.num,1);}
                if(op[j][1]==0) feval(&m2,s,n+1,r);
                else feval(&m2,s,n+2,r);
                if(j==2||j==3) {fmakesure(m1.num&&m2.num||!m1.num&&!m2.num&&m2.p,1);}
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
                        p->s=m1.s>m2.s;
                        break;
                    case 5:
                        p->s=m1.s<m2.s;
                        break;
                    case 6:
                        p->s=m1.s>=m2.s;
                        break;
                    case 7:
                        p->s=m1.s<=m2.s;
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
    fmakesure(r-l+libptr<libs+LIBLEN,4);
    strncpy(libptr, s+l+1, r-l);
    libptr[r-l]=0;
    p->num=false;
    p->p=libptr;
    libptr+=r-l+1;
}

int super_filter(int ent, struct fileheader *fileinfo, char *direct)
{
    struct fileheader *ptr1;
    struct flock ldata, ldata2;
    int fd, fd2, size = sizeof(fileheader), total, i, count = 0;
    char olddirect[PATHLEN];
    char *ptr;
    struct stat buf;
    int mode=6, load_content=0;
    static char index[1024]="";

    clear();
    prints("                  超强文章选择\n\n");
    multi_getdata(2, 0, "请输入表达式: ", index, 1020, 20, 0);
    if(!index[0]) 
        return FULLUPDATE;
    load_content = (strstr(index, "content")!=NULL);
    if (digestmode > 0) {
        if (digestmode == 7 || digestmode == 8)
            unlink(currdirect);
        digestmode = 0;
        setbdir(digestmode, currdirect, currboard->filename);
    }
    digestmode = 0;
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
    /* 开始互斥过程*/

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
        libptr = libs;
        set_vard(fvars+fget_var("no"), i+1);
        set_vard(fvars+fget_var("id"), ptr1->id);
        set_vard(fvars+fget_var("reid"), ptr1->reid);
        set_vard(fvars+fget_var("groupid"), ptr1->groupid);
        set_vard(fvars+fget_var("m"), ptr1->accessed[0]&FILE_MARKED);
        set_vard(fvars+fget_var("g"), ptr1->accessed[0]&FILE_DIGEST);
        set_vard(fvars+fget_var("b"), (ptr1->accessed[0]&FILE_MARKED)&&(ptr1->accessed[0]&FILE_DIGEST));
        set_vard(fvars+fget_var("noreply"), ptr1->accessed[1]&FILE_READ);
        set_vard(fvars+fget_var("sign"), ptr1->accessed[0]&FILE_SIGN);
#ifdef FILTER
        set_vard(fvars+fget_var("censor"), ptr1->accessed[1]&FILE_CENSOR);
#endif
        set_vard(fvars+fget_var("del"), ptr1->accessed[1]&FILE_DEL);
        set_vard(fvars+fget_var("import"), ptr1->accessed[0]&FILE_IMPORTED);
        set_vard(fvars+fget_var("attach"), ptr1->attachment);
        set_vars(fvars+fget_var("title"), ptr1->title);
        set_vars(fvars+fget_var("author"), ptr1->owner);
        if(load_content) {
        }
        ferr=0;
        feval(fvars+fget_var("res"), index, 0, strlen(index)-1);
        if(ferr) break;
        if(fvars[fget_var("res")].s) {
            write(fd, ptr1, size);
            count++;
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
    return NEWDIRECT;
}

