#ifndef IPLOOK_H
#define IPLOOK_H

#ifdef __cplusplus
extern "C" {
#endif 
/* 
 *  *
 *  *   IP 查询接口， kxn@smth
 *  *    
 *  *     @param  const char *ipstr      IP 字符串
 *  *     @param  char ** area           接受 area 的字符串指针
 *  *     @param  char ** location       接受 location 的字符串指针
 *  *     
 *  *     @result int                    0 表查询成功， -1 查找不到
 *  * 
 *  * */
	
int LookIP(const char *ipstr, char **area, char **location);

/*
 *  IP 信息日期，从 QQWry.Dat 中取得
 */  

char *GetIPInfoVersion();

/*
 *  IP 信息数量，由 convert 生成
 */

int GetIPInfoCount();

#ifdef __cplusplus
};
#endif
	
#endif 

