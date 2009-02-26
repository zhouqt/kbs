#include "tcplib.h"
#include "bbs.h"

void
MsToTimeval(const int ms, struct timeval *tv)
{
    tv->tv_sec = ms / TIME_CNV_RATIO;
    tv->tv_usec = (ms - tv->tv_sec * TIME_CNV_RATIO) * TIME_CNV_RATIO;
}

int
TimevalToMs(const struct timeval *tv)
{
    return (tv->tv_sec * TIME_CNV_RATIO + tv->tv_usec / TIME_CNV_RATIO);
}

void
TimevalSubtract(struct timeval *ltv,
                const struct timeval *rtv)
{
    if (ltv->tv_usec < rtv->tv_usec) {
        ltv->tv_sec --;
        ltv->tv_usec += TIME_CNV_RATIO * TIME_CNV_RATIO;
    }
    ltv->tv_sec -= rtv->tv_sec;
    ltv->tv_usec -= rtv->tv_usec;
}

int
NonBlockConnect(int sockfd, const struct sockaddr *saptr,
                socklen_t salen, const int msec)
{
    int flags, n, error;
    socklen_t len;
    fd_set rset, wset;
    struct timeval tval;

    /* 设置为非阻塞的 socket */
    flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    error = 0;
    if ((n = connect(sockfd, saptr, salen)) < 0) {
        if (errno != EINPROGRESS)
            return ERR_TCPLIB_CONNECT; /* 连接失败 */
    }

    /* Do whatever we want while the connect is taking place. */

    if (n == 0)
        goto done; /* connect completed immediately */

    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);
    wset = rset;  /* structure assignment */
    MsToTimeval(msec, &tval);

    n = SignalSafeSelect(sockfd+1, &rset, &wset, NULL, msec ? &tval : NULL);
    if (n == 0) {
        errno = ETIMEDOUT;
        return ERR_TCPLIB_TIMEOUT; /* 连接服务器超时 */
    } else if (n < 0)
        return ERR_TCPLIB_OTHERS; /* select()出错 */
    if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)) {
        len = sizeof(error);
        if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
            return ERR_TCPLIB_OTHERS; /* Solaris pending error */
    }

done:
    fcntl(sockfd, F_SETFL, flags); /* restore file status flags */

    if (error) {
        errno = error;
        return ERR_TCPLIB_CONNECT; /* 连接服务器失败 */
    }
    return 0; /* 连接服务器成功 */
}

int
NonBlockConnectEx(int sockfd, const struct sockaddr *saptr,
                  socklen_t salen, const int msec, const int conn)
{
    int flags, n, error;
    socklen_t len;
    fd_set rset, wset;
    struct timeval tval;

    /* 设置为非阻塞的 socket */
    flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    error = 0;
    if (conn == 1) {
        if ((n = connect(sockfd, saptr, salen)) < 0) {
            if (errno != EINPROGRESS)
                return ERR_TCPLIB_CONNECT; /* 连接失败 */
        }

        /* Do whatever we want while the connect is taking place. */

        if (n == 0)
            goto done; /* connect completed immediately */
    }

    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);
    wset = rset;  /* structure assignment */
    MsToTimeval(msec, &tval);

    n = SignalSafeSelect(sockfd+1, &rset, &wset, NULL, msec ? &tval : NULL);
    if (n == 0) {
        errno = ETIMEDOUT;
        return ERR_TCPLIB_TIMEOUT; /* 连接服务器超时 */
    } else if (n < 0)
        return ERR_TCPLIB_OTHERS; /* select()出错 */
    if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)) {
        len = sizeof(error);
        if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
            return ERR_TCPLIB_OTHERS; /* Solaris pending error */
    }

done:
    fcntl(sockfd, F_SETFL, flags); /* restore file status flags */

    if (error) {
        errno = error;
        return ERR_TCPLIB_CONNECT; /* 连接服务器失败 */
    }
    return 0; /* 连接服务器成功 */
}

