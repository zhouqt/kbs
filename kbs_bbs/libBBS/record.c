/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "bbs.h"

#define BUFSIZE (MAXUSERS + 244)

#ifdef SYSV
int
flock(fd, op)
int fd, op;
{
    switch (op) {
        case LOCK_EX:
            return lockf( fd, F_LOCK, 0 );
        case LOCK_UN:
            return lockf( fd, F_ULOCK, 0 );
        default:
            return -1;
    }
}
#endif

int
safewrite(fd, buf, size)
int fd;
char *buf;
int size;
{
    int cc, sz = size, origsz = size;
    char *bp = buf;

#ifdef POSTBUG
    if (size == sizeof(struct fileheader)) {
        char tmp[80];
        struct stat stbuf;
        struct fileheader *fbuf = (struct fileheader *)buf;

        setbpath( tmp, fbuf->filename );
        if (!isalpha(fbuf->filename[0]) || stat(tmp, &stbuf) == -1)
            if (fbuf->filename[0] != 'M' || fbuf->filename[1] != '.') {
                report("safewrite: foiled attempt to write bugged record\n");
                return origsz;
            }
        }
#endif
    do {
        cc = write(fd,bp,sz);
        if ((cc < 0) && (errno != EINTR)) {
            report("safewrite err!");
            return -1;
        }
        if (cc > 0) {
            bp += cc;
            sz -= cc;
        }
    } while (sz > 0);
    return origsz;
}

#ifdef POSTBUG

char bigbuf[10240];
int numtowrite;
int bug_possible = 0;

void
saverecords(filename, size, pos)
char *filename;
int size, pos;
{
    int fd;
    if (!bug_possible) return 0;
    if((fd = open(filename,O_RDONLY)) == -1) return -1;
    if (pos > 5) numtowrite = 5;
    else numtowrite = 4;
    lseek(fd, (pos-numtowrite-1)*size, SEEK_SET);
    read(fd, bigbuf, numtowrite*size);
    close(fd);  /*---	period	2000-10-20	file should be closed	---*/
}

void
restorerecords(filename, size, pos)
char *filename;
int size, pos;
{
    int fd;
    if (!bug_possible) return 0;
    if ((fd = open(filename, O_WRONLY)) == -1) return -1;
    flock(fd, LOCK_EX);
    lseek(fd, (pos-numtowrite-1)*size, SEEK_SET);
    safewrite(fd, bigbuf, numtowrite*size);
    report("post bug poison set out!");
    flock(fd, LOCK_UN);
    bigbuf[0] = '\0';
    close(fd);
}

#endif

long
get_num_records(filename,size)
char    *filename ;
int     size;
{
    struct stat st ;

    if(stat(filename,&st) == -1)
        return 0 ;
    return (st.st_size/size) ;
}

get_sum_records(char* fpath, int size) /* alex于1996.10.20添加 */
{
   struct stat st;
   long ans = 0;
   FILE* fp;
   fileheader fhdr;
   char buf[200], *p;

   if (!(fp = fopen(fpath, "r")))
      return -1;

   strcpy(buf, fpath);
   p = strrchr(buf, '/') + 1;

   while (fread(&fhdr, size, 1, fp) == 1) {
      strcpy(p, fhdr.filename);
      if (stat(buf, &st) == 0 && S_ISREG(st.st_mode) && st.st_nlink == 1)
         ans += st.st_size;
   }
   fclose(fp);
   return ans / 1024;
}


