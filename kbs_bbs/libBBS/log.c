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
    
    
    
    bbslog.c
    Copyright (C) 2001, wwj@j32.org
*/


#include "bbs.h"
#include <stdarg.h>


/*

  优先级
  
  0   message
  1   notice
  2   warning
  3   error
  4   cirtical error
*/

typedef struct _logconfig{
    int  compare;  /* -1 小于等于, 0 相等,  1 大于等于 */
    int  prio;     /* 优先级 */
    char * name;   /* 确定的来源，NULL是全匹配 */
    char * file;   /* 文件名，相对于 BBSHOME/reclog ，如果是NULL，丢弃 */
    int  bufsize;  /* 缓存大小，如果是 0，不缓存 */
    int  searchnext; /* 如果这个匹配了，还查找后面的config吗？*/
    
    /* 运行时参数 */
    int  bufptr;   /* 使用缓存位置 */
    char * buf;    /* 缓存 */
    int  fd;       /* 文件句柄 */
}logconfig;

static logconfig logconf[]= {
    {1, 3, NULL,"error.log", 0 , 1, 0, NULL, 0},   /* 错误记录 */
    {1, 0, "connect","connect.log", 0, 0, 0, NULL, 0},     /* 连接记录 */
    {1, 0, "user","user.log", 0, 0, 0, NULL, 0},/*所有用户的使用记录，使用缓冲*/
    {1, 0, "usermsg","msg.log", 0 , 0, 0, NULL, 0},/* 所有用户的message记录 */
    {1, 0, "boardusage", "boardusage.log",0,0, 0, NULL, 0}, /* 版面使用记录 */
    {1, 0, "chatd","trace.chatd",1024, 0, 0, NULL, 0},  /* 聊天室使用记录，使用缓冲 */
    {1, 0, "system","usies", 0 , 0, 0, NULL, 0},   /* 最后所有的记录都在这里 */
    {1, 0, NULL,"trace", 0 , 0, 0, NULL, 0}   /* 最后所有的记录都在这里 */
};

static int bdoatexit=0;


static void getheader(char * header,const char *from,int prio)
{
    struct tm *pt;
    time_t tt;
    time(&tt);
    pt=localtime(&tt);
     
    sprintf(header,"[%02u/%02u %02u:%02u:%02u %5d %d.%s] %s ",
        pt->tm_mon+1,pt->tm_mday,pt->tm_hour,pt->tm_min,pt->tm_sec,getpid(),
        prio,from,
        currentuser->userid
        );
}

/* 写入log, 如果buf==NULL那么flush。否则根据大小决定是否缓存 */
static void writelog(logconfig * pconf,const char * from,int prio, const char * buf)
{
    char header[64];
    
    if(!from)from=pconf->name;
    if(!from)from="unknown";
    
    getheader(header,from,prio);
    
    if(buf && pconf->buf){
        if((int)(pconf->bufptr+strlen(header)+strlen(buf)+2)<=pconf->bufsize){
             strcpy(&pconf->buf[pconf->bufptr],header);
             pconf->bufptr+=strlen(header);
             strcpy(&pconf->buf[pconf->bufptr],buf);
             pconf->bufptr+=strlen(buf);
             strcpy(&pconf->buf[pconf->bufptr],"\r\n");
             pconf->bufptr+=2;
             return;
        }
    }
    
    flock(pconf->fd,LOCK_SH);
    lseek(pconf->fd,0,SEEK_END);
	                
    if(pconf->buf && pconf->bufptr){
        write(pconf->fd,pconf->buf,pconf->bufptr);
        pconf->bufptr=0;
    }
    if(buf){
        write(pconf->fd,header,strlen(header));
        write(pconf->fd,buf,strlen(buf));
        write(pconf->fd,"\r\n",2);
    }
    flock(pconf->fd,LOCK_UN);
}

static void logatexit()
{
    logconfig * pconf;
    pconf=logconf;
    
    while(pconf-logconf<(int)(sizeof(logconf)/sizeof(logconfig))){
        if(pconf->buf && pconf->bufptr) writelog(pconf,NULL,0,NULL);
        if(pconf->buf)free(pconf->buf);
        if(pconf->fd>0)close(pconf->fd);
        pconf++;   
    }
}

int bbslog(const char * from,const char *fmt,...)
{
    logconfig * pconf;
    int  prio;
    char buf[512];
	va_list v;
    
    prio=0;
    
    if(from[0]>='0' && from[0]<='9'){
        prio=from[0]-'0';
        from++;
    }
/* temp disable bbslog*/
#ifdef BBSMAIN
    if (prio==0&&strcasecmp(from,"boardusage")) return 0;
#endif
    if(strlen(from)>16)return -3;
    if(!fmt || !*fmt)return 0;
    
	va_start( v,fmt );
    vsprintf(buf,fmt,v);
	
    
    pconf=logconf;
    
    while(pconf-logconf<(int)(sizeof(logconf)/sizeof(logconfig))){
        if( (pconf->compare== 1 && prio>=pconf->prio) ||
            (pconf->compare== 0 && prio==pconf->prio) ||
            (pconf->compare==-1 && prio<=pconf->prio) ){
            if(!pconf->name || !strcasecmp(pconf->name,from)) {
                if(pconf->fd<=0){  /* init it! */
                    if(!pconf->file)return 0; /* discard it */
                    pconf->fd=open(pconf->file,O_WRONLY);
	                if(pconf->fd<0)pconf->fd=creat(pconf->file,0644);
	                if(pconf->fd<0)return -1;
	                
	                if(pconf->bufsize){
	                    if(!bdoatexit){ atexit(logatexit); bdoatexit=1; }
	                    pconf->buf=(char *)malloc(pconf->bufsize);
	                    pconf->bufptr=0;
	                }
                }
                writelog(pconf,from,prio,buf);
                if(!pconf->searchnext)break;
            }
        }
        pconf++;
    }
        if (pconf->fd>=0 && !pconf->bufsize) {        
        close(pconf->fd);         
        pconf->fd=-1;             
    }                             

	return 0;
}



