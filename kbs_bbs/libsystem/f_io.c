/* etnlegend, 2006.04.01, Reorganization FileIO functions in libsystem */

#include "system.h"

#define FMASK 0644


/* Function `f_append_file`
 *
 * Behavs: Append content of file `src` to file `dst`
 * Params:
 *   `src`: Name of the file holding the data to be appended
 *   `dst`: Name of the file to be appended with
 *  `mode`: Mode for opening the destination file
 * Return:
 *       0: Success
 *      -1: Failed for file read error
 *      -2: Failed for file `src` open error
 *      -4: Failed for file `dst` open error
 *      -8: Failed for memory allocation error
 *       +: Failed for file write error
 *
*/
static int f_append_file(const char *src,const char *dst,int mode){
#ifndef BUFFER_IN_STACK
    char *buf;
#else
    char buf[READ_BUFFER_SIZE];
#endif
    int sfd,dfd,len,ret;
    const void *p;
#ifndef BUFFER_IN_STACK
    if(!(buf=(char*)malloc(READ_BUFFER_SIZE*sizeof(char)))){
        return -8;
    }
#endif
    if((sfd=open(src,O_RDONLY,FMASK))==-1){
#ifndef BUFFER_IN_STACK
        free(buf);
#endif
        return -2;
    }
    if((dfd=open(dst,O_WRONLY|mode,FMASK))==-1){
#ifndef BUFFER_IN_STACK
        free(buf);
#endif
        close(sfd);
        return -4;
    }
    while((len=read(sfd,buf,READ_BUFFER_SIZE))>0){
        for(p=buf,ret=0;len>0&&ret!=-1;vpm(p,ret),len-=ret)
            ret=write(dfd,p,len);
        if(len)
            break;
    }
#ifndef BUFFER_IN_STACK
    free(buf);
#endif
    close(sfd);
    close(dfd);
    return len;
}

/* Function `rm_dir`
 *
 * Behavs: Remove directory `path`
 * Params:
 *  `path`: directory to be removed
 * Return:
 *       0: Success
 *   Other: Failed
 *
*/
static int rm_dir(const char *path){
    DIR *dp;
    struct dirent *de;
    struct stat st;
    char buf[512],*p,*q,*r,*s;
    if(!(dp=opendir(path)))
        return -1;
    if(!(s=(p=strdup(path)))){
        closedir(dp);
        return -2;
    }
    q=buf;
    if(*path=='/')
        *q++='/';
    while(!!(r=strsep(&p,"/"))){
        if(!*r)
            continue;
        while(*r)
            *q++=*r++;
        *q++='/';
    }
    free(s);
    while(!!(de=readdir(dp))){
        if(!*(de->d_name))
            continue;
        if(!strcmp(de->d_name,"."))
            continue;
        if(!strcmp(de->d_name,".."))
            continue;
        sprintf(q,"%s",de->d_name);
        if(lstat(buf,&st))
            continue;
        if(!S_ISDIR(st.st_mode))
            unlink(buf);
        else
            rm_dir(buf);
    }
    closedir(dp);
    *q=0;
    return rmdir(buf);
}


/*********************************************************************** f_cat */

/* Function `f_cat`
 *
 * Behavs: Append text string `str` to file `file`
 * Params:
 *  `file`: Name of the file to be appended with
 *   `str`: NULL terminated string as appendage
 * Return:
 *       0: Success
 *       -: Failed for file open error
 *       +: Failed for file write error
 *
*/
int f_cat(const char *file,const char *str){
    int fd,len,ret;
    const void *p;
    if((fd=open(file,O_WRONLY|O_CREAT|O_APPEND,FMASK))==-1)
        return -1;
    for(p=str,len=strlen(str),ret=0;len>0&&ret!=-1;vpm(p,ret),len-=ret)
        ret=write(fd,p,len);
    close(fd);
    return len;
}

/* Function `f_catfile`
 *
 * Behavs: Append content of file `src` to file `dst`
 * Params:
 *   `src`: Name of the file holding the data to be appended
 *   `dst`: Name of the file to be appended with
 * Return:
 *       0: Success
 *      -1: Failed for file read error
 *      -2: Failed for file `src` open error
 *      -4: Failed for file `dst` open error
 *      -8: Failed for memory allocation error
 *       +: Failed for file write error
 *
*/
int f_catfile(const char *src,const char *dst){
    return f_append_file(src,dst,(O_CREAT|O_APPEND));
}


/*********************************************************************** f_cp */

