#include "bbs.h"
int f_rm(char *fpath);
int f_mv(char *src, char *dst);
int f_cp(char *src, char *dst, int mode);
void f_cat(char *fpath, char *msg);
void f_catfile(char* srcpath,char* dstpath);
int f_touch(char *filename);
int f_ln(char *src, char *dst);
typedef int(*OUTC_FUNC)(char);
void uuencode(char* inbuf, int size, char *filename, OUTC_FUNC fn);


/* 字符串匹配函数*/
char* bm_strstr(const char* string,const char* pattern);
/* 字符串多次匹配函数*/
char* bm_strstr_rp(const char* string,const char* pattern,
	size_t* shift,bool* init);
/* 字符串大小写不敏感的匹配函数*/
char* bm_strcasestr(const char* string,const char* pattern);
/* 字符串多次大小写不敏感匹配函数*/
char* bm_strcasestr_rp(const char* string,const char* pattern,
	size_t* shift,bool* init);
void *memfind(const void *in_block,     /* 数据块 */
              const size_t block_size,  /* 数据块长度 */
              const void *in_pattern,   /* 需要查找的数据 */
              const size_t pattern_size,        /* 查找数据的长度 */
              size_t * shift,   /* 移位表，应该是256*size_t的数组 */
              bool * init); /* 是否需要初始化移位表 */

void *txtfind(const void *in_block,     /* 数据块 */
              const size_t block_size,  /* 数据块长度 */
              const void *in_pattern,   /* 需要查找的数据 */
              const size_t pattern_size,        /* 查找数据的长度 */
              size_t * shift,   /* 移位表，应该是256*size_t的数组 */
              bool * init); /* 是否需要初始化移位表 */

int lock_reg(int fd,int cmd,int type,off_t offset,int whence,off_t len);
pid_t lock_test(int fd,int cmd,int type,off_t offset,int whence,off_t len);

/* some marco from APUE*/
#define read_lock(fd,offset,whence,len)\
	lock_reg(fd,F_SETLK,F_RDLCK,offset,whence,len)

#define readw_lock(fd,offset,whence,len)\
	lock_reg(fd,F_SETLKW,F_RDLCK,offset,whence,len)

#define write_lock(fd,offset,whence,len)\
	lock_reg(fd,F_SETLK,F_WRLCK,offset,whence,len)

#define writew_lock(fd,offset,whence,len)\
	lock_reg(fd,F_SETLKW,F_WRLCK,offset,whence,len)

#define un_lock(fd,offset,whence,len)\
	lock_reg(fd,F_SETLKW,F_UNLCK,offset,whence,len)

#define is_read_lockable(fd,offset,whence,len)\
	!lock_test(fd,F_GETLK,F_RDLCK,offset,whence,len)

#define is_write_lockable(fd,offset,whence,len)\
	!lock_test(fd,F_GETLK,F_WRLCK,offset,whence,len)