int
append_record(filename,record,size)
char *filename ;
char *record ;
int size ;
{
    int fd ;

#ifdef POSTBUG
    int numrecs = (int)get_num_records(filename, size);
    bug_possible = 1;
    if (size == sizeof(struct fileheader) && numrecs && (numrecs % 4 == 0))
        saverecords(filename, size, numrecs+1);
#endif
    /*if((fd = open(filename,O_WRONLY|O_CREAT,0644)) == -1) {*/
    if((fd = open(filename,O_WRONLY|O_CREAT,0664)) == -1) 
    { /* Leeward 98.04.27: 0664->Enable write access of WWW-POST programe */
        perror("open") ;
        return -1 ;
    }
    flock(fd,LOCK_EX) ;
    lseek(fd, 0, SEEK_END);
    if(safewrite(fd,record,size) == -1)
        report("apprec write err!");
    flock(fd,LOCK_UN);
    close(fd) ;
#ifdef POSTBUG
    if (size == sizeof(struct fileheader) && numrecs && (numrecs % 4 == 0))
        restorerecords(filename, size, numrecs+1);
    bug_possible = 0;
#endif
    return 0 ;
}

void
toobigmesg()
{
/* change by KCN 1999.09.08
    fprintf( stderr, "record size too big!!\n" );
*/
}

int
apply_record(filename,fptr,size)
char *filename ;
int (*fptr)() ;
int size ;
{
    char abuf[BUFSIZE] ;
    int fd ;

    if( size > BUFSIZE ) {
        toobigmesg();
        return -1;
    }
    if((fd = open(filename,O_RDONLY,0)) == -1)
        return -1 ;
    while(read(fd,abuf,size) == size)
        if((*fptr)(abuf) == QUIT) {
            close(fd) ;
            return QUIT ;
        }
    close(fd) ;
    return 0 ;
}
/*---   Added by period   2000-10-26  ---*/
/*---	也可以考虑用一次读入CheckStep个记录的方法.	---*
 *---	就是在内存占用和系统IO之间作个选择		---*/
/*#ifdef _DEBUG_*/
#define _FREE_MEMORY_

#ifndef _FREE_MEMORY_
#  ifndef _FREE_IO_
#    error
#  endif
#else
#  ifdef _FREE_IO_
#    error
#  endif
#endif

int 
search_record_back(filename, size, start, fptr, farg, rptr, sorted)
char *filename ; /* idx file name */
int size ;	/* record size */
int start ;	/* where to start reverse search */
int (*fptr)() ;	/* compare function */ 
char *farg ;	/* additional param to call fptr() / original record */
char *rptr ;	/* record data buffer to be used for reading idx file */
int sorted ; /* if records in file are sorted */
{
    const int CheckStep = 40;
    int fd ;
    int id = 1 , npos, nold=0, nres, ncnt = 0;
#ifdef _FREE_MEMORY_
    int nblk;
    char * rbuf;
    ncnt = CheckStep - 1;
#endif
    
    npos  = get_num_records(filename, size);
    if(start > npos) start = npos;  /* if not enough,begin at end of file */
    start --;   /* convert from list index to file index */
    npos = (start>CheckStep) ? (start - CheckStep) : 0;

    if((fd = open(filename,O_RDONLY,0)) == -1)
        return 0 ;
    if(-1 == lseek(fd, npos*size, SEEK_SET)) {
        close(fd);
        return 0;
    }
#ifdef _FREE_MEMORY_
    id = start - 1;
    nblk = size * CheckStep;
    if( (NULL == (rbuf = malloc(nblk)))
            || (read(fd, rbuf, nblk) <= 0) ) {
	if(NULL != rbuf) free(rbuf);
        close(fd);
        return 0;
    }
    while(1) {
        nres = (*fptr)(farg, rbuf+ncnt*size);
        if(!nres) {
            memcpy(rptr, rbuf+ncnt*size, size);   /* rptr needed as return value !!! */
            free(rbuf);
            close(fd) ;
            return (id + 1);    /* convert from file index to list index */
        }
        if(nres > 0 && sorted)    /* farg is newer than rptr */
            if(++nold > CheckStep) break; /* 多于CheckStep篇旧记录则认为未找到 */
        if(--ncnt < 0) {
            ncnt = CheckStep - 1;   /* 每次CheckStep篇向前查找 */
            npos -= ((npos>CheckStep) ? CheckStep : npos);
            if(-1 == lseek(fd, npos*size, SEEK_SET))
                break;/* return 0 */
            if(read(fd, rbuf, nblk) <= 0)
                break;
        }
        id-- ;
    }
    free(rbuf);
#else
    id = npos;
    while(read(fd,rptr,size) == size) {
        nres = (*fptr)(farg, rptr);
        if(!nres) {
            close(fd) ;
            return (id + 1); /* convert from file index to list index */
        }
        if(nres > 0 && sorted)    /* farg is newer than rptr */
            if(++nold > CheckStep) break; /* 多于CheckStep篇旧记录则认为未找到 */
        if(++ncnt >= CheckStep) {
            ncnt = 0; /* 每次CheckStep篇向前查找 */
            npos = (npos>CheckStep) ? (npos - CheckStep) : 0;
            id = npos;
            if(-1 == lseek(fd, npos*size, SEEK_SET))
                break;/* return 0 */
        } else id++ ;
    }
#endif /* _FREE_MEMORY_ */
    close(fd) ;
    return 0 ;
}
/*#endif*/ /*_DEBUG_*/
/*---   End of Addition     ---*/
int
search_record(filename,rptr,size,fptr,farg)
char *filename ; 
char *rptr ;
int size ;
int (*fptr)() ;
char *farg ;
{
    int fd ;
    int id = 1 ;

    if((fd = open(filename,O_RDONLY,0)) == -1)
        return 0 ;
    while(read(fd,rptr,size) == size) {
        if((*fptr)(farg,rptr)) {
            close(fd) ;
            return id ;
        }
        id++ ;
    }
    close(fd) ;
    return 0 ;
}

