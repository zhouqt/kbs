/******************************************************
超级计算器2003, 作者: bad@smth.org  Qian Wenjie
在水木清华bbs系统上运行

本游戏是自由软件，请随意复制移植
请在修改后的文件头部保留版权信息
******************************************************/

#define BBSMAIN
//#include "bbs.h"
#include <math.h>
#include <stdio.h>

#define MINIMUM	0.0000000001
#define sqr(x)	(x)*(x)
#define Pi		3.1415926535897932384626433832795

int err=0;

struct var_struct {
    char name[8];
    int height,width;
    double **p;
};

struct var_struct vars[100];
int vart = 0;

#define makesure(o) if(err) return;\
    if(!(o)) {err=1; return;}

void makesize(struct var_struct * a, int h, int w)
{
    int i;
    if(a->p) {
        for(i=0;i<a->height;i++)
            free(a->p[i]);
        free(a->p);
    }
    a->p = (double**)malloc(sizeof(double*)*h);
    for(i=0;i<h;i++)
        a->p[i] = (double*)malloc(sizeof(double)*w);
    a->height = h;
    a->width = w;
}

int is_single_var(struct var_struct * a)
{
    return (a->height==1&&a->width==1);
}

void copy_var(struct var_struct * a, struct var_struct * b)
{
    int i,j;
    makesize(b, a->height, a->width);
    for(i=0;i<a->height;i++)
        for(j=0;j<a->width;j++)
            b->p[i][j]=a->p[i][j];
}

void add_var(struct var_struct * a, struct var_struct * b, struct var_struct * c)
{
    int i,j;
    makesure(a->height==b->height);
    makesure(a->width==b->width);
    makesize(c, a->height, a->width);
    for(i=0;i<a->height;i++)
        for(j=0;j<a->width;j++)
            c->p[i][j]=a->p[i][j]+b->p[i][j];
}

void sub_var(struct var_struct * a, struct var_struct * b, struct var_struct * c)
{
    int i,j;
    makesure(a->height==b->height);
    makesure(a->width==b->width);
    makesize(c, a->height, a->width);
    for(i=0;i<a->height;i++)
        for(j=0;j<a->width;j++)
            c->p[i][j]=a->p[i][j]-b->p[i][j];
}

void setzero(struct var_struct * a)
{
    int i,j;
    for(i=0;i<a->height;i++)
        for(j=0;j<a->width;j++)
            a->p[i][j]=0;
}

void setunit(struct var_struct * a)
{
    int i,j;
    makesure(a->height==a->width);
    for(i=0;i<a->height;i++)
        for(j=0;j<a->width;j++)
            a->p[i][j]=(i==j);
}

void swaprow(struct var_struct * s, int a, int b)
{
    int i;
    double temp;
    makesure(a<s->height&&a>=0);
    makesure(b<s->height&&b>=0);
    for(i=0;i<s->width;i++) {
        temp = s->p[a][i];
        s->p[a][i] = s->p[b][i];
        s->p[b][i] = temp;
    }
}

void swapcol(struct var_struct * s, int a, int b)
{
    int i;
    double temp;
    makesure(a<s->width&&a>=0);
    makesure(b<s->width&&b>=0);
    for(i=0;i<s->height;i++) {
        temp = s->p[i][a];
        s->p[i][a] = s->p[i][b];
        s->p[i][b] = temp;
    }
}

void plusrow(struct var_struct * s, int a, int b, double r)
{
    int i;
    makesure(a<s->height&&a>=0);
    makesure(b<s->height&&b>=0);
    for(i=0;i<s->width;i++) {
        s->p[b][i] += s->p[a][i]*r;
    }
}

void pluscol(struct var_struct * s, int a, int b, double r)
{
    int i;
    makesure(a<s->width&&a>=0);
    makesure(b<s->width&&b>=0);
    for(i=0;i<s->height;i++) {
        s->p[i][b] += s->p[i][a]*r;
    }
}

void multrow(struct var_struct * s, int a, double r)
{
    int i;
    makesure(a<s->height&&a>=0);
    for(i=0;i<s->width;i++) {
        s->p[a][i] *= r;
    }
}

void multcol(struct var_struct * s, int a, double r)
{
    int i;
    makesure(a<s->width&&a>=0);
    for(i=0;i<s->height;i++) {
        s->p[i][a] *= r;
    }
}

int get_var(char * name)
{
    int i;
    for(i=0;i<vart;i++)
        if(!strcasecmp(vars[i].name, name)) {
            return i;
        }
    strcpy(vars[vart].name, name);
    vars[vart].p = 0;
    return (vart++);
}

void set_var(struct var_struct * a, double f)
{
    makesize(a, 1, 1);
    **(a->p) = f;
}

int check_var_name(char * s, int l)
{
    int i,p=1;
    for(i=0;i<l;i++)
        p==p&&(isalpha(s[i]));
    return p;
}

#define isdouble(c) (isdigit(c)||c=='.'||c=='-')

int check_var_double(char * s, int l)
{
    int i,p=1;
    for(i=0;i<l;i++)
        p==p&&(isdouble(s[i]));
    return p;
}

