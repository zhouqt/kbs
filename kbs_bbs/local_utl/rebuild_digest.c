#include "bbs.h"
static ssize_t data_write(int fd,const void *data,size_t size){
    size_t len;
    ssize_t ret;
    const void *p;
    for(ret=0,p=data,len=size;len>0&&ret!=-1;vpm(p,ret),len-=ret)
        ret=write(fd,p,len);
    return ret;
}
int main(int argc,char **argv){
    struct stat st;
    struct fileheader fh,*fptr;
    char buf[256],fn[256];
    const char *board;
    int fd,i,j,count;
    void *ptr;
    if(argc!=2||!(board=argv[1]))
        return __LINE__;
    if(chdir(BBSHOME)==-1)
        return __LINE__;
    setbdir(DIR_MODE_NORMAL,fn,board);
    if(stat(fn,&st)||!S_ISREG(st.st_mode)||!(st.st_size))
        return __LINE__;
    if((fd=open(fn,O_RDONLY,0644))==-1)
        return __LINE__;
    if(flock(fd,LOCK_SH)==-1)
        return __LINE__;
    ptr=mmap(NULL,st.st_size,PROT_READ,MAP_SHARED,fd,0);
    flock(fd,LOCK_UN);
    close(fd);
    if(ptr==MAP_FAILED)
        return __LINE__;
    setbdir(DIR_MODE_DIGEST,fn,board);
    if((fd=open(fn,O_WRONLY|O_CREAT|O_TRUNC,0644))==-1){
        munmap(ptr,st.st_size);
        return __LINE__;
    }
    if(flock(fd,LOCK_EX)==-1){
        close(fd);
        munmap(ptr,st.st_size);
        return __LINE__;
    }
    for(count=st.st_size/sizeof(struct fileheader),
        fptr=(struct fileheader*)ptr,j=0,i=0;i<count;i++){
        if((fptr[i].accessed[0])&FILE_DIGEST){
            memcpy(&fh,&fptr[i],sizeof(struct fileheader));
            fh.accessed[0]&=~FILE_DIGEST;
            if(fh.filename[1]=='/')
                fh.filename[2]='G';
            else
                fh.filename[0]='G';
            setbfile(fn,board,fptr[i].filename);
            setbfile(buf,board,fh.filename);
            unlink(buf);
            if(link(fn,buf)==-1)
                continue;
            data_write(fd,&fh,sizeof(struct fileheader));
            j++;
        }
    }
    flock(fd,LOCK_UN);
    close(fd);
    munmap(ptr,st.st_size);
    printf("Rebuild: %-32.32s <%d>!\n",board,j);
    return 0;
}