int
get_record_handle(fd,rptr,size,id)
int fd;
char *rptr ;
int size, id ;
{
    if(lseek(fd,size*(id-1),SEEK_SET) == -1) 
        return -1 ;
    if(read(fd,rptr,size) != size) 
        return -1 ;
    return 0 ;
}
int
get_record(filename,rptr,size,id)
char *filename ;
char *rptr ;
int size, id ;
{
    int fd ;
    int ret;

    if((fd = open(filename,O_RDONLY,0)) == -1)
        return -1 ;
    ret = get_record_handle(fd,rptr,size,id);
    close(fd) ;
    return ret;
}

int
get_records(filename,rptr,size,id,number)
char *filename ;
char *rptr ;
int size, id, number ;
{
    int fd ;
    int n ;

    if((fd = open(filename,O_RDONLY,0)) == -1)
        return -1 ;
    if(lseek(fd,size*(id-1),SEEK_SET) == -1) {
        close(fd) ;
        return 0 ;
    }
    if((n = read(fd,rptr,size*number)) == -1) {
        close(fd) ;
        return -1 ;
    }
    close(fd) ;
    return (n/size) ;
}

int
substitute_record(filename,rptr,size,id)
char *filename ;
char *rptr ;
int size, id ;
{
/* add by KCN */
    struct flock ldata;
    int retval;

    int fd ;
#ifdef POSTBUG
    if (size == sizeof(struct fileheader) && (id > 1) && ((id - 1) % 4 == 0))
        saverecords(filename, size, id);
#endif
    if((fd = open(filename,O_WRONLY|O_CREAT,0644)) == -1)
        return -1 ;
/* change by KCN 
    flock(fd,LOCK_EX) ;
*/
    ldata.l_type=F_WRLCK;
    ldata.l_whence=0;
    ldata.l_len=size;
    ldata.l_start=size*(id-1);
    if ((retval=fcntl(fd,F_SETLKW,&ldata))== -1) {
        report("reclock error");
	close(fd);	/*---	period	2000-10-20	file should be closed	---*/
        return -1;
    }

    if (lseek(fd,size*(id-1),SEEK_SET) == -1) {
        report("subrec seek err");
        /*---	period	2000-10-24	---*/
	ldata.l_type=F_UNLCK;
	fcntl(fd,F_SETLK,&ldata);
	close(fd);
        return -1;
    }
    if (safewrite(fd,rptr,size) != size)
        report("subrec write err");
/* change by KCN 
    flock(fd,LOCK_UN) ;
*/
    ldata.l_type=F_UNLCK;
    fcntl(fd,F_SETLK,&ldata);

    close(fd) ;
#ifdef POSTBUG
    if (size == sizeof(struct fileheader) && (id > 1) && ((id - 1) % 4 == 0))
        restorerecords(filename, size, id);
#endif
    return 0 ;
}

