#include "bbs.h"
int f_rm(char *fpath);
int f_mv(char *src, char *dst);
int f_cp(char *src, char *dst, int mode);
void f_cat(char *fpath, char *msg);
int f_touch(char *filename);
int f_ln(char *src, char *dst);

/* 字符串匹配函数*/
char* bm_strcmp(const char* string,const char* pattern);
/* 字符串多次匹配函数*/
char* bm_strcmp_rp(const char* string,const char* pattern,
	size_t* shift,bool* init);
/* 字符串大小写不敏感的匹配函数*/
char* bm_strcasecmp(const char* string,const char* pattern);
/* 字符串多次大小写不敏感匹配函数*/
char* bm_strcasecmp_rp(const char* string,const char* pattern,
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

