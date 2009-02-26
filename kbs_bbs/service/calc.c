/******************************************************
超级计算器2003, 作者: bad@smth.org  Qian Wenjie
在水木清华bbs系统上运行

本游戏是自由软件，请随意复制移植
请在修改后的文件头部保留版权信息
******************************************************/

#include "service.h"
#include "bbs.h"
#include <math.h>

SMTH_API struct user_info uinfo;
SMTH_API bool UPDOWN;
SMTH_API int scr_cols, scr_lns;

#define MINIMUM 0.00001
#define Pi  3.1415926535897932384626433832795

#define MAX_VAR 100
#define MAX_MEM 10000

int calcerr=0;

struct var_struct {
    char name[8];
    int height,width;
    double **p;
};

struct var_struct vars[MAX_VAR];
int vart = 0;

#define makesure(o,p) if(calcerr) return;\
    if(!(o)) {calcerr=p; return;}

void makesize(struct var_struct * a, int h, int w)
{
    int i;
    makesure(w*h<=MAX_MEM,2);
    if (a->p) {
        for (i=0;i<a->height;i++)
            free(a->p[i]);
        free(a->p);
    }
    a->p = (double**)malloc(sizeof(double*)*h);
    for (i=0;i<h;i++)
        a->p[i] = (double*)malloc(sizeof(double)*w);
    a->height = h;
    a->width = w;
}