int
checkreadonly(checked) /* Haohmaru 2000.3.19 */
char *checked;          
{
  struct stat st;
  char        buf[STRLEN];

  sprintf(buf, "boards/%s", checked);
  stat(buf, &st);
  if (365 == (st.st_mode & 0X1FF)) /* Checking if DIR access mode is "555" */
	return YEA;
  else
    return NA;
}

void
tmpfilename( filename, tmpfile, deleted )
char    *filename, *tmpfile, *deleted;
{
    char        *ptr, delfname[STRLEN], tmpfname[STRLEN];

    strcpy( tmpfile, filename );
            if (YEA == checkreadonly(currboard))/*Haohmaru 2000.3.19*/
           {
               sprintf(delfname,".%sdeleted",currboard);
               sprintf(tmpfname,".%stmpfile",currboard);
		if( (ptr = strchr( tmpfile, '/' )) != NULL ) {
       		 strcpy( ptr+1, delfname );
 		       strcpy( deleted, tmpfile );
 		       strcpy( ptr+1, tmpfname );
 		   } else {
 		       strcpy( deleted, delfname );
 		       strcpy( tmpfile, tmpfname );
 		   }
	        return;
           }
	else{
    sprintf(delfname , ".deleted");
    sprintf(tmpfname , ".tmpfile");}

/*    if( (ptr = strchr( tmpfile, '/' )) != NULL ) {
 changed by alex , 97.5.2 , 修正不能删除friends的bug */ 
    if( (ptr = strrchr( tmpfile, '/' )) != NULL ) {
        strcpy( ptr+1, delfname );
        strcpy( deleted, tmpfile );
        strcpy( ptr+1, tmpfname );
    } else {
        strcpy( deleted, delfname );
        strcpy( tmpfile, tmpfname );
    }
}

int
delete_record(filename,size,id)
char *filename ;
int size, id ;
{
    char        tmpfile[ STRLEN ], deleted[ STRLEN ];
    char        abuf[BUFSIZE] ;
    int         fdr, fdw, fd ;
    int         count ;

    if( size > BUFSIZE ) {
        toobigmesg();
        return -1;
    }
    tmpfilename( filename, tmpfile, deleted );
    if((fd = open(".dellock",O_RDWR|O_CREAT|O_APPEND, 0644)) == -1)
        return -1 ;
    flock(fd,LOCK_EX) ;

    if((fdr = open(filename,O_RDONLY,0)) == -1) {
        report("delrec open err");
        flock(fd,LOCK_UN) ;
        close(fd) ;
        return -1 ;
    }
    if((fdw = open(tmpfile,O_WRONLY|O_CREAT|O_EXCL,0644)) == -1) {
        flock(fd,LOCK_UN) ;
        report("delrec tmp err");
        close(fd) ;
        close(fdr) ;
        return -1 ;
    }
    count = 1 ;
    while(read(fdr,abuf,size) == size)
        if(id != count++ && (safewrite(fdw,abuf,size) == -1)) {
            unlink(tmpfile) ;
            close(fdr) ;
            close(fdw) ;
            report("delrec write err");
            flock(fd,LOCK_UN) ;
            close(fd) ;
            return -1 ;
        }
    close(fdr) ;
    close(fdw) ;
    if( rename(filename,deleted) == -1 ||
        rename(tmpfile,filename) == -1 ) {
        flock(fd,LOCK_UN) ;
        report("delrec rename err");
        close(fd) ;
        return -1 ;
    }
    flock(fd,LOCK_UN) ;
    close(fd) ;
    return 0 ;
}