void set_matrix(struct var_struct * p, char * s)
{
    char buf[80];
    int i,j,k,height=1,width=0;
    for(i=0;i<strlen(s);i++)
        if(s[i]==';') height++;
    j=0;
    for(i=0;i<strlen(s);i++) {
        if(s[i]==';') break;
        if(isdouble(s[i])) {
            if(j==0) width++;
            j=1;
        }
        else j=0;
    }
    makesize(p, height, width);
    for(i=0;i<height;i++)
        for(j=0;j<width;j++) {
            while(*s&&!isdouble(*s)) s++;
            if(!*s) {
                err=1;
                return;
            }
            k=0;
            while(isdouble(s[k])) k++;
            strcpy(buf, s);
            buf[k]=0;
            s+=k;
            p->p[i][j]=atof(buf);
        }
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
        if(s[n]==']') i++;
        if(s[n]=='[') i--;
        n--;
    }while(i!=0&&n>=l);
    if(i==0) return n+1;
    else return -1;
}

void eval(struct var_struct * p, char * s, int l, int r)
{
    int i,j,n;
    char op[5]="+-*/^";
    struct var_struct * t,q;
    char buf[300];
    while(s[l]==' '&&l<=r) l++;
    while(s[r]==' '&&l<=r) r--;
    makesure(l<=r);
    while(s[l]=='('&&s[r]==')'&&get_rl(s,r,l)==l) {
        l++; r--;
        while(s[l]==' '&&l<=r) l++;
        while(s[r]==' '&&l<=r) r--;
    }
    if(check_var_name(s+l, r-l+1)||s[l]=='%'&&check_var_name(s+l+1, r-l)) {
        strcpy(buf, s+l);
        buf[r-l+1]=0;
        i=get_var(buf);
        copy_var(vars+i, p);
        return;
    }
    if(check_var_double(s+l, r-l+1)) {
        strcpy(buf, s+l);
        buf[r-l+1]=0;
        set_var(p, atof(buf));
        return;
    }
    if(s[l]=='['&&s[r]==']'&&get_rl2(s,r,l)==l) {
        strcpy(buf, s+l+1);
        buf[r-l-1]=0;
        set_matrix(p, buf);
        return;
    }
    i=l;
    while(isalpha(s[i])&&i<=r) i++;
    if(i>l&&s[i]=='('&&s[r]==')'&&get_rl(s,r,l)==i) {
        strcpy(buf, s+l);
        buf[i-l]=0;
        struct var_struct u;
        u.p = 0;
        eval(&u, s, i+1, r-1);
        set_var(p, 0);
        return ;
    }
    for(j=0;j<5;j++) {
        n=r;
        do{
            if(s[n]==op[j]) {
                struct var_struct m1,m2;
                m1.p=0; m2.p=0;
                eval(&m1,s,l,n-1);
                eval(&m2,s,n+1,r);
                switch(j) {
                    case 0:
                        add_var(&m1, &m2, p);
                        break;
                    case 1:
                        add_var(&m1, &m2, p);
                        break;
                    case 2:
                        break;
                    case 3:
                        makesure(is_single_var(&m1)&&is_single_var(&m2));
                        makesure(fabs(**(m2.p))>MINIMUM);
                        set_var(p, (**(m1.p))/(**(m2.p)));
                        break;
                    case 4:
                        makesure(is_single_var(&m1)&&is_single_var(&m2));
                        makesure(fabs(**(m2.p))>MINIMUM);
                        set_var(p, exp(log(**(m1.p))*(**(m2.p))));
                        break;
                }
                return;
            }
            if(s[n]==')') n=get_rl(s,r,l);
            if(s[n]==']') n=get_rl2(s,r,l);
            n--;
        }while(n>=l);
    }
    err=1;
}

void print_var(struct var_struct * p)
{
    int i,j;
    printf("%s\n", p->name);
    printf("[");
    for(i=0;i<p->height;i++){
        for(j=0;j<p->width;j++) {
            printf("%lf", p->p[i][j]);
            if(j<p->width-1) printf(" ");
        }
        if(i==p->height-1) printf("]");
        printf("\n");
    }
}

int main()
{
    char cmd[300];
    int y,x,res,i,j;
    res = get_var("res");
    set_var(vars+get_var("%pi"), Pi);
    set_var(vars+get_var("%e"), 2.71828182846);
//    clear();
    while(1) {
        printf("> ");
//        getyx(&y, &x);
//        getdata(y, x, 0, cmd, 300, 1, 0, 1);
        fscanf("%s", cmd);
        if(!strcasecmp(cmd, "exit")) break;
        if(!strcasecmp(cmd, "quit")) break;
        if(strchr(cmd, '=')) {
            i=strchr(cmd, '=')-cmd;
            if(i<=1||!check_var_name(cmd, i-1)) {
                printf("error\n");
                continue;
            }
            cmd[i]=0;
            res = get_var(cmd);
            i++;
        }
        else {
            res = get_var("res");
            i=0;
        }
        eval(vars+res, cmd+i, 0, strlen(cmd+i)-1);
        print_var(vars+res);
    }
}