void del(struct var_struct * a)
{
    int i;
    makesure(1,0);
    if (a->p) {
        for (i=0;i<a->height;i++)
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
    makesure(1,0);
    if (a==b) return;
    makesize(b, a->height, a->width);
    makesure(1,0);
    for (i=0;i<a->height;i++)
        for (j=0;j<a->width;j++)
            b->p[i][j]=a->p[i][j];
}

void link_h(struct var_struct * a, struct var_struct * b, struct var_struct * c)
{
    int i,j;
    makesure(a->width==b->width,3);
    makesize(c, a->height+b->height, a->width);
    makesure(1,0);
    for (i=0;i<a->height;i++)
        for (j=0;j<a->width;j++)
            c->p[i][j]=a->p[i][j];
    for (i=0;i<b->height;i++)
        for (j=0;j<b->width;j++)
            c->p[i+a->height][j]=b->p[i][j];
}

void link_w(struct var_struct * a, struct var_struct * b, struct var_struct * c)
{
    int i,j;
    makesure(a->height==b->height,4);
    makesize(c, a->height, a->width+b->width);
    makesure(1,0);
    for (i=0;i<a->height;i++)
        for (j=0;j<a->width;j++)
            c->p[i][j]=a->p[i][j];
    for (i=0;i<b->height;i++)
        for (j=0;j<b->width;j++)
            c->p[i][j+a->width]=b->p[i][j];
}

void domatrix(struct var_struct * a, struct var_struct * b, struct var_struct * c, int op)
{
    int i,j;
    makesure((a->height==b->height&&a->width==b->width),5);
    makesize(c, a->height, a->width);
    makesure(1,0);
    for (i=0;i<a->height;i++)
        for (j=0;j<a->width;j++) {
            switch (op) {
                case 4: c->p[i][j]=a->p[i][j]*b->p[i][j]; break;
                case 5: makesure(fabs(b->p[i][j])>MINIMUM,6); c->p[i][j]=a->p[i][j]/b->p[i][j]; break;
                case 6: makesure(a->p[i][j]>0,7); c->p[i][j]=exp(log(a->p[i][j])*b->p[i][j]); break;
            }
        }
}

void add_var(struct var_struct * a, struct var_struct * b, struct var_struct * c)
{
    int i,j;
    makesure((a->height==b->height)&&(a->width==b->width),5);
    makesize(c, a->height, a->width);
    makesure(1,0);
    for (i=0;i<a->height;i++)
        for (j=0;j<a->width;j++)
            c->p[i][j]=a->p[i][j]+b->p[i][j];
}

void sub_var(struct var_struct * a, struct var_struct * b, struct var_struct * c)
{
    int i,j;
    makesure((a->height==b->height)&&(a->width==b->width),5);
    makesize(c, a->height, a->width);
    makesure(1,0);
    for (i=0;i<a->height;i++)
        for (j=0;j<a->width;j++)
            c->p[i][j]=a->p[i][j]-b->p[i][j];
}

void mul_var(struct var_struct * a, struct var_struct * b, struct var_struct * c)
{
    int i,j,k;
    makesure(1,0);
    if (is_single_var(a)) {
        makesize(c, b->height, b->width);
        for (i=0;i<b->height;i++)
            for (j=0;j<b->width;j++)
                c->p[i][j]=b->p[i][j]*a->p[0][0];
        return;
    }
    if (is_single_var(b)) {
        makesize(c, a->height, a->width);
        for (i=0;i<a->height;i++)
            for (j=0;j<a->width;j++)
                c->p[i][j]=a->p[i][j]*b->p[0][0];
        return;
    }
    makesure(a->width==b->height,5);
    makesize(c, a->height, b->width);
    makesure(1,0);
    for (i=0;i<a->height;i++)
        for (j=0;j<b->width;j++) {
            c->p[i][j]=0;
            for (k=0;k<a->width;k++)
                c->p[i][j]+=a->p[i][k]*b->p[k][j];
        }
}

void setzero(struct var_struct * a)
{
    int i,j;
    makesure(1,0);
    for (i=0;i<a->height;i++)
        for (j=0;j<a->width;j++)
            a->p[i][j]=0;
}

void setunit(struct var_struct * a)
{
    int i,j;
    makesure(a->height==a->width,8);
    for (i=0;i<a->height;i++)
        for (j=0;j<a->width;j++)
            a->p[i][j]=(i==j);
}

void swaprow(struct var_struct * s, int a, int b)
{
    int i;
    double temp;
    makesure(a<s->height&&a>=0,9);
    makesure(b<s->height&&b>=0,9);
    for (i=0;i<s->width;i++) {
        temp = s->p[a][i];
        s->p[a][i] = s->p[b][i];
        s->p[b][i] = temp;
    }
}

void swapcol(struct var_struct * s, int a, int b)
{
    int i;
    double temp;
    makesure(a<s->width&&a>=0,9);
    makesure(b<s->width&&b>=0,9);
    for (i=0;i<s->height;i++) {
        temp = s->p[i][a];
        s->p[i][a] = s->p[i][b];
        s->p[i][b] = temp;
    }
}

void plusrow(struct var_struct * s, int a, int b, double r)
{
    int i;
    makesure(a<s->height&&a>=0,9);
    makesure(b<s->height&&b>=0,9);
    for (i=0;i<s->width;i++) {
        s->p[b][i] += s->p[a][i]*r;
    }
}

void pluscol(struct var_struct * s, int a, int b, double r)
{
    int i;
    makesure(a<s->width&&a>=0,9);
    makesure(b<s->width&&b>=0,9);
    for (i=0;i<s->height;i++) {
        s->p[i][b] += s->p[i][a]*r;
    }
}

void multrow(struct var_struct * s, int a, double r)
{
    int i;
    makesure(a<s->height&&a>=0,9);
    for (i=0;i<s->width;i++) {
        s->p[a][i] *= r;
    }
}

void multcol(struct var_struct * s, int a, double r)
{
    int i;
    makesure(a<s->width&&a>=0,9);
    for (i=0;i<s->height;i++) {
        s->p[i][a] *= r;
    }
}

void reverse(struct var_struct * s, struct var_struct * t)
{
    int i,j;
    makesure(1,0);
    makesize(t, s->width, s->height);
    makesure(1,0);
    for (i=0;i<s->width;i++)
        for (j=0;j<s->height;j++)
            t->p[i][j]=s->p[j][i];
}

double envalue(struct var_struct * s)
{
    int i,j,flag;
    double result,ration;
    struct var_struct temp;
    if (calcerr) return 0;
    if (s->height!=s->width) {
        calcerr=8;
        return 0;
    }
    temp.p = 0;
    copy_var(s, &temp);
    if (calcerr) return 0;
    result=1;
    for (i=0;i<temp.height;i++) {
        if (temp.p[i][i]==0) {
            flag=0;
            for (j=i+1;j<temp.height;j++) {
                if (temp.p[j][i]!=0) {
                    swaprow(&temp,i,j);
                    result*=-1;
                    flag=1;
                    break;
                }
            }
        }
        if (fabs(temp.p[i][i])<MINIMUM) {
            del(&temp);
            return 0;
        }
        //保证对角线元素不为零
        for (j=i+1;j<temp.height;j++) {
            ration=-temp.p[j][i]/temp.p[i][i];
            plusrow(&temp,i,j,ration);
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
    makesure(A->height==A->width,8);
    temp.p = 0; tempA.p = 0;
    makesize(&temp, A->height, A->height);
    setunit(&temp);
    copy_var(A, &tempA);
    makesure(1,0);
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
        if (fabs(tempA.p[i][i])<MINIMUM) {
            del(&temp);
            del(&tempA);
            calcerr=13;
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
    copy_var(&temp,s);
    del(&temp);
    del(&tempA);
}

int get_var(char * name)
{
    int i;
    if (calcerr) return 0;
    if (!name[0]||strlen(name)>6) {
        calcerr=14;
        return 0;
    }
    for (i=0;i<vart;i++)
        if (!strncasecmp(vars[i].name, name, 8)) {
            return i;
        }
    if (vart>=MAX_VAR) {
        calcerr=15;
        return 0;
    }
    strncpy(vars[vart].name, name, 8);
    vars[vart].p = 0;
    vart++;
    return (vart-1);
}

void set_var(struct var_struct * a, double f)
{
    makesure(1,0);
    makesize(a, 1, 1);
    makesure(1,0);
    (**(a->p)) = f;
}

int check_var_name(char * s, int l)
{
    int i,p=1;
    for (i=0;i<l;i++)
        p=p&&(isalpha(s[i]));
    return p;
}

#define isdouble(c) (isdigit(c)||c=='.'||c=='-')

int check_var_double(char * s, int l)
{
    int i,p=1;
    for (i=0;i<l;i++)
        p=p&&(isdouble(s[i]))&&(i==0||s[i]!='-');
    return p;
}

int check_var_array(char * s, int l)
{
    int i,p=1,count=0;
    for (i=0;i<l;i++) {
        if (s[i]==':') count++;
        p=p&&(isdouble(s[i])||s[i]==':');
    }
    return (p&&count>=1&&count<=2)?count:0;
}

void selmatrix(struct var_struct * s, struct var_struct * u, struct var_struct * v, struct var_struct * p)
{
    int i,j,i0,j0;
    makesure(1,0);
    if (!u->p) {
        makesize(u, 1, s->height);
        makesure(1,0);
        for (i=0;i<s->height;i++)
            u->p[0][i]=(double)i+1;
    }
    if (!v->p) {
        makesize(v, 1, s->width);
        makesure(1,0);
        for (i=0;i<s->width;i++)
            v->p[0][i]=(double)i+1;
    }
    makesure(u->height==1&&v->height==1,10);
    for (i=0;i<u->width;i++) {
        j=(int)(u->p[0][i]+0.5);
        makesure(j>=1&&j<=s->height,9);
    }
    for (i=0;i<v->width;i++) {
        j=(int)(v->p[0][i]+0.5);
        makesure(j>=1&&j<=s->width,9);
    }
    makesize(p, u->width, v->width);
    makesure(1,0);
    for (i=0;i<u->width;i++) {
        i0=(int)(u->p[0][i]+0.5);
        for (j=0;j<v->width;j++) {
            j0=(int)(v->p[0][j]+0.5);
            p->p[i][j]=s->p[i0-1][j0-1];
        }
    }
}

int get_rl(char * s, int r, int l)
{
    int n=r,i=0;
    do {
        if (s[n]==')') i++;
        if (s[n]=='(') i--;
        n--;
    } while (i!=0&&n>=l);
    if (i==0) return n+1;
    else return -1;
}

int get_rl3(char * s, int r, int l)
{
    int n=r,i=0;
    do {
        if (s[n]==']') i++;
        if (s[n]=='[') i--;
        n--;
    } while (i!=0&&n>=l);
    if (i==0) return n+1;
    else return -1;
}

double factorial(double f)
{
    double x, fp=f, ff, prod=0.24197072451914334979783;
    while (fp<20.0) {
        fp += 1.0;
        prod *= fp;
    }

    ff = fp*fp;
    x = exp((fp + 0.5) * (log(fp)-1)
            +(1.0/12.0 - (1.0/360.0 - (1.0/1260.0 - (1.0/1680.0
                                       -1.0/(ff*1188.0))/ff)/ff)/ff)/fp) / prod;
    return(x);
}

char funcname[][8]= {"sin","cos","tan","asin","acos","atan",
                     "log","exp","ln","fact",
                     "sinh","cosh","tanh","asinh","acosh","atanh",
                     "abs","sign","sqr","sqrt","round","floor","ceil",
                     "det","inv",
                     ""
                    };

void take_func(struct var_struct * p, struct var_struct * q, int kind)
{
    int i,j;
    makesure(1,0);
    if (kind<23) {
        makesize(p, q->height, q->width);
        makesure(1,0);
        for (i=0;i<q->height;i++)
            for (j=0;j<q->width;j++) {
                switch (kind) {
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
    } else {
        switch (kind) {
            case 23: set_var(p, envalue(q)); break;
            case 24: inverse(p, q); break;
        }
    }
}

void eval(struct var_struct * p, char * s, int l, int r)
{
    int i,j,n;
    char op[9]=";, +-*/^E";
    char buf[1000];
    while (s[l]==' '&&l<=r) l++;
    while (s[r]==' '&&l<=r) r--;
    makesure(l<=r,11);
    while ((s[l]=='('&&s[r]==')'&&get_rl(s,r,l)==l)||
            (s[l]=='['&&s[r]==']'&&get_rl3(s,r,l)==l)) {
        l++; r--;
        while (s[l]==' '&&l<=r) l++;
        while (s[r]==' '&&l<=r) r--;
    }
    if (check_var_name(s+l, r-l+1)||(s[l]=='%'&&check_var_name(s+l+1, r-l))) {
        strncpy(buf, s+l, 1000);
        buf[r-l+1]=0;
        i=get_var(buf);
        copy_var(vars+i, p);
        return;
    }
    if (check_var_double(s+l, r-l+1)) {
        double f;
        strncpy(buf, s+l, 1000);
        buf[r-l+1]=0;
        f = atof(buf);
        set_var(p, f);
        return;
    }
    if (check_var_array(s+l, r-l+1)) {
        double f1,f2,f3;
        strncpy(buf, s+l, 1000);
        buf[r-l+1]=0;
        i=check_var_array(s+l,r-l+1);
        if (buf[0]==':'&&!buf[1]) {
            p->p=0;
            return;
        } else if (i==1) {
            sscanf(buf, "%lf:%lf", &f1, &f2);
            if (f1<f2) f3=1; else f3=-1;
        } else {
            sscanf(buf, "%lf:%lf:%lf", &f1, &f2, &f3);
            if (fabs(f3)<MINIMUM) {
                calcerr=17;
                return;
            }
            if ((f2-f1)/f3<0) f3=-f3;
        }
        if (f3>0)
            makesize(p, 1, (int)((f2-f1+MINIMUM)/f3)+1);
        else
            makesize(p, 1, (int)((f2-f1-MINIMUM)/f3)+1);
        makesure(1,0);
        i=0;
        do {
            p->p[0][i]=f1;
            f1+=f3;
            i++;
            if (i>p->width) break;
        } while ((f1-MINIMUM<=f2&&f3>0)||(f1+MINIMUM>=f2&&f3<0));
        return;
    }
    i=l;
    while (isalpha(s[i])&&i<=r) i++;
    if (i>l&&s[i]=='('&&s[r]==')'&&get_rl(s,r,l)==i) {
        struct var_struct u,v;
        u.p=0; v.p=0;
        strncpy(buf, s+l, 1000);
        buf[i-l]=0;
        j=0;
        while (funcname[j][0]) {
            if (!strncasecmp(funcname[j],buf,10)) break;
            j++;
        }
        if (funcname[j][0]) {
            eval(&u, s, i+1, r-1);
            makesure(1,0);
            take_func(p, &u, j);
            del(&u);
        } else {
            int k;
            char* kk;
            j=get_var(buf);
            strncpy(buf, s+i+1, 1000);
            buf[r-1-i]=0;
            kk=strchr(buf, ',');
            if (!kk) {
                calcerr=18;
                return;
            }
            k=kk-buf;
            eval(&u, buf, 0, k-1);
            makesure(1,0);
            eval(&v, buf, k+1, strlen(buf)-1);
            makesure(1,0);
            selmatrix(vars+j, &u, &v, p);
            del(&u);
            del(&v);
        }
        return ;
    }
    for (j=0;j<9;j++) {
        n=r;
        do {
            if (toupper(s[n])==op[j]) {
                struct var_struct m1,m2,m3;
                m1.p=0; m2.p=0; m3.p=0;
                if (n>l&&s[n-1]=='.'&&(j==4||j==5||j==6)) eval(&m1,s,l,n-2);
                else eval(&m1,s,l,n-1);
                makesure(1,0);
                eval(&m2,s,n+1,r);
                makesure(1,0);
                switch (j) {
                    case 0:
                        link_h(&m1, &m2, p);
                        break;
                    case 1:
                    case 2:
                        link_w(&m1, &m2, p);
                        break;
                    case 3:
                        add_var(&m1, &m2, p);
                        break;
                    case 4:
                        sub_var(&m1, &m2, p);
                        break;
                    case 5:
                        if (n>l&&s[n-1]=='.') domatrix(&m1,&m2,p,4);
                        else mul_var(&m1, &m2, p);
                        break;
                    case 6:
                        if (n>l&&s[n-1]=='.') domatrix(&m1,&m2,p,5);
                        else {
                            inverse(&m3, &m2);
                            mul_var(&m1, &m3, p);
                            del(&m3);
                        }
                        break;
                    case 7:
                        if (n>l&&s[n-1]=='.') domatrix(&m1,&m2,p,6);
                        else {
                            makesure(is_single_var(&m1)&&is_single_var(&m2),12);
                            makesure((**(m1.p))>0,7);
                            set_var(p, exp(log(**(m1.p))*(**(m2.p))));
                        }
                        break;
                    case 8:
                        makesure(is_single_var(&m1)&&is_single_var(&m2),12);
                        set_var(p, exp(log(10)*(**(m2.p)))*(**m1.p));
                        break;
                }
                del(&m1);
                del(&m2);
                return;
            }
            if (s[n]==')') n=get_rl(s,n,l);
            if (s[n]==']') n=get_rl3(s,n,l);
            n--;
        } while (n>=l);
    }
    if (s[r]=='\'') {
        struct var_struct m;
        m.p = 0;
        eval(&m, s, l, r-1);
        makesure(1,0);
        reverse(&m,p);
        del(&m);
        return;
    }
    calcerr=18;
}

void print_var(struct var_struct * p)
{
    int i,j;
    char buf[1000];
    snprintf(buf, 1000, "%s =\n", p->name);
    outline(buf);
    if (!p->p) {
        outline("null\n");
        return;
    } else if (is_single_var(p)) {
        if (fabs(**(p->p))>1e7||fabs(**(p->p))<1e-3)
            snprintf(buf, 1000, "%le\n", **(p->p));
        else
            snprintf(buf, 1000, "%lf\n", **(p->p));
        outline(buf);
        return;
    }
    for (i=0;i<p->height;i++) {
        if (i==0) outline("[");
        else outline(" ");
        for (j=0;j<p->width;j++) {
            if (fabs(p->p[i][j])>1e7||fabs(**(p->p))<1e-3)
                snprintf(buf, 1000, "%le", p->p[i][j]);
            else
                snprintf(buf, 1000, "%lf", p->p[i][j]);
            outline(buf);
            if (j<p->width-1) outline(" ");
        }
        if (i==p->height-1) outline("]");
        outline("\n");
    }
}

#define HIST 5

int calc_main()
{
    char cmd[1005];
    char history[HIST][1005];
    char einfo[20][30]={
        "",
        "",
        "矩阵太大",
        "矩阵宽度不一致",
        "矩阵长度不一致",
        "矩阵大小不一致",
        "除数不能为0",
        "底数必须为正数",
        "必须为正方矩阵",
        "越界错",
        "矩阵长度必须为1",
        "表达式出错",
        "操作数必须不为矩阵",
        "该矩阵无法求逆",
        "变量名称错",
        "变量数目过多",
        "矩阵输入错误",
        "单步值不能为0",
        "表达式出错",
        ""
    };
    int y,x,res,i,ch,hi,oldmode;
    oldmode=uinfo.mode;
    modify_user_mode(CALC);
    for (i=0;i<HIST;i++) history[i][0]=0;
    res = get_var("res");
    set_var(vars+get_var("%pi"), Pi);
    set_var(vars+get_var("%e"), exp(1));
    clear();
    outline("欢迎使用超级计算器1.0        作者:bad@smth.org\n");
    outline("输入exit退出，输入help帮助\n\n");
    while (1) {
        hi=0;
        getyx(&y, &x);
        cmd[0]=0;
        do {
            UPDOWN = true;
            ch = multi_getdata(y, x, scr_cols-1, "> ", cmd, 995, 13, 0,0);
            UPDOWN = false;
            if (ch==-KEY_UP) {
                if (hi==HIST) cmd[0]=0;
                else strncpy(cmd, history[hi], 1000);
                hi++; if (hi>HIST) hi=0;
            } else if (ch==-KEY_DOWN) {
                hi--; if (hi<0) hi=HIST;
                if (hi==HIST) cmd[0]=0;
                else strncpy(cmd, history[hi], 1000);
            }
        } while (ch<0);
        y = y-1+ch;
        if (y>=scr_lns) y = scr_lns-1;
        move(y, 0);
        outline("\n");
        if (!cmd[0]) continue;
        if (!strncasecmp(cmd, "exit", 5)) break;
        if (!strncasecmp(cmd, "quit", 5)) break;
        for (i=HIST-1;i>0;i--)
            strncpy(history[i],history[i-1],1000);
        strncpy(history[0],cmd,1000);
        if (!strncasecmp(cmd, "help", 5)||!strncasecmp(cmd, "?", 2)) {
            outline("变量: 1到6个字母,例如x=3\n");
            outline("常量: %pi, %e\n");
            outline("矩阵: [3,4;5,6] a(3:4,1:5:2) 1:5:0.5\n");
            outline("函数: sin,cos,tan,asin,acos,atan,log,exp,ln,fact,\n");
            outline("      sinh,cosh,tanh,asinh,acosh,atanh\n");
            outline("      abs,sign,sqr,sqrt,round,floor,ceil\n");
            outline("      det,inv\n");
            outline("操作: + - * / ^ '(转置) .*(矩阵乘) ./(矩阵除) \n");
            continue;
        }
        if (strchr(cmd, '=')) {
            i=strchr(cmd, '=')-cmd;
            if (i<=0||!check_var_name(cmd, i)) {
                calcerr=19;
                goto checkcalcerr;
            }
            cmd[i]=0;
            res = get_var(cmd);
            i++;
        } else {
            res = get_var("res");
            i=0;
        }
        eval(vars+res, cmd+i, 0, strlen(cmd+i)-1);
checkcalcerr:
        if (calcerr) {
            outline(einfo[calcerr]);
            outline("\n");
            calcerr=0;
            continue;
        } else
            print_var(vars+res);
    }
    for (i=0;i<vart;i++)
        del(vars+i);
    modify_user_mode(oldmode);
    return 0;
}