int
delete_range(filename,id1,id2,del_mode)
char *filename ;
int id1,id2,del_mode ;
{
    struct fileheader fhdr;
    char        tmpfile[ STRLEN ], deleted[ STRLEN ], buf[STRLEN];
    int         fdr, fdw, fd;
    int         count;

    tmpfilename( filename, tmpfile, deleted );
    if((fd = open(".dellock",O_RDWR|O_CREAT|O_APPEND, 0644)) == -1)
        return -1;
    flock(fd,LOCK_EX);

    if((fdr = open(filename,O_RDONLY,0)) == -1) {
        flock(fd,LOCK_UN);
        close(fd);
        return -2;
    }

    if((fdw = open(tmpfile,O_WRONLY|O_CREAT|O_EXCL,0644)) == -1) {
/* add by KCN for debug
#undef perror
	dup2(0,1);
	dup2(0,2);
	perror(tmpfile);
	pressanykey();
*/
        close(fdr);
        flock(fd,LOCK_UN);
        close(fd);
        return -3;
    }
    count = 1;
    while(read(fdr,&fhdr,sizeof fhdr) == sizeof fhdr) {
        if(count < id1 || count > id2 || ((del_mode!=1) && strcmp(fhdr.owner,"deliver") && (fhdr.accessed[0] & FILE_MARKED)))/*Haohmaru.99.4.20.强制删除mark文章*/ {
            if((safewrite(fdw,&fhdr,sizeof fhdr) == -1)) {
                unlink(tmpfile);
                close(fdw);
                close(fdr);
                flock(fd,LOCK_UN);
                close(fd);
                return -4;
            }
        } else {
            char *t;
            char fullpath[STRLEN];

            strcpy(buf,filename);
            if( (t = strrchr(buf,'/')) != NULL )
                *t = '\0';
            sprintf(fullpath,"%s/%s",buf,fhdr.filename);
#ifndef LEEWARD_X_RECORD /* This macro need defining for *.c under directories
                                                  of both local_utl and innd */
            /* Leeward: 98.01.22 adds below for logs in deleted/junk */
            cancelpost(currboard, currentuser.userid, 
                       &fhdr, !strcmp(fhdr.owner, currentuser.userid));
#endif
            unlink(fullpath);
        }
        count++;
    }
    close(fdw);
    close(fdr);
    if (YEA == checkreadonly(currboard))/*Haohmaru.使只读情况下也可以区段删除*/
    {
	sprintf(buf,"cp -f ~bbsroot/%s ~bbsroot/%s",filename,deleted);
	system(buf);
    }
    else if(rename(filename,deleted) == -1) {
        flock(fd,LOCK_UN);
        close(fd);
        return -5;
    }
    if (YEA == checkreadonly(currboard))
    {
        sprintf(buf,"cp -f ~bbsroot/%s ~bbsroot/%s",tmpfile,filename);
        system(buf);
	sprintf(buf,"rm -rf ~bbsroot/%s",tmpfile);
	system(buf);
    }
    else if(rename(tmpfile,filename) == -1) {
        flock(fd,LOCK_UN);
        close(fd);
        return -6;
    }
    flock(fd,LOCK_UN);
    close(fd);
    return 0;
}