/* Function `f_cp`
 *
 * Behavs: Copy file `src` to file `dst`
 * Params:
 *   `src`: Name of the source file
 *   `dst`: Name of the destination file
 *  `mode`: Mode for opening the destination file
 * Return:
 *       0: Success
 *      -1: Failed for file read error
 *      -2: Failed for file `src` open error
 *      -4: Failed for file `dst` open error
 *      -8: Failed for memory allocation error
 *       +: Failed for file write error
 *
*/
int f_cp(const char *src,const char *dst,int mode){
    return f_append_file(src,dst,(O_CREAT|O_TRUNC|mode));
}


/*********************************************************************** f_ln */

/* Function `f_ln`
 *
 * Behavs: Hard Link/Copy file `src` to file `dst`
 * Params:
 *   `src`: Name of the source file
 *   `dst`: Name of the destination file
 * Return:
 *       0: Success
 *      -1: Failed for file read error
 *      -2: Failed for file `src` open error
 *      -4: Failed for file `dst` open error
 *      -8: Failed for memory allocation error
 *     -16: Failed for link error
 *       +: Failed for file write error
 *
*/
int f_ln(const char *src,const char *dst){
    if(!link(src,dst))
        return 0;
    return ((errno==EMLINK||errno==EXDEV)?f_cp(src,dst,0):-16);
}


/*********************************************************************** f_lock */

/* Function `lock_reg`
 *
 * Behavs: Register file lock
 * Params:
 *    `fd`: File Descriptor to which the lock would be applied
 *   `cmd`: Command to `fcntl`, should be one of F_SETLK/F_SETLKW/F_GETLK/F_GETLKW
 *  `type`: Lock type, shoule be one of F_RDLCK/F_WRLCK/F_UNLCK
 *`offset`: Offset of the start of the lock region
 *`whence`: To what the offset is relative
 *   `len`: Length of the lock region
 * Return:
 *      -1: Failed
 *   Other: Success
 *
*/
int lock_reg(int fd,int cmd,int type,off_t offset,int whence,off_t len){
    struct flock lock;
    lock.l_type=type;
    lock.l_start=offset;
    lock.l_whence=whence;
    lock.l_len=len;
    return fcntl(fd,cmd,&lock);
}

#ifndef HAVE_FLOCK
/* Function `flock`
 *
 * Behavs: Perform quick lock on file descriptor
 * Params:
 *    `fd`: File Descriptor to which the lock would be applied
 *    `op`: Lock type, should be one of LOCK_SH/LOCK_EX/LOCK_UN bitwise OR with LOCK_NB/0
 * Return:
 *      -1: Failed
 *   Other: Success
 *
*/
int flock(int fd,int op){
    int cmd,ret;
    cmd=((op&LOCK_NB)?F_SETLK:F_SETLKW);
    switch(op&~LOCK_NB){
        case LOCK_SH:
            ret=lock_reg(fd,cmd,F_RDLCK,0,SEEK_SET,0);
            break;
        case LOCK_EX:
            ret=lock_reg(fd,cmd,F_WRLCK,0,SEEK_SET,0);
            break;
        case LOCK_UN:
            ret=lock_reg(fd,cmd,F_UNLCK,0,SEEK_SET,0);
            break;
        default:
            errno=EINVAL;
            return -1;
    }
    if(ret==-1&&(errno==EAGAIN||errno==EACCES))
        errno=EWOULDBLOCK;
    return ret;
}
#endif


/*********************************************************************** f_mv */

/* Function `f_mv`
 *
 * Behavs: Move file `src` to file `dst`
 * Params:
 *   `src`: Name of the source file
 *   `dst`: Name of the destination file
 * Return:
 *       0: Success
 *   Other: Failed
 *
*/
int f_mv(const char *src,const char *dst){
    if(!rename(src,dst))
        return 0;
    if(!f_cp(src,dst,0)){
        unlink(src);
        return 0;
    }
    return -1;
}


/*********************************************************************** f_rm */

/* Function `f_rm`
 *
 * Behavs: Remove file or directory `path`
 * Params:
 *  `path`: File or directory to be removed
 * Return:
 *       0: Success
 *   Other: Failed
 *
*/
int f_rm(const char *path){
    struct stat st;
    if(lstat(path,&st))
        return -1;
    return (!S_ISDIR(st.st_mode)?unlink(path):rm_dir(path));
}


/*********************************************************************** f_touch */

/* Function `f_touch`
 *
 * Behavs: touch file `file`
 * Params:
 *  `file`: File to be touched
 * Return:
 *       0: Success
 *   Other: Failed
 *
*/
int f_touch(const char *file){
    int fd;
    if((fd=open(file,O_WRONLY|O_CREAT|O_NONBLOCK|O_NOCTTY|O_LARGEFILE,FMASK))==-1)
        return -1;
    close(fd);
    return utime(file,NULL);
}