int
DoConnect(const char *ip, const int port, const int msec)
{
    struct sockaddr_in sa;
    int conn_fd;
    int rv;

    /*assert(ip != NULL && port != 0);*/

    /* 创建 socket */
    if ((conn_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return ERR_TCPLIB_OTHERS; /* 创建 socket 失败 */
    /* 设置 sockaddr_in 结构 */
    bzero(&sa, sizeof(sa));
    sa.sin_family = AF_INET;
#ifdef HAVE_INET_ATON
    if (inet_aton(ip, &sa.sin_addr) <= 0)
#else
    if (inet_pton(AF_INET, ip, &sa.sin_addr) <= 0)
#endif
        return ERR_TCPLIB_OTHERS; /* 获取服务器地址失败 */
    sa.sin_port = htons(port);
    /* 创建连接 */
    rv = NonBlockConnect(conn_fd, (struct sockaddr *)&sa, sizeof(sa), msec);
    if (rv < 0) {
        /* 创建连接失败，关掉 socket */
        close(conn_fd);
        return rv;
    }

    return conn_fd;
}

void
DoDisconnect(int conn_fd)
{
    close(conn_fd);
}

int
SignalSafeSelect(int nfds, fd_set *rs, fd_set *ws, fd_set *es,
                 struct timeval *tv)
{
    int rv;

    while ((rv = select(nfds, rs, ws, es, tv)) < 0) {
        if (errno != EINTR)
            break;
    }
    return rv;
}

int
SafeAccept(int sockfd, struct sockaddr *cliaddr, socklen_t *addrlen)
{
    int flags, n;
    fd_set rset;
    int conn_fd = ERR_TCPLIB_OTHERS;

    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);
    n = SignalSafeSelect(sockfd+1, &rset, NULL, NULL, NULL);
    if (n < 0)
        return ERR_TCPLIB_OTHERS;
    if (FD_ISSET(sockfd, &rset)) {
        /* 设置为非阻塞的 socket */
        flags = fcntl(sockfd, F_GETFL, 0);
        fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
        /* 然后调用 accept() 接受连接请求 */
        while ((conn_fd = accept(sockfd, cliaddr, addrlen)) < 0) {
            /* from UNPv1 Page 424 */
            if (errno != EWOULDBLOCK && errno != ECONNABORTED
                    && errno != EINTR)
                break;
        }
        /* 使 socket 复原 */
        fcntl(sockfd, F_SETFL, flags);
    }
    return conn_fd;
}

int
DoSendData(int sockfd, const void *buf, const size_t len, int msec)
{
    struct timeval tv;
    fd_set wset;
    int rv;
    int wc;

    MsToTimeval(msec, &tv);
    FD_ZERO(&wset);
    FD_SET(sockfd, &wset);
    rv = SignalSafeSelect(sockfd+1, NULL, &wset, NULL, &tv);
    if (rv < 0)
        return ERR_TCPLIB_OTHERS; /* some error occured */
    if (rv == 0)
        return ERR_TCPLIB_TIMEOUT; /* timed out */
    if (FD_ISSET(sockfd, &wset)) {
        /* 一次发送所有数据 */
        wc = send(sockfd, buf, len, 0);
        return wc; /* return bytes had sent */
    }
    return 0; /* nothing had sent */
}

int
DoRecvData(int sockfd, void *buf, const size_t len, int msec)
{
    struct timeval tv;
    fd_set rset;
    int rv;
    int rc;

    MsToTimeval(msec, &tv);
    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);
    rv = SignalSafeSelect(sockfd+1, &rset, NULL, NULL, msec ? &tv : NULL);
    if (rv < 0)
        return ERR_TCPLIB_OTHERS; /* some error occured */
    if (rv == 0)
        return ERR_TCPLIB_TIMEOUT; /* timed out */
    if (FD_ISSET(sockfd, &rset)) {
        /* 一次接收所有数据 */
        rc = recv(sockfd, buf, len, 0);
        return rc; /* return bytes had received */
    }
    return 0; /* nothing had received */
}

