#include "bbs.h"
#define USE_FILE_COPY

#ifndef USE_FILE_COPY
#define USE_HARD_LINK
#else
#undef USE_HARD_LINK
#endif
#define TRIES_ON_SAME_NAME 4
static const char* const suffix="0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
/*在指定的.DIR结构中用二分法查找具有指定id的fileheader的位置*/
unsigned int getipos(struct fileheader *dir,unsigned int size,unsigned int id)
{
    /*二分区间为[0,size],包括dir[size](通常是当前fileheader本身)*/
    unsigned int l,h,m;
    /*边界情况*/
    if (dir[h=size].id==id)
        return size;
    if (dir[l=0].id==id)
        return 0;
    /*开区间二分法*/
    while ((m=(l+h)/2)!=l)
        if (dir[m].id<id)
            l=m;
        else if (dir[m].id>id)
            h=m;
        else
            return m;
    return (unsigned int)-1;
}
/*在指定的.DIR结构中,从当前位置逆序查找第一个具有指定groupid的fileheader的位置*/
unsigned int getgpos(struct fileheader *dir,unsigned int size,unsigned int groupid)
{
    unsigned int i;
    for (i=size-1;i!=(unsigned int)-1;i--)
        if (dir[i].groupid==groupid)
            return i;
    return (unsigned int)-1;
}
/*初始化*/
int initdir(char *prog,char *path,char *board,unsigned int **offset,unsigned int *size,struct fileheader **dir,char **p)
{
    struct stat st;
    int filedes;
    /*打开.DIR文件*/
    sprintf(path,"boards/%s/.DIR",board);
    if (stat(path,&st)||!S_ISREG(st.st_mode)||(filedes=open(path,O_RDONLY))==-1) {
        printf("[%s] 打开 %s 错误...\n",prog,path);
        return 0x21;
    }
    /*计算.DIR大小,折算为fileheader数量*/
    if (st.st_size%sizeof(struct fileheader)) {
        printf("[%s] 文件 %s 损坏, 请修复后再进行版面合并操作...\n",prog,path);
        return 0x22;
    }
    (*size)=st.st_size/sizeof(struct fileheader);
    /*申请辅助用偏移表空间*/
    if ((*size)&&!((*offset)=(unsigned int*)malloc((*size)*sizeof(unsigned int)))) {
        printf("[%s] 申请辅助表空间失败...\n",prog);
        return 0x23;
    }
    /*映象.DIR文件*/
    if ((*size)&&((*dir)=mmap(NULL,(*size)*sizeof(struct fileheader),PROT_READ|PROT_WRITE,MAP_PRIVATE,filedes,0))==(void*)-1) {
        printf("[%s] 构造 %s 文件映象失败...\n",prog,path);
        return 0x24;
    }
    close(filedes);
    (*p)=path+strlen(path)-4;
    return 0;
}
/*合并版面处理*/
void process(char *prog,struct fileheader *ddir,struct fileheader *dir,unsigned int *offset,
             unsigned int dpos,unsigned int pos,char *dpath,char *dp,char *path,char *p)
{
    unsigned int ret;
    memcpy(&ddir[dpos],&dir[pos],sizeof(struct fileheader));
    /*存入当前对应id并分配新的顺序id*/
    offset[pos]=dpos;
    ddir[dpos].id=dpos+1;
    /*寻找对应的groupid*/
    if ((ret=getipos(dir,pos,ddir[dpos].groupid))!=(unsigned int)-1)/*groupid对应的fileheader存在*/
        ddir[dpos].groupid=offset[ret]+1;
    else if ((ret=getgpos(dir,pos,ddir[dpos].groupid))!=(unsigned int)-1)/*寻找向前最近的同主题fileheader*/
        ddir[dpos].groupid=ddir[offset[ret]].groupid;
    else
        ddir[dpos].groupid=ddir[dpos].id;/*设置groupid=id*/
    /*寻找对应的reid*/
    if (ddir[dpos].groupid==ddir[dpos].id)/*主题起始文章,reid=id*/
        ddir[dpos].reid=ddir[dpos].id;
    else if ((ret=getipos(dir,pos,ddir[dpos].reid))!=(unsigned int)-1)/*reid对应的fileheader存在*/
        ddir[dpos].reid=offset[ret]+1;
    else
        ddir[dpos].reid=ddir[dpos].id;/*设置reid=id*/
    /*对丢失主题起始文章而出现的新的主题起始文标题处理*/
    if (ddir[dpos].groupid==ddir[dpos].id&&!strncmp(ddir[dpos].title,"Re: ",4))
        memmove(ddir[dpos].title,ddir[dpos].title+4,ARTICLE_TITLE_LEN-4);
    sprintf(p,"%s",ddir[dpos].filename);
    sprintf(dp,"%s",ddir[dpos].filename);
#ifdef USE_HARD_LINK
    /*硬链接对应文件*/
    if (link(path,dpath)) {
        if (errno==EEXIST) {
            char *ptr;int i;
            ptr=ddir[dpos].filename+strlen(ddir[dpos].filename)-2;
            srand(getpid());
            /*失败的时候在同一时间点上更换后缀*/
            for (i=0;i<TRIES_ON_SAME_NAME;i++) {
                *ptr=suffix[rand()%62];*(ptr+1)=suffix[rand()%62];
                sprintf(dp,"%s",ddir[dpos].filename);
                if (!link(path,dpath)) {
                    printf("[%s] 更改重名硬链接: %s -> %s\n",prog,path,ddir[dpos].filename);
                    return;
                }
            }
        }
        printf("[%s] 硬链接 %d (%s) 失败...\n",prog,pos,path);
    }
#else
    /*复制对应文件*/
    if (f_cp(path,dpath,O_EXCL)==-1) {
        char *ptr;int i;
        ptr=ddir[dpos].filename+strlen(ddir[dpos].filename)-2;
        srand(getpid());
        /*失败的时候在同一时间点上更换后缀*/
        for (i=0;i<TRIES_ON_SAME_NAME;i++) {
            *ptr=suffix[rand()%62];*(ptr+1)=suffix[rand()%62];
            sprintf(dp,"%s",ddir[dpos].filename);
            if (f_cp(path,dpath,O_EXCL)!=-1) {
                printf("[%s] 更改重名文件: %s -> %s\n",prog,path,ddir[dpos].filename);
                return;
            }
        }
        printf("[%s] 复制 %d (%s) 失败...\n",prog,pos,path);
    }
#endif
    return;
}
/*检查一个.DIR结构是否满足id的顺序关系*/
int checkseq(struct fileheader *dir,unsigned int size)
{
    unsigned int i;
    for (i=1;i<size;i++)
        if (!(dir[i-1].id<dir[i].id))
            return 0;
    return 1;
}
/*对不满足id顺序关系的.DIR结构进行顺序化预处理*/
int seqdir(struct fileheader *dir,unsigned int size)
{
    unsigned int i,j;
    unsigned int *ioffset,*goffset;
    /*两个辅助表,分别用于寻找groupid和reid对应关系*/
    if (!(ioffset=(unsigned int*)malloc(2*size*sizeof(unsigned int))))
        return 0;
    goffset=ioffset+size;
    for (i=0;i<size;i++) {
        ioffset[i]=dir[i].id;
        goffset[i]=dir[i].groupid;
        dir[i].id=i+1;
        /*对应reid*/
        for (j=i-1;j!=(unsigned int)-1;j--)
            if (ioffset[j]==dir[i].reid) {
                dir[i].reid=j+1;
                break;
            }
        if (j==(unsigned int)-1)
            dir[i].reid=dir[i].id;
        /*对应groupid*/
        for (j=0;!(j>i);j++)
            if (goffset[j]==dir[i].groupid) {
                dir[i].groupid=j+1;
                break;
            }
    }
    free(ioffset);
    return 1;
}
/*主函数*/
int main(int argc,char **argv)
{
    FILE *fp;
    struct stat st;
    struct fileheader *dir1,*dir2,*dir;
    struct boardheader bh;
    struct BoardStatus *bs;
    char path1[512],path2[512],path[512],buf[512],*p1,*p2,*p;
    int ret;
    unsigned int size1,size2,pos1,pos2,pos,*offset1,*offset2;
    //初始化及错误检测
    if (argc!=4) {
        printf("[%s] 错误的参数结构...\nUSAGE: %s <srcboard1> <srcboard2> <dstdir>\n",argv[0],argv[0]);
        return 0x11;
    }
    if (chdir(BBSHOME)) {
        printf("[%s] 切换到 BBS 主目录 %s 错误...\n",argv[0],BBSHOME);
        return 0x12;
    }
    /*初始化源版面数据*/
    if ((ret=initdir(argv[0],path1,argv[1],&offset1,&size1,&dir1,&p1))!=0)
        return ret;
    if ((ret=initdir(argv[0],path2,argv[2],&offset2,&size2,&dir2,&p2))!=0)
        return ret;
    /*检测顺序并进行可能必要的顺序化*/
    if (!checkseq(dir1,size1)&&!seqdir(dir1,size1)) {
        printf("[%s] 版面 %s .DIR 结构错误, 修复失败...\n",argv[0],argv[1]);
        return 0x51;
    }
    if (!checkseq(dir2,size2)&&!seqdir(dir2,size2)) {
        printf("[%s] 版面 %s .DIR 结构错误, 修复失败...\n",argv[0],argv[2]);
        return 0x51;
    }
    /*创建目标目录,用pid进行标识区分*/
    sprintf(path,"boards/%s",argv[3]);
    sprintf(buf,"boards/%s_%d",argv[3], (int)getpid());
    if (!stat(path,&st)&&S_ISDIR(st.st_mode))
        rename(path,buf);
    else
        unlink(path);
    if (mkdir(path,(S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH))) {
        printf("[%s] 创建版面目录 %s 错误...\n",argv[0],path);
        return 0x31;
    }
    build_board_structure(argv[3]);
    p=path+strlen(path);*p++='/';
    sprintf(p,".DIR");
    /*准备目标.DIR文件的写操作*/
    if (!(fp=fopen(path,"wb"))) {
        printf("[%s] 创建 %s 错误...\n",argv[0],path);
        return 0x32;
    }
    /*申请目标.DIR结构需要的内存空间*/
    if (!(dir=(struct fileheader*)malloc((size1+size2)*sizeof(struct fileheader)))) {
        printf("[%s] 申请内存空间失败...\n",argv[0]);
        return 0x33;
    }
    //主处理
    pos1=0;pos2=0;pos=0;
    while (true) {
        /*比较时间戳*/
        if (pos1<size1&&pos2<size2) {
            //ret=strcmp(&dir1[pos1].filename[2],&dir2[pos2].filename[2]);
            ret = get_posttime(&dir1[pos1]) - get_posttime(&dir2[pos2]);
        } else if (pos1<size1)/*源版面2已经处理完成*/
            ret=-1;
        else if (pos2<size2)/*源版面1已经处理完成*/
            ret=1;
        else/*done*/
            break;
        /*处理当前fileheader*/
        if (!(ret>0)) {/*源版面1*/
            process(argv[0],dir,dir1,offset1,pos,pos1,path,p,path1,p1);
            pos1++;pos++;
        } else {/*源版面2*/
            process(argv[0],dir,dir2,offset2,pos,pos2,path,p,path2,p2);
            pos2++;pos++;
        }
    }
    /*写回目的.DIR*/
    if (fwrite(dir,sizeof(struct fileheader),(size1+size2),fp)!=(size1+size2)) {
        printf("[%s] 写入文件错误...\n",argv[0]);
        return 0x41;
    }
    /*设定未读标记*/
    resolve_boards();
    ret=getboardnum(argv[3],&bh);
    if (ret) {
        bh.idseq=size1+size2;
        set_board(ret,&bh,NULL);
        bs=getbstatus(ret);
        bs->total=size1+size2;
        bs->lastpost=size1+size2;
        bs->updatemark=true;
        bs->updatetitle=true;
        bs->updateorigin=true;
        bs->nowid=size1+size2;
        bs->toptitle=0;
    }
    /*清理*/
    fclose(fp);
    munmap(dir1,size1*sizeof(struct fileheader));
    if (size1)
        free(offset1);
    munmap(dir2,size2*sizeof(struct fileheader));
    if (size2)
        free(offset2);
    free(dir);
    return 0;
}
