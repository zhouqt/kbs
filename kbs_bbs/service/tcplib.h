#ifndef XXJH_TCPLIB_H
#define XXJH_TCPLIB_H

#include "bbs.h"

/**
 * tcplib 用到的错误号.
 */
enum TCPLIB_ERRNO
{
	ERR_TCPLIB_CONNECT    = -2,	 /** 连接失败 */
	ERR_TCPLIB_TIMEOUT    = -3,	 /** 超时 */
	ERR_TCPLIB_RECVFAILED = -4,	 /** 接收报文失败 */
	ERR_TCPLIB_SENTFAILED = -5,	 /** 发送报文失败 */
	ERR_TCPLIB_VERSION    = -10, /** 版本错误 */
	ERR_TCPLIB_HEADER     = -11, /** 报文头格式错误 */
	ERR_TCPLIB_TOOLONG    = -12, /** 报文数据过长 */
	ERR_TCPLIB_OPENFILE   = -20, /** 打开文件错误 */
	ERR_TCPLIB_RESUMEPOS  = -21, /** 续传位置错误 */
    ERR_TCPLIB_OTHERS     = -100 /** 其他错误，可从 errno 获得错误信息 */
};

#ifdef __cplusplus
const int TCPLIB_TRUE  = 1;
const int TCPLIB_FALSE = 0;
const int TIME_CNV_RATIO   = 1000;	/* 时间转换率，秒->毫秒，毫秒->微秒 */
#else
#define TCPLIB_TRUE  1
#define TCPLIB_FALSE 0
#define TIME_CNV_RATIO   1000	/* 时间转换率，秒->毫秒，毫秒->微秒 */

#endif

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**
 * 用于把毫秒值转换成 timeval 结构体.
 * 调用者必须提供一个 timeval 结构体的缓冲区，
 * 并把该缓冲区的起始地址传入.
 * 
 * @param ms 待转换的毫秒值.
 * @param tv 指向 timeval 结构体的指针.
 * @see TimevalToMs()
 */
void MsToTimeval(const int ms, struct timeval *tv);

/**
 * 用于把 timeval 结构体转换成毫秒值.
 * 
 * @param tv 指向待转换的 timeval 结构体的指针.
 * @return 转换后的毫秒数值
 * @see MsToTimeval()
 */
int TimevalToMs(const struct timeval *tv);

/**
 * 用于两个 timeval 结构体的相减.
 * 调用者提供两个 timeval 结构体指针参数，转换时，
 * 由第一个参数减去第二个参数。函数返回时，将转换
 * 结果存放在第一参数中.
 * 
 * @param ltv 左 timeval 结构体指针（在减号的左边，相当于被减数）
 * @param rtv 右 timeval 结构体指针（在减号的右边，相当于减数）
 */
void TimevalSubtract(struct timeval *ltv, 
							 const struct timeval *rtv);

/**
 * 非阻塞型的 TCP 连接函数.
 * 这个函数用法与标准的 connect() 函数非常类似，唯一的区别在于用户
 * 可以自己指定 TCP 连接过程的超时时间。如果在这个时间内没能成功地
 * 连接上目标，将返回超时错误。前面三个参数的意义与 connect() 完全
 * 相同，最后一个参数是超时时间。关于这个函数的实现请参考 UNPv1e2
 * 第 411 页。
 * 
 * @param sockfd socket 描述符
 * @param saptr 指向存放目标主机地址结构的指针
 * @param salen 地址结构的长度
 * @param msec 连接过程的超时时间，单位毫秒
 * @return ==0 连接成功
 *         <0  连接失败
 * @see connect()
 */
int NonBlockConnect(int sockfd, const struct sockaddr *saptr,
		socklen_t salen, const int msec);

int NonBlockConnectEx(int sockfd, const struct sockaddr *saptr,
		socklen_t salen, const int msec, const int conn);

/**
 * 带超时控制的 TCP 连接函数.
 * 这个函数是对 NonBlockConnect() 的封装，用户只需提供目标主机的IP
 * 地址，端口号以及超时时间，就可以完成复杂的 TCP 连接过程。
 * 
 * @param ip 目标主机的 IP 地址
 * @param port 目标主机的端口号
 * @param msec 连接过程的超时时间，单位毫秒
 * @return >=0 连接成功，返回值为用于和目标主机通讯的 socket 描述符
 *         <0  连接失败
 * @see NonBlockConnect()
 */
int DoConnect(const char *ip, const int port, const int msec);

/**
 * 断开 TCP 连接的函数.
 * 
 * @param conn_fd 待断开的 TCP 连接的描述符
 */
void DoDisconnect(int conn_fd);

/**
 * 不受信号中断影响的 select() 函数.
 * 这个函数名为 SignalSafeSelect()，顾名思义，可以知道这个函数是不受
 * 信号中断的影响的。它实际上是对标准 select() 的封装，并小心地检查
 * select() 返回的错误值，如果发现错误是由于信号中断引起的，忽略掉这
 * 个信号中断错误并重新调用 select()。这个函数的使用方法与 select()
 * 完全一致。
 * 
 * @param nfds 需要检查的描述符数目
 * @param rs 用于检查是否可读的描述符集合
 * @param ws 用于检查是否可写的描述符集合
 * @param es 用于检查是否异常的描述符集合
 * @param tv 超时时间
 * @return >0  已经准备好(可读、可写或异常及它们的组合)的描述符数量
 *         ==0 超时
 *         <0  出错
 * @see select()
 */
int SignalSafeSelect(int nfds, fd_set *rs, fd_set *ws, fd_set *es,
		struct timeval *tv);

/**
 * 更安全的 accept() 函数.
 * 顾名思义，SafeAccept() 表示这个函数是“安全的”，当然不可能是绝对
 * 的安全，但是比标准的 accept() 要安全。这个函数不受信号中断的影响，
 * 而且小心地处理了 accept() 返回的错误。这个函数的用法与 accept() 
 * 完全相同。关于它的实现，请参考 UNPv1e2 第 422 页。
 * 
 * @param sockfd Listening socket 描述符
 * @param cliaddr 客户端的地址结构
 * @param addrlen 客户端的地址结构的长度
 * @return >=0 接受连接请求成功，返回值为用于和客户端通讯的 socket
 *             描述符
 *         <0  出错
 * @see accept()
 */
int SafeAccept(int sockfd, struct sockaddr *cliaddr, socklen_t *addrlen);

/**
 * 带超时控制的报文发送函数.
 */
int DoSendData(int sockfd, const void *buf, const size_t len, int msec);

/**
 * 带超时控制的报文接收函数.
 */
int DoRecvData(int sockfd, void *buf, const size_t len, int msec);

#ifdef __cplusplus
}
#endif

#endif /* XXJH_TCPLIB_H */

