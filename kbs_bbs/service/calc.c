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

void del(struct var_struct * a)
{
    int i;
    if(a->p) {
        for(i=0;i<a->height;i++)
            free(a->p[i]);
        free(a->p);
    }
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

void mul_var(struct var_struct * a, struct var_struct * b, struct var_struct * c)
{
    int i,j,k;
    makesure(a->width==b->height);
    makesize(c, a->height, b->width);
    for(i=0;i<a->height;i++)
        for(j=0;j<b->width;j++) {
            c->p[i][j]=0;
            for(k=0;k<a->width;k++)
                c->p[i][j]+=a->p[i][k]*b->p[k][j];
        }
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

void reverse(struct var_struct * s, struct var_struct * t)
{
    int i,j;
    makesize(t, s->width, s->height);
    for(i=0;i<s->width;i++)
        for(j=0;j<s->height;j++)
            t->p[i][j]=s->p[j][i];
}

double envalue(struct var_struct * s)
{
    int i,j,flag;
    double result,ration;
    struct var_struct temp;
    if(s->height!=s->width) {
        err=1;
        return 0;
    }
    temp.p = 0;  
    copy_var(s, &temp);
    result=1;
    for (i=0;i<temp.height;i++) {
        if (temp.p[i][i]==0) {
            flag=0;
            for (j=i+1;j<temp.height;j++) {
                if (temp.p[j][i]!=0) {
                    swaprow(temp,i,j);
                    result*=-1;
                    flag=1;
                    break;
                }
            }
        }
        if (flag==0) {
            del(&temp);
            return 0;
        }
        //保证对角线元素不为零
        for (j=i+1;j<temp.height;j++) {
            ration=-temp.p[j][i]/temp.p[i][i];
            plusrow(temp,i,j,ration);
        }
        result*=temp.p[i][i];
    }
    del(&temp);
    return result;
}

void inverse(struct var_struct * s, struct var_struct * A)
{
    int i,j;
    double ration;
    struct var_struct temp,tempA;
    makesure(A->height==A->width);
    temp.p = 0; tempA.p = 0;
    makesize(&temp, A->height, A->height);
    setunit(&temp);
    copy_var(A, &tempA);
    for (i=0;i<temp.height;i++) {
        if (tempA.p[i][i]==0) {
            for (j=i+1;j<temp.height;j++) {
                if (tempA.p[j][i]!=0) {
                    swaprow(&tempA,i,j);
                    swaprow(&temp,i,j);
                    break;
                }
            }
        }
        if(fabs(tempA.p[i][i])<MINIMUM) {
            del(&temp);
            del(&tempA);
            err=1;
            return;
        }
        //保证对角线元素不为零
        ration=1/tempA.p[i][i];
        multrow(&tempA,i,ration);
        multrow(&temp,i,ration);
        for (j=i+1;j<temp.height;j++) {
            ration=-tempA.p[j][i];
            plusrow(&tempA,i,j,ration);
            plusrow(&temp,i,j,ration);
        }
    }
    //现已变成对角线元素为1的上三角阵
    for (i=0;i<temp.height;i++) {
        for (j=0;j<i;j++) {
            ration=-tempA.p[j][i];
            plusrow(&tempA,i,j,ration);
            plusrow(&temp,i,j,ration);
        }
    }
    copy_var(s, &temp);
    del(&temp);
    del(&tempA);
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
    vart++;
    return (vart-1);
}

void set_var(struct var_struct * a, double f)
{
    makesize(a, 1, 1);
    (**(a->p)) = f;
}

int check_var_name(char * s, int l)
{
    int i,p=1;
    for(i=0;i<l;i++)
        p=p&&(isalpha(s[i]));
    return p;
}

#define isdouble(c) (isdigit(c)||c=='.'||c=='-')

int check_var_double(char * s, int l)
{
    int i,p=1;
    for(i=0;i<l;i++)
        p=p&&(isdouble(s[i]));
    return p;
}

void set_matrix(struct var_struct * p, char * s)
{
    char buf[80];
    double f;
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
            sscanf(buf, "%lf", &f);
            p->p[i][j]=f;
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

double factorial( double f )
{
    double x, fp=f, ff, prod=0.24197072451914334979783;
    while (fp<20.0) {
        fp += 1.0;
        prod *= fp;
    }

    ff = fp*fp;
    x = exp( (fp + 0.5) * (log(fp)-1)
      +(1.0/12.0 - (1.0/360.0 - (1.0/1260.0 - (1.0/1680.0
      -1.0/(ff*1188.0))/ff)/ff)/ff)/fp ) / prod;
    return(x);
}

char funcname[][8]=
{"sin","cos","tan","asin","acos","atan",
"log","exp","ln","fact",
"sinh","cosh","tanh","asinh","acosh","atanh",
"abs","sign","sqr","sqrt","round","floor","ceil",
"det","inv",
""};

void take_func(struct var_struct * p, struct var_struct * q, int kind)
{
    int i,j;
    makesure(1);
    if(kind<23) {
        makesize(p, q->height, q->width);
        for(i=0;i<q->height;i++)
            for(j=0;j<q->width;j++) {
                switch(kind) {
                    case 0: p->p[i][j]=sin(q->p[i][j]); break;
                    case 1: p->p[i][j]=cos(q->p[i][j]); break;
                    case 2: p->p[i][j]=tan(q->p[i][j]); break;
                    case 3: p->p[i][j]=asin(q->p[i][j]); break;
                    case 4: p->p[i][j]=acos(q->p[i][j]); break;
                    case 5: p->p[i][j]=atan(q->p[i][j]); break;
                    case 6: p->p[i][j]=log10(q->p[i][j]); break;
                    case 7: p->p[i][j]=exp(q->p[i][j]); break;
                    case 8: p->p[i][j]=log(q->p[i][j]); break;
                    case 9: p->p[i][j]=factorial(q->p[i][j]); break;
                    case 10: p->p[i][j]=sinh(q->p[i][j]); break;
                    case 11: p->p[i][j]=cosh(q->p[i][j]); break;
                    case 12: p->p[i][j]=tanh(q->p[i][j]); break;
                    case 13: p->p[i][j]=asinh(q->p[i][j]); break;
                    case 14: p->p[i][j]=acosh(q->p[i][j]); break;
                    case 15: p->p[i][j]=atanh(q->p[i][j]); break;
                    case 16: p->p[i][j]=fabs(q->p[i][j]); break;
                    case 17: p->p[i][j]=(q->p[i][j]>0)?1:((q->p[i][j]<0)?-1:0); break;
                    case 18: p->p[i][j]=(q->p[i][j])*(q->p[i][j]); break;
                    case 19: p->p[i][j]=sqrt(q->p[i][j]); break;
                    case 20: p->p[i][j]=rint(q->p[i][j]); break;
                    case 21: p->p[i][j]=floor(q->p[i][j]); break;
                    case 22: p->p[i][j]=ceil(q->p[i][j]); break;
                }
            }
    }
    else {
        switch(kind) {
            23: set_var(p, envalue(q)); break;
            24: inverse(p, q); break;
        }
    }
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
        double f;
        strcpy(buf, s+l);
        buf[r-l+1]=0;
        sscanf(buf, "%lf", &f);
        set_var(p, f);
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
        struct var_struct u;
        strcpy(buf, s+l);
        buf[i-l]=0;
        j=0;
        while(funcname[j][0]) {
            if(!strcasecmp(funcname[j],buf)) break;
            j++;
        }
        if(funcname[j][0]) {
            u.p = 0;
            eval(&u, s, i+1, r-1);
            makesure(1);
            take_func(p, &u, j);
            del(&u);
        }
        else
            err=1;
        return ;
    }
    for(j=0;j<5;j++) {
        n=r;
        do{
            if(s[n]==op[j]) {
                struct var_struct m1,m2;
                m1.p=0; m2.p=0;
                eval(&m1,s,l,n-1);
                makesure(1);
                eval(&m2,s,n+1,r);
                makesure(1);
                switch(j) {
                    case 0:
                        add_var(&m1, &m2, p);
                        break;
                    case 1:
                        add_var(&m1, &m2, p);
                        break;
                    case 2:
                        mul_var(&m1, &m2, p);
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
                del(&m1);
                del(&m2);
                return;
            }
            if(s[n]==')') n=get_rl(s,r,l);
            if(s[n]==']') n=get_rl2(s,r,l);
            n--;
        }while(n>=l);
    }
    if(s[r]=='\'') {
        struct var_struct m;
        m.p = 0;
        eval(&m, s, l, r-1);
        makesure(1);
        reverse(p, &m);
        del(&m);
        return;
    }
    err=1;
}

void print_var(struct var_struct * p)
{
    int i,j;
    printf("%s =\n", p->name);
    if(!p->p) {
        printf("null\n");
        return;
    }
    else if(is_single_var(p)) {
        printf("%lf\n", **(p->p));
        return;
    }
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
        gets(cmd);
//        scanf("%s", cmd);
        if(!strcasecmp(cmd, "exit")) break;
        if(!strcasecmp(cmd, "quit")) break;
        if(strchr(cmd, '=')) {
            i=strchr(cmd, '=')-cmd;
            if(i<=0||!check_var_name(cmd, i)) {
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
        if(err) {
            printf("error\n");
            err=0;
            continue;
        }
        else
            print_var(vars+res);
    }
}
