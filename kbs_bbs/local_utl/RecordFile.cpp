#include <unistd.h>
#include "RecordFile.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>
#include "bbs.h"

extern int errno;

extern "C" int flock(int,int);

CRecordFile::CRecordFile(char* filename)
{
	if (strlen(filename)) {
		m_strFileName=new char[strlen(filename)];
		strcpy(m_strFileName,filename);
	} else
		m_strFileName = (char*)NULL;
}

CRecordFile::~CRecordFile()
{
	if (m_strFileName)
		delete m_strFileName;
}

int CRecordFile::ApplyRecord(int len,APPLY_FUNC func)
{
  char* data = new char[len];
  int fd ;
  int ret;

  if((fd = open(m_strFileName,O_RDONLY,0)) == -1)
        return -1 ;
  while(read(fd,data,len) == len) {
      if(ret=(*func)(data)) {
          break;
      }
  }
  close(fd) ;
  delete data;
  return ret;
}

int CRecordFile::SearchRecord(void* data,int len,int (*func)(void*,void*),void* param)
{
    int fd ;
    int id = 1 ;

    if((fd = open(m_strFileName,O_RDONLY,0)) == -1)
        return 0 ;
    while(read(fd,data,len) == len) {
        if((*func)(param,data)) {
            close(fd) ;
            return id ;
        }
        id++ ;
    }
    close(fd) ;
    return 0 ;
}

int CRecordFile::GetRecord(void* data,int len,int pos)
{
    int fd ;

    if((fd = open(m_strFileName,O_RDONLY,0)) == -1)
        return -1 ;
    if(lseek(fd,len*(pos-1),SEEK_SET) == -1) {
        close(fd) ;
        return -1 ;
    }
    if(read(fd,data,len) != len) {
        close(fd) ;
        return -1 ;
    }
    close(fd) ;
    return 0 ;
}


int SafeWrite(int fd,char* buf,int size)
{
    int cc, sz = size, origsz = size;
    char *bp = buf;

    do {
        cc = write(fd,bp,sz);
        if ((cc < 0) && (errno != EINTR)) {
            return -1;
        }
        if (cc > 0) {
            bp += cc;
            sz -= cc;
        }
    } while (sz > 0);
    return origsz;
}

int CRecordFile::AppendRecord(void* data,int len)
{
   int fd ;

    if((fd = open(m_strFileName,O_WRONLY|O_CREAT,0664)) == -1) 
    { /* Leeward 98.04.27: 0664->Enable write access of WWW-POST programe */
        perror("open") ;
        return -1 ;
    }
    flock(fd,F_WRLCK) ;
    lseek(fd, 0, SEEK_END);
    if(SafeWrite(fd,(char*)data,len) == -1)
	printf("write %s:%s",m_strFileName,strerror(errno));
    flock(fd,F_UNLCK);
    close(fd) ;
    return 0 ;
}

int CRecordFile::SubstituteRecord(void* data,int len,int pos)
{
    int fd ;
    if((fd = open(m_strFileName,O_WRONLY|O_CREAT,0644)) == -1) {
	printf("open %s:%s",m_strFileName,strerror(errno));
        return -1 ;
    }
    flock(fd,F_WRLCK) ;
    if (lseek(fd,len*(pos-1),SEEK_SET) == -1)
    	return -1;
    if (SafeWrite(fd,(char*)data,len) != len)
    	return -1;
    flock(fd,F_UNLCK) ;
    close(fd) ;
    return 0 ;
}
