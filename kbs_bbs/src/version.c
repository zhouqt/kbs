#include "bbs.h"

#define MODULE_VERSION(x,d,l,m,n)  {x,d,l,m,n,0},
#define MODULE_OPTION(d,m)  {null,d,null,m,0},
#define BEGIN_DEFINE const  struct ModuleVersion const *modules={
#define END_DEFINE     {null,null,-1,-1}};
struct ModuleVersion {
  char* name;
  char* desc;
  char* allow_lower; /*最低允许什么版本，如果为空，
        表示必须精确匹配，如果为"0.0"，就不需要匹配*/
  int major;
  int minor;
};

#include "bbsversion.h"

/**
  *  检查版本号
  *  成功返回0
  *  文件不存在， 保存新版本号返回1
  *  失败返回负值
  */

#define BUF_LEN 1024

char* ignoreBlank(char* p)
{
    while (*p&&*p==' '&&*p=='\t') p++;
    return p;
}

void wrongFormat(char* data)
{
    printf("Wrong format:%s\nshoud be: module majorversion.minorversion[.option]\n",data);
}

char* parseNumber(char* p)
{
    char* prev=p;
    while (*p&&(*p>='0'&&*p<='9')) p++;
    if (prev==p) {
        return NULL;
    }
    return p;
}

int saveversion(char* versionfile)
{
    bool printdot;
    FILE* fp;
    struct ModuleVersion * def;
    if ((fp=fopen(versionfile,"wt"))==NULL) {
        perror(versionfile);
        return -1;
    }
    def=modules;
    fprintf(fp,"#modulename major.minor.option");
    while (def->major!=-1&&(def->minor!=-1)) {
        if (def->name!=NULL) {
            fprintf(fp,"\n%s %d.%d",def->name,def->major,def->minor);
            printdot=true;
        }
        else {
            if (printdot)
                fputs(fp,".");
            fprintf(fp,"%d",def->major?1:0);
            printdot=false;
        }
    }
    return 1;
}


/**
  *  遍历module数组，找到名字为name的模块定义
  */
struct ModuleVersion*  findmodule(char* name)
{
    struct ModuleVersion * def;
    def=modules;
    while (def->major!=-1&&(def->minor!=-1)) {
        if ((def->name!=NULL)&&(!strcmp(def->name,name)))
            return def;
    }
    return NULL;
}

int checkversion(char* versionfile)
{
    int ret=0;
    struct ModuleVersion * def;
    FILE* fp;
    char savebuf[BUF_LEN];
    char buf[BUF_LEN];
    if (!dashf(versionfile)) {
        return saveversion(versionfile);
    }

    if ((fp=fopen(versionfile,"rt"))==NULL) {
          perror(versionfile);
          return -1;
    }

    while (fgets(buf,BUF_LEN,fp)!=EOF) {
        char *p,*prev,*option;
        char *name;
        int major,minor;
        strcpy(savebuf,buf);
        if (buf[0]=='#') continue;
        p=buf;
        /* remove \r\n */
        while (*p&&*p!='\r'&&*p!='\n') p++;
        p++;
        p=buf;
        
        /* get module name */
        name=ignoreBlank(p);
        if (*name==0) continue;
        while (*p&&*p!=' '&&*p!='\t') p++;
        if (*p==0) {
            wrongFormat(savebuf);
            continue;
        }
        *p=0;

        /* get major number */
        p++;
        prev=p;
        if (((p=parseNumber(p,savebuf))==NULL)||*p!='.') {
            wrongFormat(savebuf);
            continue;
        }
        *p=0;
        major=atoi(prev);

        /* get minor number */
        p++;
        prev=p;
        if (((p=parseNumber(p,savebuf))==NULL)||(*p!='.'&&*p!=0)) {
            wrongFormat(savebuf);
            continue;
        }
        if (*p=='.') option=p+1;
        else option=NULL;
        *p=0;
        minor=atoi(prev);
        p++;

        /* check major,minjor version */
        def=findmodule(name);
        if (def==NULL)  {
            printf("can't find module %s:");
            ret=-2;
            break;
        }
        if (def.major!=major||def.minor!=minor) {
            int allow_major,allow_minor;
            char* p;
            if (def.allow_lower==NULL)  { //需要精确匹配
                printf("Module %s unmatch\nCode version: %d.%d,Data Version: %d.%d\n\
Please check or upgrade data.\n\
If you are sure it has NOT problem,you can use -f %s to force sync it",
                def.name,def.major,def.minor,major,minor,
                def.name);
                return -6;
            }
            if (!strcmp(def.allow_lower,"0.0")) //not care
                continue;

            /* get allowed version */
            allow_major=0;
            p=def.allow_lower;
            while (*p&&*p!='.') {
                allow_major=allow_major*10+*p-'0';
                p++;
            }
            if (*p!=0) {
                p++;
                allow_minor=0;
                while (*p) {
                    allow_minor=allow_minor*10+*p-'0';
                    p++;
                }
            }
            if (major>allow_major||((major==allow_major)&&(minor>=allow_minor))) {
                /*版本可以接受*/
                printf("Module %s unmatch\nCode version: %d.%d,Data Version: %d.%d\n\
Allow version: %d.%d\nIt's allowed,but you should care it.",
                    def.name,def.major,def.minor,major,minor,
                    allow_major,allow_minor);
            } else {
                /*版本不被许可*/
                printf("Module %s unmatch\nCode version: %d.%d,Data Version: %d.%d\n\
    Allow version: %d.%d\nIt's allowed,but you should care it.",
                    def.name,def.major,def.minor,major,minor,
                    allow_major,allow_minor);
                return -3;;
            }
        }//version not match
       /* check option */
        while (option!=NULL&&*option) {
            def++;
            if (def.name!=NULL) {
                printf("unwanted option:\n%s\n",savebuf);
                for (p=buf;p!=option;p++) printf(" ");
                printf("^\n");
                return -4;
            }
            if (*option!='1'&&*option!='0') {
                wrongFormat(savebuf);
                for (p=buf;p!=option;p++) printf(" ");
                printf("^\n");
                return -5;
            }
            if ((*option=='1'&&!def->major)||(*option=='0'&&def->major)) {
                printf("Module %s option unmatch\nOption Description: %s\nWant: %d\n%s\n",
                    name,def->desc,def->major,savebuf);
                for (p=buf;p!=option;p++) printf(" ");
                printf("^\n");
                return -5;
            }
            option++;
        }
        if (def.name==NULL) {/*有未匹配完的Option*/
            printf("want option but reach the end of line:\n%s\n",savebuf);
            for (p=buf;p!=option;p++) printf(" ");
            printf("^\n");
            return -7;
        }
    }//get line
    /* 检查是否有没有匹配的模块定义 todo*/
    return ret;
}

main()
{
    chdir(BBSHOME);
    checkversion(VERSIONFILE);
}