int
update_file(dirname,size,ent,filecheck,fileupdate)
char *dirname ;
int size,ent ;
int (*filecheck)() ;
void (*fileupdate)() ;
{
    char abuf[BUFSIZE] ;
    int fd ;

    if( size > BUFSIZE) {
        toobigmesg();
        return -1 ;
    }
    if((fd = open(dirname,O_RDWR)) == -1)
        return -1 ;
    flock(fd,LOCK_EX) ;
    if(lseek(fd,size*(ent-1),SEEK_SET) != -1) {
        if(read(fd,abuf,size) == size)
            if((*filecheck)(abuf)) {
                lseek(fd,-size,SEEK_CUR) ;
                (*fileupdate)(abuf) ;
                if(safewrite(fd,abuf,size) != size) {
                    report("update err");
                    flock(fd,LOCK_UN) ;
                    close(fd) ;
                    return -1 ;
                }
                flock(fd,LOCK_UN) ;
                close(fd) ;
                return 0 ;
            }
    }
    lseek(fd,0,SEEK_SET) ;
    while(read(fd,abuf,size) == size) {
        if((*filecheck)(abuf)) {
            lseek(fd,-size,SEEK_CUR) ;
            (*fileupdate)(abuf) ;
            if(safewrite(fd,abuf,size) != size) {
                report("update err");
                flock(fd,LOCK_UN) ;
                close(fd) ;
                return -1 ;
            }
            flock(fd,LOCK_UN) ;
            close(fd) ;
            return 0 ;
        }
    }
    flock(fd,LOCK_UN) ;
    close(fd) ;
    return -1 ;
}

int
delete_file(dirname,size,ent,filecheck)
char *dirname ;
int size,ent ;
int (*filecheck)() ;
{
    char abuf[BUFSIZE] ;
    int fd ;
    struct stat st ;
    long numents ;

    if( size > BUFSIZE) {
        toobigmesg();
        return -1 ;
    }
    if((fd = open(dirname,O_RDWR)) == -1)
        return -1 ;
    flock(fd,LOCK_EX) ;
/*---	modified by period	2000-09.21	4 debug	---*/
    numents = fstat(fd,&st);
    if(0 != numents) {
	char buf[256];
	sprintf(buf, "%s stat error - delf", dirname);
	report(buf);
    }
/*    fstat(fd,&st) ;  */
    numents = ((long)st.st_size)/size ;
    if(((long)st.st_size) % size != 0)
/* change by KCN 1999.09.08
        fprintf(stderr,"align err\n") ;
*/
    if(lseek(fd,size*(ent-1),SEEK_SET) != -1) {
        if(read(fd,abuf,size) == size)
            if((*filecheck)(abuf)) {
                int i ;
                for(i = ent; i < numents; i++) {
                    if(lseek(fd,(i)*size,SEEK_SET) == -1)       break ;
                    if(read(fd,abuf,size) != size)              break ;
                    if(lseek(fd,(i-1)*size,SEEK_SET) == -1)     break ;
                    if(safewrite(fd,abuf,size) != size)         break ;
                }
                ftruncate(fd,(off_t)size*(numents-1)) ;
                flock(fd,LOCK_UN) ;
                close(fd) ;
                return 0 ;
            }
    }
    lseek(fd,0,SEEK_SET) ;

    /* Leeward 99.07.13 revised below '1' to '0' to fix a big bug */
    /* ent = 1 ; */
    ent = 0 ;
    while(read(fd,abuf,size) == size) {
        if((*filecheck)(abuf)) {
            int i ;
            for(i = ent; i < numents; i++) {
                if(lseek(fd,(i+1)*size,SEEK_SET) == -1) break ;
                if(read(fd,abuf,size) != size) break ;
                if(lseek(fd,(i)*size,SEEK_SET) == -1) break ;
                if(safewrite(fd,abuf,size) != size) break ;
            }
            ftruncate(fd,(off_t)size*(numents-1)) ;
            flock(fd,LOCK_UN) ;
            close(fd) ;
            return 0 ;
        }
        ent++ ;
    }
    flock(fd,LOCK_UN) ;
    close(fd) ;
    return -1 ;
}

